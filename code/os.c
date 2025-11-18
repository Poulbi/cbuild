#ifdef OS_IMPLEMENTATION
# if OS_LINUX
#  define OS_LINUX_IMPLEMENTATION
# elif OS_WINDOWS
#  define OS_WINDOWS_IMPLEMENTATION
# endif
#endif

#if OS_LINUX
# include "os_linux.c"
#elif OS_WINDOWS
# include "os_windows.c"
#else
# error "OS not supported or not defined."
#endif