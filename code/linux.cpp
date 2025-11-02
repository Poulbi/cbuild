//- Error wrappers 
void LinuxErrorWrapperPipe(int Pipe[2])
{
    int Ret = pipe(Pipe);
    if(Ret == -1)
    {
        perror("pipe");
        Assert(0);
    }
}

void LinuxErrorWrapperDup2(int OldFile, int NewFile)
{
    int Ret = dup2(OldFile, NewFile);
    if(Ret == -1)
    {
        perror("dup2");
        Assert(0);
    }
}

int LinuxErrorWrapperGetAvailableBytesToRead(int File)
{
    int BytesToRead = 0;
    
    int Ret = ioctl(File, FIONREAD, &BytesToRead);
    if(Ret == -1)
    {
        perror("read(ioctl)");
        Assert(0);
    }
    
    return BytesToRead;
}

psize LinuxErrorWrapperRead(int File, void *Buffer, psize BytesToRead)
{
    psize BytesRead = read(File, Buffer, BytesToRead);
    if(BytesRead == -1)
    {
        perror("read");
        Assert(0);
    }
    Assert(BytesRead == BytesRead);
    
    return BytesRead;
}

//-
str8 LinuxFindCommandInPATH(psize BufferSize, u8 *Buffer, char *Command, char *Env[])
{
    char **VarAt = Env;
    char Search[] = "PATH=";
    int MatchedSearch = false;
    
    str8 Result = {};
    Result.Data = Buffer;
    
    while(*VarAt && !MatchedSearch)
    {
        MatchedSearch = true;
        
        for(unsigned int At = 0;
            (At < sizeof(Search) - 1) && (VarAt[At]);
            At++)
        {
            if(Search[At] != VarAt[0][At])
            {
                MatchedSearch = false;
                break;
            }
        }
        
        VarAt++;
    }
    
    if(MatchedSearch)
    {
        VarAt--;
        char *Scan = VarAt[0];
        while(*Scan && *Scan != '=') Scan++;
        Scan++;
        
        while((*Scan) && (Scan != VarAt[1]))
        {
            int Len = 0;
            while(Scan[Len] && Scan[Len] != ':' && 
                  (Scan+Len != VarAt[1])) Len++;
            
            // Add the PATH entry
            int At;
            for(At = 0; At < Len; At++)
            {
                Result.Data[At] = Scan[At];
            }
            Result.Data[At++] = '/';
            
            // Add the executable name
            for(char *CharAt = Command;
                *CharAt;
                CharAt++)
            {
                Result.Data[At++] = *CharAt;
            }
            Result.Data[At] = 0;
            
            // Check if it exists
            int AccessMode = F_OK | X_OK;
            int Ret = access((char *)Result.Data, AccessMode);
            if(Ret == 0)
            {
                Result.Size = At;
                break;
            }
            
            Scan += Len + 1;
        }
    }
    
    return Result;
}

linux_command_result LinuxRunCommand(char *Args[], b32 Pipe)
{
    linux_command_result Result = {};
    
    int StdoutPipe[2] = {};
    int StdinPipe[2] = {};
    int StderrPipe[2] = {};
    
    int Ret = 0;
    int WaitStatus = 0;
    
    if(Pipe)
    {    
        LinuxErrorWrapperPipe(StdoutPipe);
        LinuxErrorWrapperPipe(StdinPipe);
        LinuxErrorWrapperPipe(StderrPipe);
        Result.Stdout = StdoutPipe[0];
        Result.Stdin = StdinPipe[1];
        Result.Stderr = StderrPipe[0];
    }
    
    pid_t ChildPID = fork();
    if(ChildPID != -1)
    {
        if(ChildPID == 0)
        {
            if(Pipe)
            {            
                LinuxErrorWrapperDup2(StdoutPipe[1], STDOUT_FILENO);
                LinuxErrorWrapperDup2(StdinPipe[0], STDIN_FILENO);
                LinuxErrorWrapperDup2(StderrPipe[1], STDERR_FILENO);
            }
            
            if(execve(Args[0], Args, __environ) == -1)
            {
                perror("exec");
            }
        }
        else
        {
            wait(&WaitStatus);
        }
    }
    else
    {
        perror("fork");
    }
    
    b32 Exited = WIFEXITED(WaitStatus);
    if(Exited)
    {
        s8 ExitStatus = WEXITSTATUS(WaitStatus);
        if(ExitStatus)
        {
            Result.Error = true;
        }
    }
    
    if(Pipe)
    {    
        Result.StdoutBytesToRead = LinuxErrorWrapperGetAvailableBytesToRead(Result.Stdout);
        Result.StderrBytesToRead = LinuxErrorWrapperGetAvailableBytesToRead(Result.Stderr);
    }
    
    return Result;
}

