#ifdef OS_WINDOWS
# define RADDBG_MARKUP_IMPLEMENTATION
#else
# define RADDBG_MARKUP_STUBS
#endif
#include "raddbg_markup.h"

#include "cbuild.h"

//~ Global variables
global_variable u8 OutputBuffer[Kilobytes(64)] = {};
global_variable u8 TempBuffer[Kilobytes(4)] = {};

#if 0
raddbg_type_view(str8, no_addr(cast(char *)(array(Data, Size))));
raddbg_type_view(str8_list, array(Strings, Count));
#endif

int main(int ArgsCount, char *Args[], char *Env[])
{
    str8 Temp = {0};
    Temp.Size = ArrayCount(TempBuffer);
    Temp.Data = TempBuffer;
    str8 Output = {0};
    Output.Size = ArrayCount(OutputBuffer);
    Output.Data = OutputBuffer;
    
    ChangeToExecutableDirectory(Args);
    
    OS_RebuildSelf(Temp, Output, ArgsCount, Args, Env);
    
    return 0;
}