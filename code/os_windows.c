// Windows OS layer

#ifndef OS_WINDOWS_C
#define OS_WINDOWS_C

//~ API
#define os_command_result windows_command_result

#define OS_RunCommandString WindowsRunCommandString
#define OS_RebuildSelf WindowsRebuildSelf
#define OS_ChangeToExecutableDirectory WindowsChangeToExecutableDirectory

#define Assert(Expression) if(!(Expression)) { *(char *)0 = 0;; } 
#define OS_Define "-DOS_WINDOWS=1"

//~ Types
struct windows_command_result
{
    int Nothing;
};
typedef struct windows_command_result windows_command_result;

#endif // OS_WINDOWS_C

#ifdef OS_WINDOWS_IMPLEMENTATION
//~ Functions 
internal windows_command_result
WindowsRunCommandString()
{
    windows_command_result Result = {0};
    
    return Result;
}

internal void
WindowsChangeToExecutableDirectory(char *Args[])
{
    
}

internal void
WindowsRebuildSelf(int ArgsCount, char *Args[], char *Env[])
{
    
}

#endif // OS_WINDOWS_IMPLEMENTATION