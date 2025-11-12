//~ Libraries
// Standard library (TODO(luca): get rid of it) 
#include <stdio.h>
#include <string.h>
// POSIX
#include <unistd.h>
// Linux
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <linux/limits.h>
// External
#include "libs/lr/lr_types.h"

//~ Macros
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Assert(Expression) if(!(Expression)) { __asm__ volatile("int3"); } 
#define MemoryCopy memcpy

#include "cshell.h" 
#include "strings.cpp"

#include "linux.cpp"

str8_list CommonBuildCommand()
{
    str8_list BuildCommand = {};
    u8 StringsBuffer[4096] = {};
    BuildCommand.Strings = (str8 *)StringsBuffer;
    BuildCommand.Capacity = ArrayCount(StringsBuffer)/sizeof(str8);
    
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
    
    return BuildCommand;
}

int main(int ArgsCount, char *Args[], char *Env[])
{
    LinuxChangeToExecutableDirectory(Args);
    
    u8 OutputBuffer[Kilobytes(64)] = {};
    
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
    
#if 1    
    Rebuild = false;
#endif
    
    if(ForceRebuild || Rebuild)
    {
        printf("[self compile]\n");
        str8_list BuildCommandList = CommonBuildCommand();
        Str8ListAppendMultiple(&BuildCommandList,  S8Lit("-o cshell"), S8Lit("../code/cshell.cpp"));
        str8 BuildCommand = Str8ListJoin(BuildCommandList, sizeof(OutputBuffer), OutputBuffer, ' ');
        
        //printf("%*s\n", (int)BuildCommand.Size, BuildCommand.Data);
        LinuxRebuildSelf(BuildCommand, ArgsCount, Args, Env);
    }
    else
    {    
        
    }
    
    return 0;
}