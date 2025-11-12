#include "cshell.h"

int main(int ArgsCount, char *Args[], char *Env[])
{
    LinuxChangeToExecutableDirectory(Args);
    LinuxRebuildSelf(ArgsCount, Args, Env);
    
    // [Code goes here]
    
    return 0;
}