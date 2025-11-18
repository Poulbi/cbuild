#include "build.h"

int main(int ArgsCount, char *Args[], char *Env[])
{
    OS_ChangeToExecutableDirectory(Args);
    OS_RebuildSelf(ArgsCount, Args, Env);
    
    // [Code goes here]
    
    return 0;
}
