#include "AhmedLab.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool ReadFile(const char* fileName, MemoryBlock& result)
{
    FILE* fp = fopen(fileName, "r");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        const int64_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        result = Allocate(size + 1);
        fread(result.data, 1, size, fp);
        result.data[size] = 0;
        fclose(fp);
        return true;
    }
    return false;
}

bool WriteFile(const char* fileName, const MemoryBlock block)
{
    FILE* fp = fopen(fileName, "w");
    if (fp)
    {
        fwrite(block.data, 1, block.size, fp);
        fclose(fp);
        return true;
    }
    return false;
}

bool PrintToConsole(const char* text, Color c)
{
    return printf(text);
}

void ReadFromConsole(char* buffer, size_t bufferSize)
{
    fgets(buffer, bufferSize, stdin);
}

void Deallocate(MemoryBlock& block)
{
    free(block.data);
    block.data = NULL;
    block.size = 0;
}

MemoryBlock Allocate(size_t size)
{
    MemoryBlock result;
    result.data = (uint8_t*)calloc(1, size);
    if (result.data)
    {
        result.size = size;
    }
    return result;
}

void ZeroMemory(MemoryBlock block)
{
    memset(block.data, 0, block.size);
}

size_t StringLength(const char* text)
{
    size_t result = 0;
    while (*text++)
    {
        result++;
    }
    return result;
}

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

bool CompareStrings(const char* s0, const char* s1)
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

Buffer CreateBufferFromString(const char* string)
{
    Buffer result;
    result.text = string;
    result.size = StringLength(string);
    result.cursor = 0;
    return result;
}

double& At(Matrix& m, size_t i, size_t j)
{
    return m.data[m.rows * i + j];
}

const double& At(const Matrix& m, size_t i, size_t j)
{
    return m.data[m.rows * i + j];
}

void GetRow(const Matrix& m, size_t row, double* result)
{
    for (size_t i = 0; i < m.cols; ++i)
    {
        result[i] = At(m, row, i);
    }
}

double DotProduct(const double* v0, const double* v1, size_t size)
{
    double result = 0;
    for (size_t i = 0; i < size; i++)
    {
        result += v0[i] * v1[i];
    }
    return result;
}

void GetCol(const Matrix& m, size_t col, double* result)
{
    for (size_t i = 0; i < m.rows; ++i)
    {
        result[i] = At(m, i, col);
    }
}

Matrix CreateMatrix(size_t rows, size_t cols)
{
    Matrix result;
    result.rows = rows;
    result.cols = cols;
    if (rows * cols <= Matrix::stackBufferSize)
    {
        result.data = result.stackBuffer;
    }
    else
    {
        result.data = (double*)Allocate(rows * cols * sizeof(double)).data;
    }
    return result;
}

Matrix Zeros(size_t rows, size_t cols)
{
    Matrix result = CreateMatrix(rows, cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = 0;
    }
    return result;
}

Matrix Ones(size_t rows, size_t cols)
{
    Matrix result = CreateMatrix(rows, cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = 1;
    }
    return result;
}

Matrix Eye(size_t rows, size_t cols)
{
    Matrix result = CreateMatrix(rows, cols);
    for (size_t i = 0; i < result.rows; ++i)
    {
        for (size_t j = 0; j < result.cols; ++j)
        {
            At(result, i, j) = (i == j) ? 1 : 0;
        }
    }
    return result;
}

void FreeMatrix(Matrix& m)
{
    if (m.data && m.data != m.stackBuffer)
    {
        MemoryBlock block;
        block.data = (uint8_t*)m.data;
        block.size = m.rows * m.cols * sizeof(double);
        Deallocate(block);
        m.data = NULL;
    }
}

bool CanMultiply(const Matrix& m0, const Matrix& m1)
{
    return ((m0.rows == m0.cols == 1) || (m1.rows == m1.cols == 1) ||
            (m0.cols == m1.rows));
}

bool CanAdd(const Matrix& m0, const Matrix& m1)
{
    return ((m0.rows == m0.cols == 1) || (m1.rows == m1.cols == 1) ||
            (m0.rows == m1.rows && m0.cols == m1.cols));
}

bool CanSubtract(const Matrix& m0, const Matrix& m1)
{
    return CanAdd(m0, m1);
}

