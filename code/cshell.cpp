#include <stdio.h>

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct string
{
    int Size;
    char *Data;
};
#define S(String) (string){.Size = (sizeof((String)) - 1), .Data = (String)}

#define PipeCommands(...) \
do \
{ \
command Values[] = { __VA_ARGS__, }; \
_PipeCommands(ArrayCount(Values), Values); \
} while(0);

struct command
{
    string Text;
};
#define CMD(String) (command){.Text = (String)}
#define CMD_S(String) (command){.Text = S((String))}

command PipeCommand(command Command1, command Command2)
{
    command Result = {};
    return Result;
}

command _PipeCommands(int Count, command *Commands)
{
    command Result = Commands[0];
    for(int Index = 1;
        Index < Count;
        Index += 1)
    {
        Result = PipeCommand(Result, Commands[Index]);
    }
    
    return Result;
}

int main(void)
{
    PipeCommands(CMD_S("find ."), CMD_S("grep '.cpp$'"), CMD_S("nl"));
    
    command Command = CMD_S("find .");
    Command = PipeCommand(Command, CMD_S("grep '.cpp$'"));
    Command = PipeCommand(Command, CMD_S("nl"));
    
    return 0;
}