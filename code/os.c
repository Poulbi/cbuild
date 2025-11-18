#ifdef OS_IMPLEMENTATION
# if OS_LINUX
#  define OS_LINUX_IMPLEMENTATION
# elif OS_WINDOWS
#  define OS_WINDOWS_IMPLEMENTATION
# endif
#endif

#ifndef OS_C
#define OS_C

#if OS_WINDOWS
# define os_command_result windows_command_result
# define OS_RunCommandString WindowsRunCommandString
# define OS_RebuildSelf WindowsRebuildSelf
# define OS_ChangeDirectory WindowsChangeDirectory

# define Assert(Expression) if(!(Expression)) { *(char *)0 = 0;; } 

# define OS_Define "-DOS_WINDOWS=1"
# define OS_PathMaxLength 256
# define OS_PathSeparator '\\'

#elif OS_LINUX
#define os_command_result linux_command_result
#define OS_RunCommandString LinuxRunCommandString
#define OS_ChangeDirectory LinuxChangeDirectory
#define OS_RebuildSelf LinuxRebuildSelf

#define Assert(Expression) if(!(Expression)) { __asm__ volatile("int3"); } 

#define OS_Define "-DOS_LINUX=1"
#define OS_PathMaxLength PATH_MAX
#define OS_PathSeparator '/'

#else
# error "OS not supported or not defined."
#endif

#endif // OS_C

#if OS_LINUX
# include "os_linux.c"
#elif OS_WINDOWS
# include "os_windows.c"
#else
# error "OS not supported or not defined."
#endif