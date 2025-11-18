//~ Libraries
// Standard library (TODO(luca): get rid of it) 
#include <stdio.h>
#include <string.h>

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
str8_list CommonBuildCommand(str8 StringsBuffer, b32 GCC, b32 Clang, b32 Debug)
{
    str8_list BuildCommand = {};
    
#if OS_WINDOWS
    b32 Windows = true;
#else
    b32 Windows = false;
#endif
    
#if OS_LINUX
    b32 Linux = true;
#else
    b32 Linux = false;
#endif
    
    // Exclusive arguments
    if(GCC) Clang = false;
    b32 Release = !Debug;
    
    BuildCommand.Strings = (str8 *)(StringsBuffer.Data);
    BuildCommand.Capacity = StringsBuffer.Size/sizeof(str8);
    
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
    
    if(Linux)
    {
        Str8ListAppend(&BuildCommand, LinuxLinkerFlags);
    }
    
    if(Windows)
    {
        ZeroMemory(&BuildCommand, sizeof(BuildCommand));
        BuildCommand.Strings = (str8 *)StringsBuffer.Data;
        BuildCommand.Capacity = StringsBuffer.Size/sizeof(str8);
        
        Str8ListAppend(&BuildCommand, S8Lit("cl"));
        Str8ListAppend(&BuildCommand, S8Lit(OS_Define));
        Str8ListAppend(&BuildCommand, S8Lit("-MTd -Gm- -nologo -GR- -EHa- -Oi -FC -Z7"));
        Str8ListAppend(&BuildCommand, S8Lit("-WX -W4 -wd4459 -wd4456 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -wd4996 -wd4389 -wd4244"));
        Str8ListAppend(&BuildCommand, S8Lit("-I..\\.."));
    }
    
    printf("%*s mode\n", (int)Mode.Size, Mode.Data);
    printf("%*s compile\n", (int)Compiler.Size, Compiler.Data);
    
    return BuildCommand;
}


//~ OS
#define OS_IMPLEMENTATION
#include "os.c"

internal void
ChangeToExecutableDirectory(char *Args[])
{
    char *ExePath = Args[0];
    s32 Length = (s32)CountCString(ExePath);
    char ExecutableDirPath[OS_PathMaxLength] = {};
    
    s32 LastSlash = 0;
    for(s32 At = 0;
        At < Length;
        At++)
    {
        if(ExePath[At] == OS_PathSeparator)
        {
            LastSlash = At;
        }
    }
    MemoryCopy(ExecutableDirPath, ExePath, LastSlash);
    ExecutableDirPath[LastSlash] = 0;
    
    OS_ChangeDirectory(ExecutableDirPath);
}