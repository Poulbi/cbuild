//~ Libraries
// Standard library (TODO(luca): get rid of it) 
#include <stdio.h>
#include <string.h>
// POSIX
#include <unistd.h>
// Linux
#include <sys/wait.h>
#include <linux/limits.h>
// External
#include "libs/lr/lr_types.h"

//~ Macros
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Assert(Expression) if(!(Expression)) { __asm__ volatile("int3"); } 
#define MemoryCopy memcpy

struct str8
{
    psize Size;
    u8 *Data;
};
#define S8Lit(String) (str8){.Size = (sizeof((String)) - 1), .Data = (u8 *)(String)}

struct str8_node
{
    str8 *Next;
    str8 *Prev;
};

struct str8_list
{
    str8 *Strings;
    psize Count;
    psize Capacity;
};

void LinuxRunCommand(char *Args[])
{
    int WaitStatus = 0;
    pid_t ChildPID = fork();
    if(ChildPID != -1)
    {
        if(ChildPID == 0)
        {
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
    
    // TODO(luca): We need to propagate back the error so we can check it.
}

b32 IsWhiteSpace(u8 Char)
{
    b32 Result = (Char == ' ' || Char == '\t' || Char == '\n');
    return Result;
}

str8 LinuxFindCommandInPATH(psize BufferSize, u8 *Buffer, char *Command)
{
    char **VarAt = __environ;
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

void PlatformRunCommand(str8 Command)
{
    char *Args[60] = {};
    
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
        str8 ExePath = LinuxFindCommandInPATH(sizeof(Buffer), Buffer, Args[0]);
        if(ExePath.Size)
        {
            Args[0] = (char *)ExePath.Data;
        }
    }
    
    LinuxRunCommand(Args);
}

void PlatformAppendArgumentAndRun(int ArgsCount, char *Args[], char *Executable, char *ExtraArgument)
{
    char *Arguments[30] = {};
    u32 At;
    for(At = 1;
        At < ArgsCount;
        At++)
    {
        Arguments[At] = Args[At];
    }
    Arguments[0] = Executable;
    Arguments[At++] = ExtraArgument;
    
    Assert(At < ArrayCount(Arguments));
    
    LinuxRunCommand(Arguments);
}

psize CountCString(char *String)
{
    psize Result = 0;
    
    while(*String++) Result++;
    
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

internal void
LinuxSigINTHandler(int SigNum)
{
}


void Str8ListAppend(str8_list *List, str8 String)
{
    Assert(List->Count < List->Capacity);
    List->Strings[List->Count++] = String;
}

#define Str8ListAppendMultiple(List, ...) \
do \
{ \
str8 Strings[] = {__VA_ARGS__}; \
_Str8ListAppendMultiple(List, ArrayCount(Strings), Strings); \
} while(0);
void _Str8ListAppendMultiple(str8_list *List, psize Count, str8 *Strings)
{
    for(u32 At = 0;
        At < Count;
        At++)
    {
        Str8ListAppend(List, Strings[At]);
    }
}

str8 Str8ListJoin(str8_list *List, psize BufferSize, u8 *Buffer, u8 Char)
{
    str8 Result = {};
    
    psize BufferIndex = 0;
    for(psize At = 0;
        At < List->Count;
        At++)
    {
        str8 *StringAt = List->Strings + At;
        
        Assert(BufferIndex + StringAt->Size < BufferSize);
        MemoryCopy(Buffer + BufferIndex, StringAt->Data, StringAt->Size);
        BufferIndex += StringAt->Size;
        if(Char)
        {
            Buffer[BufferIndex++] = Char;
        }
    }
    
    Result.Data = Buffer;
    Result.Size = BufferIndex;
    
    return Result;
}

int main(int ArgsCount, char *Args[])
{
    signal(SIGINT, LinuxSigINTHandler);
    LinuxChangeToExecutableDirectory(Args);
    
    b32 Rebuild = true;
    for(int ArgsIndex = 1;
        ArgsIndex < ArgsCount;
        ArgsIndex++)
    {
        if(!strcmp(Args[ArgsIndex], "norebuild"))
        {
            Rebuild = false;
        }
    }
    
    if(Rebuild)
    {
        printf("[self compile]\n");
        
        str8_list BuildCommand = {};
        {        
            u8 Buffer[4096] = {};
            BuildCommand.Strings = (str8 *)Buffer;
            BuildCommand.Capacity = ArrayCount(Buffer)/sizeof(str8);
        }
        
        b32 GCC = false;
        b32 Windows = false;
        b32 Clang = true;
        b32 Debug = true;
        b32 Release = false;
        
        // Common flags
        str8 CommonCompilerFlags = S8Lit("-DOS_LINUX=1 -fsanitize-trap -nostdinc++");
        str8 CommonWarningFlags = S8Lit("-Wall -Wextra -Wconversion -Wdouble-promotion -Wno-sign-conversion -Wno-sign-compare -Wno-double-promotion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-write-strings -Wno-pointer-arith -Wno-unused-parameter -Wno-unused-function");
        
        str8 LinuxLinkerFlags = S8Lit("-lpthread -lm");
        str8 WindowsLinkerFlags = S8Lit("-luser32 -lgdi32 -lwinmm");
        
        str8 Compiler = {};
        str8 Mode = {};
        
        if(0) {}
        else if(Release)
        {
            Mode = S8Lit("release");
        }
        else if(Debug)
        {
            Mode = S8Lit("debug");
        }
        
        if(0) {}
        else if(Clang)
        {
            Compiler = S8Lit("clang");
            Str8ListAppend(&BuildCommand, Compiler);
            if(Debug)
            {
                Str8ListAppend(&BuildCommand, S8Lit("-g -ggdb -g3"));
            }
            else if(Release)
            {
                Str8ListAppend(&BuildCommand, S8Lit("-O3"));
            }
            Str8ListAppend(&BuildCommand, CommonCompilerFlags);
            Str8ListAppend(&BuildCommand, S8Lit("-fdiagnostics-absolute-paths -ftime-trace"));
            Str8ListAppend(&BuildCommand, CommonWarningFlags);
            Str8ListAppend(&BuildCommand, S8Lit("-Wno-null-dereference -Wno-missing-braces -Wno-vla-cxx-extension -Wno-writable-strings -Wno-missing-designated-field-initializers -Wno-address-of-temporary -Wno-int-to-void-pointer-cast"));
        }
        else if(GCC)
        {
            Compiler = S8Lit("g++");
            Str8ListAppend(&BuildCommand, Compiler);
            
            if(Debug)
            {
                Str8ListAppend(&BuildCommand, S8Lit("-g -ggdb -g3"));
            }
            else if(Release)
            {
                Str8ListAppend(&BuildCommand, S8Lit("-O3"));
            }
            Str8ListAppend(&BuildCommand, CommonCompilerFlags);
            Str8ListAppend(&BuildCommand, CommonWarningFlags);
            Str8ListAppend(&BuildCommand, S8Lit("-Wno-cast-function-type -Wno-missing-field-initializers -Wno-int-to-pointer-cast"));
        }
        
        if(!Windows)
        {
            Str8ListAppend(&BuildCommand, LinuxLinkerFlags);
        }
        
        printf("%*s mode\n", (int)Mode.Size, Mode.Data);
        printf("%*s compile\n", (int)Compiler.Size, Compiler.Data);
        
        Str8ListAppend(&BuildCommand, S8Lit("-o cshell"));
        Str8ListAppend(&BuildCommand, S8Lit("../code/cshell.cpp"));
        
        u8 Buffer[4096] = {};
        str8 Command = Str8ListJoin(&BuildCommand, sizeof(Buffer), Buffer, ' ');
        
        //printf("%*s\n", (int)Command.Size, Command.Data);
        
        PlatformRunCommand(Command);
        
        // 2. Run without rebuilding
        PlatformAppendArgumentAndRun(ArgsCount, Args, "./cshell", "norebuild");
    }
    else
    {
        PlatformRunCommand(S8Lit("find . -type f"));
    }
    
    return 0;
}