linux_command_result LinuxRunCommandString(str8 Command, char *Env[], b32 Pipe)
{
    linux_command_result Result = {};
    
    char *Args[64] = {};
    
    u8 ArgsBuffer[1024] = {};
    psize ArgsBufferIndex = 0;
    
    // 1. split on whitespace into null-terminated strings.
    //    TODO: skip quotes
    u32 ArgsCount = 0;
    psize Start = 0;
    for(psize At = 0;
        At <= Command.Size;
        At++)
    {
        if(IsWhiteSpace(Command.Data[At]) || At == Command.Size)
        {
            Args[ArgsCount++] = (char *)(ArgsBuffer + ArgsBufferIndex);
            Assert(ArgsCount < ArrayCount(Args));
            psize Size = At - Start;
            MemoryCopy(ArgsBuffer + ArgsBufferIndex, Command.Data + Start, Size);
            ArgsBufferIndex += Size;
            ArgsBuffer[ArgsBufferIndex++] = 0;
            Assert(ArgsBufferIndex < ArrayCount(ArgsBuffer));
            
            while(IsWhiteSpace(Command.Data[At])) At++;
            
            Start = At;
        }
    }
    
    if(Args[0] && Args[0][0] != '/')
    {
        u8 Buffer[PATH_MAX] = {};
        str8 ExePath = LinuxFindCommandInPATH(sizeof(Buffer), Buffer, Args[0], Env);
        if(ExePath.Size)
        {
            Args[0] = (char *)ExePath.Data;
        }
    }
    
    Result = LinuxRunCommand(Args, Pipe);
    
    return Result;
}

internal void
LinuxChangeToExecutableDirectory(char *Args[])
{
    char *ExePath = Args[0];
    s32 Length = (s32)CountCString(ExePath);
    char ExecutableDirPath[PATH_MAX] = {};
    s32 LastSlash = 0;
    for(s32 At = 0;
        At < Length;
        At++)
    {
        if(ExePath[At] == '/')
        {
            LastSlash = At;
        }
    }
    MemoryCopy(ExecutableDirPath, ExePath, LastSlash);
    ExecutableDirPath[LastSlash] = 0;
    if(chdir(ExecutableDirPath) == -1)
    {
        perror("chdir");
    }
}

void LinuxRebuildSelf(str8 BuildCommand, int ArgsCount, char *Args[], char *Env[])
{
    linux_command_result CommandResult = LinuxRunCommandString(BuildCommand, Env, true);
    if(CommandResult.Error)
    {
        u8 Buf[64 * 1024] = {};
        psize BytesRead = LinuxErrorWrapperRead(CommandResult.Stderr, Buf, CommandResult.StderrBytesToRead);
        printf("%*s\n", (int)BytesRead, Buf);
    }
    
    // Run without rebuilding
    char *Arguments[64] = {};
    u32 At;
    for(At = 1;
        At < ArgsCount;
        At++)
    {
        if(strcmp(Args[At], "rebuild"))
        {
            Arguments[At] = Args[At];
        }
    }
    Arguments[0] = "./cshell";
    // NOTE(luca): We changed to the Executable's build path
    Arguments[At++] = "norebuild";
    Assert(At < ArrayCount(Arguments));
    
    LinuxRunCommand(Arguments, false);
}