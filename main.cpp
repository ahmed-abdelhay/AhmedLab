#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

template <typename T>
void Swap(T& v0, T& v1)
{
    T t = v0;
    v0 = v1;
    v1 = t;
}

template<typename F>
struct privDefer
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

template<typename F>
privDefer<F> defer_func(F f)
{
    return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })

#define internal static

// Memory
// TODO(Ahmed): Add allocator.
internal void
Deallocate(void* data)
{
    free(data);
}
internal void*
Allocate(size_t size)
{
    return calloc(1, size);
}
// End memory.

//Strings.
internal size_t
StringLength(const char* text)
{
    size_t result = 0;
    while (*text++)
    {
        result++;
    }
    return result;
}

internal char*
DuplicateString(const char* text)
{
    const size_t len = StringLength(text);
    char* result = (char*)Allocate(len + 1);
    for (size_t i = 0; i < len; i++)
    {
        result[i] = text[i];
    }
    return result;
}

internal bool
CompareStrings(const char* s0, const char* s1)
{
    const size_t len0 = StringLength(s0);
    const size_t len1 = StringLength(s1);
    if (len0 == len1)
    {
        for (size_t i = 0; i < len0; i++)
        {
            if (s0[i] != s1[i])
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

// end strings.

// Math.
struct Matrix
{
    size_t dims[2] = { 0, 0 };
    double* data = NULL;
};

internal double&
At(Matrix& m, size_t i, size_t j)
{
    return m.data[m.dims[0] * i + j];
}

internal const double&
At(const Matrix& m, size_t i, size_t j)
{
    return m.data[m.dims[0] * i + j];
}

internal void
GetRow(const Matrix& m, size_t row, double* result)
{
    for (size_t i = 0; i < m.dims[1]; ++i)
    {
        result[i] = At(m, row, i);
    }
}

internal double
DotProduct(const double* v0, const double* v1, size_t size)
{
    double result = 0;
    for (size_t i = 0; i < size; i++)
    {
        result += v0[i] * v1[i];
    }
    return result;
}

internal void
GetCol(const Matrix& m, size_t col, double* result)
{
    for (size_t i = 0; i < m.dims[0]; ++i)
    {
        result[i] = At(m, i, col);
    }
}

internal Matrix
CreateMatrix(size_t rows, size_t cols)
{
    Matrix result;
    result.dims[0] = rows;
    result.dims[1] = cols;
    result.data = (double*)Allocate(rows * cols);
    return result;
}

internal Matrix
Zeros(size_t rows, size_t cols)
{
    Matrix result = CreateMatrix(rows, cols);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = 0;
    }
    return result;
}

internal Matrix
Ones(size_t rows, size_t cols)
{
    Matrix result = CreateMatrix(rows, cols);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = 1;
    }
    return result;
}

internal Matrix
Eye(size_t rows, size_t cols)
{
    Matrix result = CreateMatrix(rows, cols);
    for (size_t i = 0; i < result.dims[0]; ++i)
    {
        for (size_t j = 0; j < result.dims[1]; ++j)
        {
            At(result, i, j) = (i == j) ? 1 : 0;
        }
    }
    return result;
}

internal void
FreeMatrix(Matrix& m)
{
    if (m.data)
    {
        Deallocate(m.data);
        m.data = NULL;
    }
}

internal bool
CanMultiply(const Matrix& m0, const Matrix& m1)
{
    return m0.dims[1] == m1.dims[0];
}

internal bool
CanAdd(const Matrix& m0, const Matrix& m1)
{
    return m0.dims[0] == m1.dims[0] && m0.dims[1] == m1.dims[1];
}

internal bool
CanSubtract(const Matrix& m0, const Matrix& m1)
{
    return CanAdd(m0, m1);
}

internal Matrix
Multiply(const Matrix& m0, const Matrix& m1)
{
    assert(CanMultiply(m0, m1));
    Matrix result = CreateMatrix(m0.dims[0], m1.dims[1]);
    double* col = (double*)Allocate(m0.dims[1]);
    double* row = (double*)Allocate(m0.dims[1]);
    defer(Deallocate(col));
    defer(Deallocate(row));
    for (size_t i = 0; i < result.dims[0]; ++i)
    {
        for (size_t j = 0; j < result.dims[1]; ++j)
        {
            GetRow(m0, i, row);
            GetCol(m1, j, col);
            At(result, i, j) = DotProduct(row, col, m0.dims[1]);
        }
    }
    return result;
}

internal Matrix
Add(const Matrix& m0, const Matrix& m1)
{
    assert(CanAdd(m0, m1));
    Matrix result = CreateMatrix(m0.dims[0], m0.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = m0.data[i] + m1.data[i];
    }
    return result;
}

internal Matrix
Subtract(const Matrix& m0, const Matrix& m1)
{
    assert(CanSubtract(m0, m1));
    Matrix result = CreateMatrix(m0.dims[0], m0.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = m0.data[i] - m1.data[i];
    }
    return result;
}

internal Matrix
Abs(const Matrix& m)
{
    Matrix result = CreateMatrix(m.dims[0], m.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = fabs(m.data[i]);
    }
    return result;
}

internal Matrix
Sin(const Matrix& m)
{
    Matrix result = CreateMatrix(m.dims[0], m.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = sin(m.data[i]);
    }
    return result;
}

internal Matrix
Cos(const Matrix& m)
{
    Matrix result = CreateMatrix(m.dims[0], m.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = cos(m.data[i]);
    }
    return result;
}

internal Matrix
Tan(const Matrix& m)
{
    Matrix result = CreateMatrix(m.dims[0], m.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = tan(m.data[i]);
    }
    return result;
}

internal Matrix
ASin(const Matrix& m)
{
    Matrix result = CreateMatrix(m.dims[0], m.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = asin(m.data[i]);
    }
    return result;
}

internal Matrix
ACos(const Matrix& m)
{
    Matrix result = CreateMatrix(m.dims[0], m.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = acos(m.data[i]);
    }
    return result;
}

internal Matrix
ATan(const Matrix& m)
{
    Matrix result = CreateMatrix(m.dims[0], m.dims[1]);
    for (size_t i = 0; i < result.dims[0] * result.dims[1]; ++i)
    {
        result.data[i] = atan(m.data[i]);
    }
    return result;
}


// end math.

// state
#define MAX_VARIABLES 255

struct Variable
{
    char* name = NULL;
    Matrix data;
};

struct State
{
    Variable vars[MAX_VARIABLES] = {};
    size_t varsCount = 0;
};

enum class MessageLevel
{
    INFO,
    WARNING,
    ERROR,
    FATAL
};

internal void
PrintMessage(MessageLevel level, const char* message)
{
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"

    switch (level)
    {
    case MessageLevel::INFO:
        printf("%sInfo: ", KBLU);
        printf("%s%s.\n", KNRM, message);
        break;
    case MessageLevel::WARNING:
        printf("%sWarning: ", KYEL);
        printf("%s%s.\n", KNRM, message);
        break;
    case MessageLevel::ERROR:
        printf("%sError: ", KRED);
        printf("%s%s.\n", KNRM, message);
        break;
    case MessageLevel::FATAL:
        printf("%sFatal: ", KRED);
        printf("%s%s.\n", KNRM, message);
        break;
    }
}

internal void
PrintToConsole(const char* text)
{
    printf(text);
}

internal void
GetInput(char* buffer, size_t bufferSize)
{
    fgets(buffer, bufferSize, stdin);
}

internal bool
ProcessInput(const char* input)
{
    return false;
}

internal Variable*
FindVariable(State& state, const char* name)
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

internal void
AddVariable(State& state, const char* name, Matrix data)
{
    Variable* var = FindVariable(state, name);
    if (var)
    {
        FreeMatrix(var->data);
        var->data = data;
    }
    else
    {
        Variable newVar = {};
        newVar.data = data;
        newVar.name = DuplicateString(name);
        state.vars[state.varsCount++] = newVar;
    }
}

internal void
DeleteVariable(State& state, const char* name)
{
    Variable* var = FindVariable(state, name);
    if (var)
    {
        Variable& lastVar = state.vars[state.varsCount - 1];
        Swap(lastVar.name, var->name);
        Swap(lastVar.data, var->data);
        FreeMatrix(lastVar.data);
        Deallocate(lastVar.name);
        state.varsCount--;
    }
}
// end state.

#define PROMP_TTEXT ">>"

int main(int argc, char const* argv[])
{
    char input[500] = {};
    volatile bool shouldClose = false;
    while (!shouldClose)
    {
        PrintToConsole(PROMP_TTEXT);
        GetInput(input, sizeof(input));
        shouldClose = ProcessInput(input);
    }
    return 0;
}