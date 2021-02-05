#include "Gedo.h"
#undef ERROR

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

#define PROMPT_TEXT ">>"
#define MAX_VARIABLES 255
#define MAX_VARIABLE_NAME 30
using namespace gedo;

//-----------------------state-------------------------------
struct Variable
{
    char* name = NULL;
    Matrix value;
};

struct State
{
    Variable vars[MAX_VARIABLES] = {};
    size_t varsCount = 0;
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
enum class ALTokenType
{
    KEYWORD_IF,
    KEYWORD_WHILE,
    IDENTIFIER,
    LITERAL,
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
    ALTokenType type;
    union
    {
        char name[MAX_VARIABLE_NAME];
        double value;
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
