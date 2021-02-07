#include "AhmedLab.h"
#define PROMPT_TEXT ">>"

int main(int argc, char const* argv[])
{
    if (argc == 2)
    {
        MemoryBlock fileData = ReadFile(argv[1]);
        if (fileData.size)
        {
            defer(Deallocate(fileData));
            State state;
            ProcessInput(state, (const char*)fileData.data);
        }
        else
        {
            PrintToConsole("Can't read input file.\n");
        }
        return 0;
    }

    State state;
    char input[500] = {};
    for (;;)
    {
        PrintToConsole(PROMPT_TEXT, ConsoleColor::GREEN);
        ReadFromConsole(input, sizeof(input));
        ProcessInput(state, input);
    }
    return 0;
}