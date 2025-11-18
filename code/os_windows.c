// Windows OS layer

#ifndef OS_WINDOWS_C
#define OS_WINDOWS_C

//~ Constants
// NOTE(luca): Because windows cannot overwrite an opened file, we create a helper executable that will do the rest for us.
#define CBUILD_TEMP_EXE "cbuild_temp.exe"

//~ Types
struct windows_command_result
{
    // TODO(luca): Add pipes.
    int Nothing;
};
typedef struct windows_command_result windows_command_result;

#endif // OS_WINDOWS_C

//~ Implementation

#ifdef OS_WINDOWS_IMPLEMENTATION

internal void 
WindowsRunCommand(u8 *Command, b32 Pipe)
{
    STARTUPINFOA StartupInfo = {0};
    StartupInfo.cb = sizeof(StartupInfo);
    
    PROCESS_INFORMATION ProcessInfo = {0};
    
    b32 Succeeded = CreateProcessA(0, 
                                   (char *)Command, 
                                   0,
                                   0,
                                   false, 
                                   0,
                                   0, 
                                   0,
                                   &StartupInfo, &ProcessInfo);
    if(Succeeded)
    {
        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
    } 
    else 
    {
        printf("ERROR: CreateProcess failed (%d).\n", GetLastError());
        printf("Command: %s\n", Command);
    }
    
}

internal windows_command_result
WindowsRunCommandString(str8 Command, b32 Pipe)
{
    windows_command_result Result = {0};
    
    // NOTE(luca): Ensure command is null terminated.
    Command.Data[Command.Size] = 0;
    WindowsRunCommand(Command.Data, Pipe);
    
    return Result;
}

internal void
WindowsChangeDirectory(char *Path)
{
    Assert(SetCurrentDirectory(Path));
}

internal void
WindowsRebuildSelf(str8 StringsBuffer, str8 OutputBuffer,
                   int ArgsCount, char *Args[], char *EnvironmentUnused[])
{
    b32 Rebuild = true;
    b32 ForceRebuild = false;
    for(int ArgsIndex = 1;
        ArgsIndex < ArgsCount;
        ArgsIndex++)
    {
        if(!strcmp(Args[ArgsIndex], "norebuild"))
        {
            Rebuild = false;
        }
        if(!strcmp(Args[ArgsIndex], "rebuild"))
        {
            ForceRebuild = true;
        }
    }
    
    if(ForceRebuild || Rebuild)
    {
        printf("[self compile]\n");
        
        // Build self 
        {        
            str8_list BuildCommandList = CommonBuildCommand(StringsBuffer, false, true, true);
            
            Str8ListAppend(&BuildCommandList, S8Lit(CBUILD_SOURCE));
            Str8ListAppend(&BuildCommandList, S8Lit("/link /out:" CBUILD_TEMP_EXE));
            
            // NOTE(luca): We use `OutputBuffer` as a temporary second location, TODO: we should take a partition instead.
            str8 BuildCommand = Str8ListJoin(BuildCommandList, OutputBuffer, ' ');
            MemoryCopy(StringsBuffer.Data, BuildCommand.Data, BuildCommand.Size);
            WindowsRunCommandString(BuildCommand, true);
        }
        
        // Run new cbuild.exe
        {
            str8_list CommandList = {0};
            CommandList.Strings = (str8 *)(StringsBuffer.Data);
            CommandList.Capacity = StringsBuffer.Size;
            
            // Set the executable to the new exe
            Str8ListAppend(&CommandList, S8Lit(".\\" CBUILD_TEMP_EXE));
            
            // NOTE(luca): Run without rebuilding
            s32 At;
            for(At = 1;
                At < ArgsCount;
                At++)
            {
                // Skip the rebuilding argument
                if(strcmp(Args[At], "rebuild"))
                {
                    str8 Arg = {0};
                    Arg.Data = (u8 *)Args[At];
                    Arg.Size = CountCString(Args[At]);
                    Str8ListAppend(&CommandList, Arg); 
                }
            }
            
            Str8ListAppend(&CommandList, S8Lit("norebuild"));
            
            str8 Command = Str8ListJoin(CommandList, OutputBuffer, ' ');
            WindowsRunCommandString(Command, false);
        }
        
        ExitProcess(0);
    }
}

#endif // OS_WINDOWS_IMPLEMENTATION