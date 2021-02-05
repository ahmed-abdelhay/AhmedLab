#include <stddef.h>
#include <assert.h>
#include <stdint.h>

template <typename F> struct privDefer
{
    F f;
    privDefer(F f) : f(f)
    {
    }
    ~privDefer()
    {
        f();
    }
};
template <typename F> privDefer<F> defer_func(F f)
{
    return privDefer<F>(f);
}
#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })
#define PROMPT_TEXT ">>"
#define MAX_VARIABLES 255
#define MAX_VARIABLE_NAME 30

struct Color
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

static const Color WHITE = { 255, 255, 255 };
static const Color RED = { 255, 0, 0 };
static const Color GREEN = { 0, 255, 0 };
static const Color BLUE = { 0, 0, 255 };
static const Color YELLOW = { 255, 255, 0 };

//------------------------Platform---------------------------
struct MemoryBlock
{
    uint8_t* data = NULL;
    size_t size = 0;
};

// Memory
void Deallocate(MemoryBlock& block);
MemoryBlock Allocate(size_t size);
void ZeroMemory(MemoryBlock block);
// File IO
bool ReadFile(const char* fileName, MemoryBlock& result);
bool WriteFile(const char* fileName, const MemoryBlock data);
// IO
bool PrintToConsole(const char* text, Color c = WHITE);
void ReadFromConsole(char* buffer, size_t bufferSize);
//-----------------------------------------------------------

//----------------------containers---------------------------
template <typename T, size_t N>
struct StaticArray
{
    T vals[N] = {};
    size_t count = 0;

    T* data()
    {
        return vals;
    }
    const T* data() const
    {
        return vals;
    }
    size_t capacity() const
    {
        return N;
    }
    size_t size() const
    {
        return count;
    }
    T* begin()
    {
        return data();
    }
    T* end()
    {
        return data() + size();
    }
    const T* begin() const
    {
        return data();
    }
    const T* end() const
    {
        return data() + size();
    }
    T& operator[](size_t i)
    {
        assert(i < size());
        return data()[i];
    }
    const T& operator[](size_t i) const
    {
        assert(i < size());
        return data()[i];
    }
    void clear()
    {
        count = 0;
    }
    void push_back(const T& d)
    {
        assert(size() < N);
        data()[count++] = d;
    }
    void pop_back()
    {
        count--;
    }
    void resize(size_t s)
    {
        assert(s <= N);
        count = s;
    }
};

template <typename T>
struct Array
{
    MemoryBlock block;
    size_t count = 0;

    Array() = default;
    ~Array()
    {
        if (block.data)
        {
            Deallocate(block);
        }
    }
    Array(const Array& s)
    {
        block = Allocate(s.block.size);
        memcpy(block.data, s.block.data, block.size);
        count = s.count;
    }
    Array& operator=(const Array& s)
    {
        block = Allocate(s.block.size);
        memcpy(block.data, s.block.data, block.size);
        count = s.count;
        return *this;
    }
    Array(Array&& s) noexcept
    {
        block = s.block;
        count = s.count;
        s.block = MemoryBlock{};
        s.count = 0;
    }
    Array& operator=(Array&& s) noexcept
    {
        block = s.block;
        count = s.count;
        s.block = MemoryBlock{};
        s.count = 0;
        return *this;
    }
    T* data()
    {
        return (T*)block.data;
    }
    const T* data() const
    {
        return (const T*)block.data;
    }
    size_t capacity() const
    {
        return block.size / sizeof(T);
    }
    size_t size() const
    {
        return count;
    }
    T* begin()
    {
        return data();
    }
    T* end()
    {
        return data() + size();
    }
    const T* begin() const
    {
        return data();
    }
    const T* end() const
    {
        return data() + size();
    }
    T& operator[](size_t i)
    {
        assert(i < size());
        return data()[i];
    }
    const T& operator[](size_t i) const
    {
        assert(i < size());
        return data()[i];
    }
    void clear()
    {
        count = 0;
    }
    void push_back(const T& d)
    {
        if (size() >= capacity())
        {
            const size_t s = size();
            resize(s ? 2 * s : 8);
            count = s;
        }
        data()[count++] = d;
    }
    void pop_back()
    {
        count--;
    }
    void resize(size_t s)
    {
        if (capacity() < s)
        {
            reserve(s);
        }
        count = s;
    }
    void reserve(size_t s)
    {
        if (capacity() < s)
        {
            MemoryBlock newBlock = Allocate(s * sizeof(T));
            if (block.data)
            {
                memcpy(newBlock.data, block.data, block.size);
                Deallocate(block);
            }
            block = newBlock;
        }
    }
};

template <typename T>
void Swap(T& v0, T& v1)
{
    T t = v0;
    v0 = v1;
    v1 = t;
}
//-----------------------------------------------------------

//----------------- Strings----------------------------------
struct Buffer
{
    const char* text = 0;
    size_t size = 0;
    size_t cursor = 0;
};

size_t StringLength(const char* text);
char* DuplicateString(const char* text);
bool CompareStrings(const char* s0, const char* s1);
Buffer CreateBufferFromString(const char* string);
//-----------------------------------------------------------

//------------------Math-------------------------------------
struct Matrix
{
    static const size_t stackBufferSize = 9;
    double stackBuffer[stackBufferSize] = {};
    size_t rows = 0;
    size_t cols = 0;
    double* data = NULL;
};

double& At(Matrix& m, size_t i, size_t j);
const double& At(const Matrix& m, size_t i, size_t j);
void GetRow(const Matrix& m, size_t row, double* result);
void GetCol(const Matrix& m, size_t col, double* result);
Matrix CreateMatrix(size_t rows, size_t cols);
void FreeMatrix(Matrix& m);
Matrix Zeros(size_t rows, size_t cols);
Matrix Ones(size_t rows, size_t cols);
Matrix Eye(size_t rows, size_t cols);
bool CanMultiply(const Matrix& m0, const Matrix& m1);
bool CanAdd(const Matrix& m0, const Matrix& m1);
bool CanSubtract(const Matrix& m0, const Matrix& m1);
Matrix Multiply(const Matrix& m0, double scalar);
Matrix Add(const Matrix& m0, double scalar);
Matrix Subtract(const Matrix& m0, double scalar);
Matrix Multiply(const Matrix& m0, const Matrix& m1);
Matrix Add(const Matrix& m0, const Matrix& m1);
Matrix Subtract(const Matrix& m0, const Matrix& m1);
Matrix Abs(const Matrix& m);
Matrix Sin(const Matrix& m);
Matrix Cos(const Matrix& m);
Matrix Tan(const Matrix& m);
Matrix ASin(const Matrix& m);
Matrix ACos(const Matrix& m);
Matrix ATan(const Matrix& m);
double DotProduct(const double* v0, const double* v1, size_t size);
//-----------------------------------------------------------

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
enum class TokenType
{
    KEY_WORD_IF,
    KEY_WORD_WHILE,
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
    TokenType type;
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