Matrix Multiply(const Matrix& m0, double scalar)
{
    Matrix result = CreateMatrix(m0.rows, m0.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = m0.data[i] * scalar;
    }
    return result;
}

Matrix Add(const Matrix& m0, double scalar)
{
    Matrix result = CreateMatrix(m0.rows, m0.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = m0.data[i] + scalar;
    }
    return result;
}

Matrix Subtract(const Matrix& m0, double scalar)
{
    Matrix result = CreateMatrix(m0.rows, m0.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = m0.data[i] - scalar;
    }
    return result;
}

Matrix Multiply(const Matrix& m0, const Matrix& m1)
{
    assert(CanMultiply(m0, m1));
    if (m0.rows == m0.cols == 1)
    {
        return Multiply(m1, m0.data[0]);
    }
    else if (m1.rows == m1.cols == 1)
    {
        return Multiply(m0, m1.data[0]);
    }
    else
    {
        Matrix result = CreateMatrix(m0.rows, m1.cols);
        MemoryBlock rowBlock = Allocate(m0.cols * sizeof(double));
        MemoryBlock colBlock = Allocate(m0.cols * sizeof(double));
        defer(Deallocate(colBlock));
        defer(Deallocate(rowBlock));

        double* col = (double*)colBlock.data;
        double* row = (double*)rowBlock.data;
        for (size_t i = 0; i < result.rows; ++i)
        {
            for (size_t j = 0; j < result.cols; ++j)
            {
                GetRow(m0, i, row);
                GetCol(m1, j, col);
                At(result, i, j) = DotProduct(row, col, m0.cols);
            }
        }
        return result;
    }
}

Matrix Add(const Matrix& m0, const Matrix& m1)
{
    assert(CanAdd(m0, m1));
    if (m0.rows == m0.cols == 1)
    {
        return Add(m1, m0.data[0]);
    }
    else if (m1.rows == m1.cols == 1)
    {
        return Add(m0, m1.data[0]);
    }
    else
    {
        Matrix result = CreateMatrix(m0.rows, m0.cols);
        for (size_t i = 0; i < result.rows * result.cols; ++i)
        {
            result.data[i] = m0.data[i] + m1.data[i];
        }
        return result;
    }
}

Matrix Subtract(const Matrix& m0, const Matrix& m1)
{
    assert(CanSubtract(m0, m1));
    if (m0.rows == m0.cols == 1)
    {
        return Subtract(m1, m0.data[0]);
    }
    else if (m1.rows == m1.cols == 1)
    {
        return Subtract(m0, m1.data[0]);
    }
    else
    {
        Matrix result = CreateMatrix(m0.rows, m0.cols);
        for (size_t i = 0; i < result.rows * result.cols; ++i)
        {
            result.data[i] = m0.data[i] - m1.data[i];
        }
        return result;
    }
}

Matrix Abs(const Matrix& m)
{
    Matrix result = CreateMatrix(m.rows, m.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = fabs(m.data[i]);
    }
    return result;
}

Matrix Sin(const Matrix& m)
{
    Matrix result = CreateMatrix(m.rows, m.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = sin(m.data[i]);
    }
    return result;
}

Matrix Cos(const Matrix& m)
{
    Matrix result = CreateMatrix(m.rows, m.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = cos(m.data[i]);
    }
    return result;
}

Matrix Tan(const Matrix& m)
{
    Matrix result = CreateMatrix(m.rows, m.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = tan(m.data[i]);
    }
    return result;
}

Matrix ASin(const Matrix& m)
{
    Matrix result = CreateMatrix(m.rows, m.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = asin(m.data[i]);
    }
    return result;
}

Matrix ACos(const Matrix& m)
{
    Matrix result = CreateMatrix(m.rows, m.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = acos(m.data[i]);
    }
    return result;
}

Matrix ATan(const Matrix& m)
{
    Matrix result = CreateMatrix(m.rows, m.cols);
    for (size_t i = 0; i < result.rows * result.cols; ++i)
    {
        result.data[i] = atan(m.data[i]);
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
    ZeroMemory(block);

    sprintf(buffer, "Size = (%d X %d).\n", (int)var.value.rows,
            (int)var.value.cols);
    PrintToConsole(buffer);
    ZeroMemory(block);

    sprintf(buffer, "Data = [");
    PrintToConsole(buffer);
    ZeroMemory(block);

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
            ZeroMemory(block);
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
