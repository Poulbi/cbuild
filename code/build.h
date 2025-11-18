//~ Libraries
// Standard library (TODO(luca): get rid of it) 
#include <stdio.h>
#include <string.h>

#if OS_LINUX
// POSIX
#include <unistd.h>
// Linux
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <linux/limits.h>
#endif

// External
#include <lr/lr_types.h>
// Internal
#include "os.c"

//~ Macros
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define MemoryCopy memcpy

//~ Types
struct str8
{
    umm Size;
    u8 *Data;
};
#define S8Lit(String) (str8){.Size = (sizeof((String)) - 1), .Data = (u8 *)(String)}
typedef struct str8 str8;

struct str8_list
{
    str8 *Strings;
    umm Count;
    umm Capacity;
};
typedef struct str8_list str8_list;

//~ Global variables
global_variable u8 OutputBuffer[Kilobytes(64)] = {};

//~ Strings
umm CountCString(char *String)
{
    umm Result = 0;
    
    while(*String++) Result++;
    
    return Result;
}

b32 IsWhiteSpace(u8 Char)
{
    b32 Result = (Char == ' ' || Char == '\t' || Char == '\n');
    return Result;
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
void _Str8ListAppendMultiple(str8_list *List, umm Count, str8 *Strings)
{
    for(u32 At = 0;
        At < Count;
        At++)
    {
        Str8ListAppend(List, Strings[At]);
    }
}

str8 Str8ListJoin(str8_list List, umm BufferSize, u8 *Buffer, u8 Char)
{
    str8 Result = {};
    
    umm BufferIndex = 0;
    for(umm At = 0;
        At < List.Count;
        At++)
    {
        str8 *StringAt = List.Strings + At;
        
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

//~ Helpers
str8_list CommonBuildCommand(b32 GCC, b32 Clang, b32 Debug)
{
    str8_list BuildCommand = {};
    
    // Exclusive arguments
    if(GCC) Clang = false;
    b32 Release = !Debug;
    
    u8 StringsBuffer[Kilobytes(4)] = {};
    BuildCommand.Strings = (str8 *)StringsBuffer;
    BuildCommand.Capacity = ArrayCount(StringsBuffer)/sizeof(str8);
    
    
    str8 CommonCompilerFlags = S8Lit(OS_Define "-fsanitize-trap -nostdinc++");
    str8 CommonWarningFlags = S8Lit("-Wall -Wextra -Wconversion -Wdouble-promotion -Wno-sign-conversion -Wno-sign-compare -Wno-double-promotion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-write-strings -Wno-pointer-arith -Wno-unused-parameter -Wno-unused-function");
    
    str8 LinuxLinkerFlags = S8Lit("-lpthread -lm");
    
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
    
    Str8ListAppend(&BuildCommand, LinuxLinkerFlags);
    
    printf("%*s mode\n", (int)Mode.Size, Mode.Data);
    printf("%*s compile\n", (int)Compiler.Size, Compiler.Data);
    
    return BuildCommand;
}


//~ OS
#define OS_IMPLEMENTATION
#include "os.c"