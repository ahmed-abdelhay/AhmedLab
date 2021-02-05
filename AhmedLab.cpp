#define GEDO_IMPLEMENTATION
#include "Gedo.h"
#include "AhmedLab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* DuplicateString(const char* text)
{
    const size_t len = StringLength(text);
    char* result = (char*)Allocate(len + 1).data;
    for (size_t i = 0; i < len; i++)
    {
        result[i] = text[i];
    }
    return result;
}

void PrintMessage(MessageLevel level, const char* message)
{
    Color c;
    switch (level)
    {
    case MessageLevel::INFO:
        c = BLUE;
        break;
    case MessageLevel::WARNING:
        c = YELLOW;
        break;
    case MessageLevel::ERROR:
        c = RED;
        break;
    }
    PrintToConsole(message, c);
    PrintToConsole("\n");
}

const Variable* FindVariable(const State& state, const char* name)
{
    for (size_t i = 0; i < state.varsCount; i++)
    {
        if (CompareStrings(name, state.vars[i].name))
        {
            return &state.vars[i];
        }
    }
    return NULL;
}

Variable* FindVariable(State& state, const char* name)
{
    for (size_t i = 0; i < state.varsCount; i++)
    {
        if (CompareStrings(name, state.vars[i].name))
        {
            return &state.vars[i];
        }
    }
    return NULL;
}

void PrintVariable(const Variable& var)
{
    char buffer[200] = {};
    MemoryBlock block;
    block.size = sizeof(buffer);
    block.data = (uint8_t*)buffer;

    sprintf(buffer, "Name: %s\n", var.name);
    PrintToConsole(buffer);
    ZeroMemoryBlock(block);

    sprintf(buffer, "Size = (%zu X %zu).\n", var.value.rows, var.value.cols);
    PrintToConsole(buffer);
    ZeroMemoryBlock(block);

    sprintf(buffer, "Data = [");
    PrintToConsole(buffer);
    ZeroMemoryBlock(block);

    for (size_t i = 0; i < var.value.rows; ++i)
    {
        for (size_t j = 0; j < var.value.cols; ++j)
        {
            if (j == (var.value.cols - 1))
            {
                sprintf(buffer, "%f", At(var.value, i, j));
            }
            else
            {
                sprintf(buffer, "%f , ", At(var.value, i, j));
            }
            PrintToConsole(buffer);
            ZeroMemoryBlock(block);
        }
        if (i != (var.value.rows - 1))
        {
            PrintToConsole("\n        ");
        }
        else
        {
            PrintToConsole("]\n");
        }
    }
}

Variable* AddVariable(State& state, const char* name, Matrix data)
{
    Variable* var = FindVariable(state, name);
    if (var)
    {
        FreeMatrix(var->value);
        var->value = data;
        return var;
    }
    else
    {
        Variable newVar = {};
        newVar.value = data;
        newVar.name = DuplicateString(name);
        state.vars[state.varsCount++] = newVar;
        return &state.vars[state.varsCount - 1];
    }
}

void DeleteVariable(State& state, const char* name)
{
    Variable* var = FindVariable(state, name);
    if (var)
    {
        Variable& lastVar = state.vars[state.varsCount - 1];
        Swap(lastVar.name, var->name);
        Swap(lastVar.value, var->value);
        FreeMatrix(lastVar.value);
        MemoryBlock block;
        block.data = (uint8_t*)lastVar.name;
        block.size = StringLength(lastVar.name) + 1;
        Deallocate(block);
        state.varsCount--;
    }
}

void ProcessInput(State& state, const char* input)
{
    Buffer buffer = CreateBufferFromString(input);
    const LexerResult lexResults = Tokenize(buffer);
    if (!lexResults.success)
    {
        PrintMessage(MessageLevel::ERROR, "Error parsing the input text:\n");
        PrintToConsole(input);
        PrintToConsole("\n");
        for (size_t i = 0; i < lexResults.errorLocation; ++i)
        {
            PrintToConsole(" ");
        }
        PrintToConsole("^\n");
    }
}

LexerResult Tokenize(Buffer& buffer)
{
    LexerResult result;
    result.success = false;
    result.errorLocation = 2;
    return result;
}
