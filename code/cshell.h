/* date = October 28th 2025 0:38 pm */

#ifndef CSHELL_H
#define CSHELL_H

struct str8
{
    umm Size;
    u8 *Data;
};
#define S8Lit(String) (str8){.Size = (sizeof((String)) - 1), .Data = (u8 *)(String)}

struct str8_list
{
    str8 *Strings;
    umm Count;
    umm Capacity;
};

struct linux_command_result
{
    b32 Error;
    
    umm StdoutBytesToRead;
    b32 StdinBytesToWrite;
    umm StderrBytesToRead;
    int Stdout;
    int Stdin;
    int Stderr;
};


#endif //CSHELL_H
