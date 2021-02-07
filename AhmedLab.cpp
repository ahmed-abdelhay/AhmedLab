#include "AhmedLab.h"
#include <stdio.h>

void PrintMessage(MessageLevel level, const char* message)
{
    Color c;
    switch (level)
    {
    case MessageLevel::INFO:    c = BLUE;   break;
    case MessageLevel::WARNING: c = YELLOW; break;
    case MessageLevel::ERROR:   c = RED;    break;
    }
    PrintToConsole(message, c);
    PrintToConsole("\n");
}

const Variable* FindVariable(const State& state, const char* name)
{
    for (const Variable& var : state.vars)
    {
        if (CompareStrings(name, var.name))
        {
            return &var;
        }
    }
    return NULL;
}

Variable* FindVariable(State& state, const char* name)
{
    for (Variable& var : state.vars)
    {
        if (CompareStrings(name, var.name))
        {
            return &var;
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

    PrintToConsole("Name: ");
    for (char c : var.name)
    {
        PrintToConsole(c);
    }
    PrintToConsole("\n");
    ZeroMemoryBlock(block);

    PrintToConsole("\n");

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
        newVar.name = CreateString(name);
        state.vars.push_back(newVar);
        return &state.vars[state.vars.size() - 1];
    }
}

void DeleteVariable(State& state, const char* name)
{
    Variable* var = FindVariable(state, name);
    if (var)
    {
        Variable& lastVar = state.vars[state.vars.size() - 1];
        Swap(lastVar.name, var->name);
        Swap(lastVar.value, var->value);
        FreeMatrix(lastVar.value);
        state.vars.pop_back();
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

namespace
{
    struct TokenString
    {
        TokenType type;
        const char* string;
    };

    static const TokenString tokensStrings[]
    {
        {TokenType::KEYWORD_IF, "if"},
        {TokenType::KEYWORD_ELIF, "elif"},
        {TokenType::KEYWORD_FUNC, "func"},
        {TokenType::KEYWORD_ELSE, "else"},
        {TokenType::KEYWORD_WHILE, "while"},
        {TokenType::LOGICAL_GTE, ">="},
        {TokenType::LOGICAL_LTE, "<="},
        {TokenType::LOGICAL_EQUALS, "=="},
        {TokenType::LOGICAL_NOT_EQUALS, "!="},
        {TokenType::LOGICAL_LT, "<"},
        {TokenType::LOGICAL_GT, ">"},
        {TokenType::LOGICAL_NOT, "!"},
        {TokenType::LOGICAL_AND, "&&"},
        {TokenType::LOGICAL_OR, "||"},
        {TokenType::OPERATOR_PLUS, "+"},
        {TokenType::OPERATOR_MINUS, "-"},
        {TokenType::OPERATOR_MULTIPLY, "*"},
        {TokenType::OPERATOR_DIVIDE, "/"},
        {TokenType::OPERATOR_ASSIGN, "="},
        {TokenType::LEFT_PARAN, "("},
        {TokenType::RIGHT_PARAN, ")"},
        {TokenType::COMMA, ","},
        {TokenType::SEMICOL, ";"},
        {TokenType::LEFT_SQUARE_BRACKET, "["},
        {TokenType::RIGHT_SQUARE_BRACKET, "]"}
    };
}

LexerResult Tokenize(Buffer& buffer)
{
    const size_t count = ArrayCount(tokensStrings);
    LexerResult result;
    while (buffer.cursor < buffer.size)
    {
        Token token;
        SkipSingleLineComment(buffer);
        SkipWhiteSpaces(buffer);
        if (buffer.cursor >= buffer.size)
        {
            goto NEXT_ITERATION;
        }

        for (size_t i = 0; i < count; ++i)
        {
            if (CompareWordAndSkip(buffer, tokensStrings[i].string))
            {
                token.type = tokensStrings[i].type;
                result.tokens.push_back(token);
                goto NEXT_ITERATION;
            }
        }
        if (ParseIdentifier(buffer, token.name))
        {
            token.type = TokenType::IDENTIFIER;
            result.tokens.push_back(token);
            goto NEXT_ITERATION;
        }
        else if (ParseStringLiteral(buffer, token.stringLiteral))
        {
            token.type = TokenType::STRING_LITERAL;
            result.tokens.push_back(token);
            goto NEXT_ITERATION;
        }
        else if (ParseFloat(buffer, token.numericLiteral))
        {
            token.type = TokenType::NUMERIC_LITERAL;
            result.tokens.push_back(token);
            goto NEXT_ITERATION;
        }
        else
        {
            result.success = false;
            result.errorLocation = buffer.cursor;
            return result;
        }
    NEXT_ITERATION:;
    }
    result.success = true;
    return result;
}
