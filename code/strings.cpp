psize CountCString(char *String)
{
    psize Result = 0;
    
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
void _Str8ListAppendMultiple(str8_list *List, psize Count, str8 *Strings)
{
    for(u32 At = 0;
        At < Count;
        At++)
    {
        Str8ListAppend(List, Strings[At]);
    }
}

str8 Str8ListJoin(str8_list *List, psize BufferSize, u8 *Buffer, u8 Char)
{
    str8 Result = {};
    
    psize BufferIndex = 0;
    for(psize At = 0;
        At < List->Count;
        At++)
    {
        str8 *StringAt = List->Strings + At;
        
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
