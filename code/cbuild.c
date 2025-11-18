#include "cbuild.h"

//~ Global variables
global_variable u8 OutputBuffer[Kilobytes(64)] = {};
global_variable u8 StringsBuffer[Kilobytes(4)] = {};

int main(int ArgsCount, char *Args[], char *Env[])
{
    str8 Buffer = {0};
    Buffer.Size = ArrayCount(StringsBuffer);
    Buffer.Data = StringsBuffer;
    str8 Output = {0};
    Output.Size = ArrayCount(OutputBuffer);
    Output.Data = OutputBuffer;
    
    ChangeToExecutableDirectory(Args);
    
    OS_RebuildSelf(Buffer, Output, ArgsCount, Args, Env);
    
    printf("Done.\n");
    
    return 0;
}
