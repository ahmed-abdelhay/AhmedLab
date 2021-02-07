/*
TODO:
- Finish lexer.
- Finish parser.
- Finish code interpretor.
- Add GUI using imgui.
- Add builtin functions.
- Add imread, imwrite support.
- Add image rendering support.
- Add plot support.
- Add read, write support.
*/

#pragma once
#include "Gedo.h"

using namespace gedo;

//-----------------------state-------------------------------
struct Variable
{
    String name;
    Matrix value;
};

struct State
{
    Array<Variable> vars;
};

enum class MessageLevel
{
    INFO, WARNING, ERROR
};

void PrintMessage(MessageLevel level, const char* message);
void ProcessInput(State& state, const char* input);

const Variable* FindVariable(const State& state, const char* name);
Variable* FindVariable(State& state, const char* name);
void PrintVariable(const Variable& var);
Variable* AddVariable(State& state, const char* name, Matrix data);
void DeleteVariable(State& state, const char* name);
//-----------------------------------------------------------

//---------------------------Parsing-------------------------
enum class TokenType
{
    KEYWORD_IF,
    KEYWORD_ELIF,
    KEYWORD_FUNC,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    IDENTIFIER,
    NUMERIC_LITERAL,
    STRING_LITERAL,
    // logical operators
    LOGICAL_LT,             // <
    LOGICAL_GT,             // >
    LOGICAL_GTE,            // >=
    LOGICAL_LTE,            // <=
    LOGICAL_EQUALS,         // ==
    LOGICAL_NOT_EQUALS,     // !=
    LOGICAL_NOT,
    LOGICAL_AND,
    LOGICAL_OR,
    // operators
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_ASSIGN,
    //
    LEFT_PARAN,
    RIGHT_PARAN,
    COMMA,
    SEMICOL,
    LEFT_SQUARE_BRACKET,
    RIGHT_SQUARE_BRACKET
};

struct Token
{
    TokenType type;
    struct
    {
        String name;            // when type == IDENTIFIER.
        double numericLiteral;  // when type == NUMERIC_LITERAL.
        String stringLiteral;   // when type == STRING_LITERAL.
    };
};

struct LexerResult
{
    Array<Token> tokens;
    bool success = false;
    size_t errorLocation = 0;
};

LexerResult Tokenize(Buffer& buffer);

//-----------------------------------------------------------
