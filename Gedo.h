/*
 * Author: Ahmed Abdelhay
 * A collection of functions used to make programming easier.
 * contains memory allocation utils, some containers, file IO and some bitmap
 * handling.
 * Users can define GEDO_DYNAMIC_LIBRARY to build as a shared library instead of
 * a static library. List of features:
 * - Utils:
 *  - defer macro: similar to the defer keyword in go this can be used to
 * execute some action at the end of a scope. example: void* data =
 * malloc(size); defer(free(data));  // this code will be executed at the end of
 * the current scope.
 * - General algorithms:
 *      - Min,Max,Clamp
 *      - ArrayCount: get the count of a constant sized c array.
 *      - QuickSort.
 *      - BinarySearch.
 * - Memory utils:
 *      provide Allocator interface that proved Allocate and Free functions,
 *      it also provides some ready implementations allocators:
 *      - Malloc allocator: the default c stdlib allocator.
 *      - Arena allocator:  simple linear allocator that allocates block upfront
 * and keep using it, this is very useful if the user wants in temp allocations
 * where the user knows upfront what is the size that they will be using. it
 * also provides a default allocator where the user can set it and it will be
 * used in all the functions in this library by default. e.g. Allocator* alloc =
 * CreateMyCustomAllocator(...); SetDefaultAllocator(alloc); MemoryBlock block =
 * ReadFile(fileName); // this memory block will be allocated from alloc. it
 * also provides some conversion between units utils:
 *           - BytesToMegaBytes(size_t bytes);
 *           - BytesToGigaBytes(size_t bytes);
 *           - MegaBytesToBytes(size_t megabytes);
 *           - GigaBytesToBytes(size_t gigabytes);
 * - Containers:
 *      - ArrayView<T>      a non owning view of array of type T.
 *      - StaticArray<T,N>  owning stretchy array of type T allocated on the
 * stack with max size N.
 *      - Array<T,N>        owning stretchy array of type T allocated using
 * Allocator*.
 *      - HashTable<T>      TODO.
 * - Maths:
 *      - Math code uses double not float.
 *      - 2D/3D Vector.
 *      - 3x3 Matrix and 4x4 Matrix.
 *      - operator overloading for +,*,- between different types.
 *      - Matrix/vector and Matrix/Matrix multiplication support.
 *      - Common geometrical operation like DotProduct and CrossProduct,
 * Normalise, Transpose, Rotate.
 *      - commonly used functions in computer graphics like Perspective and
 * lookAt.
 *
 *      TODO: Add SIMD support.
 * - UUID:
 *      Provides a cross platform UUID generation function and compare.
 * - File I/O:
 *      Provides a cross platform way of handling files.
 *      - Read whole file           ReadFile(const char* fileName, Allocator& allocator);
 *      - Write whole file          WriteFile(const char* fileName, MemoryBlock block, Allocator& allocator);
 *      - Check if a file exists    DoesFileExist(const char* fileName,Allocator& allocator);
 *      - Get the file size         GetFileSize(const char* fileName, Allocator& allocator);
 *      - Check a path type         GetPathType(const char* path, Allocator& allocator);
 * - Strings:
 *      Provides custom implementation of both String (owning container) and
 * StringView (non owning view). it uses the Allocator* interface for managing
 * memory it also provides some useful functions like:
 *          - StringLength(const char* string);
 *          - GetFileExtension(const char* string);
 *          - CompareStrings(const char* str1, const char* str2);
 *          - CompareStrings(const StringView str1, const StringView str2);
 *          - ConcatStrings(const String* strings, size_t stringsCount, char
 * seperator);
 *          - SplitString(const char* string, char delim, Allocator& allocator);
 *          - SplitStringView(const char* string, char delim, Allocator&
 * allocator);
 *          - SplitStringIntoLines(const char* string, Allocator& allocator);
 *          - SplitStringViewIntoLines(const char* string, char delim,
 * Allocator& allocator);
 * - Bitmaps:
 *      Provide a way of creating bitmap (colored and mono) and blit data to the
 * bitmap, it also provide some util for creating colors, rect, and define some
 * common colors. Blit functions: 
 *          FillRectangle(ColorBitmap& dest, Rect fillArea, const ColorBitmap& src);
 *          FillRectangle(ColorBitmap& dest, Rect fillArea, const Bitmap& mask, Color c);
 *          FillRectangle(ColorBitmap& dest, Rect fillArea, Color color);
 * - Timer:
 *      Provide a way of measuring time between 2 points and then getting this
 * time. it also defines a MACRO  GEDO_TIME_BLOCK(BlockName) that can be used to
 * measure the current block.
 */
#pragma once

#include <stdint.h>

#if defined _WIN32
#define GEDO_OS_WINDOWS 1
#elif defined __linux__
#define GEDO_OS_LINUX 1
#else
#error "Not supported OS"
#endif

#if !defined GEDO_ASSERT
#include <assert.h>
#define GEDO_ASSERT assert
#endif // GEDO_ASSERT

#define GEDO_ASSERT_MSG(msg) GEDO_ASSERT(!msg)

#if !defined GEDO_MALLOC
#include <stdlib.h> // malloc, free
#include <string.h> // memset

#define GEDO_MALLOC malloc
#define GEDO_FREE free
#define GEDO_MEMSET memset
#define GEDO_MEMCPY memcpy
#endif // GEDO_MALLOC

#if defined(GEDO_DYNAMIC_LIBRARY) && defined(GEDO_OS_WINDOWS)
// dynamic library
#if defined(GEDO_IMPLEMENTATION)
#define GEDO_DEF __declspec(dllexport)
#else
#define GEDO_DEF __declspec(dllimport)
#endif
#else
#define GEDO_DEF
#endif

template <typename F> struct privDefer {
  F f;
  privDefer(F f) : f(f) {}
  ~privDefer() { f(); }
};
template <typename F> privDefer<F> defer_func(F f) { return privDefer<F>(f); }
#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })

namespace gedo
{
    //-----------------Algorithms----------------------------//
    template <typename T, size_t N>
    size_t ArrayCount(const T (&)[N])
    {
        return N;
    }

    template<typename T>
    T Max(const T& t0, const T& t1)
    {
        return (t0 > t1) ? t0 : t1;
    }

    template<typename T>
    T Min(const T& t0, const T& t1)
    {
        return (t0 < t1) ? t0 : t1;
    }

    template<typename T>
    T Clamp(const T& t, const T& low, const T& high)
    {
        return (t < low) ? low : (t > high) ? high : t;
    }

    template <typename T>
    void Swap(T& t0, T& t1)
    {
        T t = t0;
        t0 = t1;
        t1 = t;
    }

    template <typename T, typename TPredicate>
    void QuickSort(T* p, size_t size, TPredicate compare)
    {
        /* threshold for transitioning to insertion sort */
        while (size > 12)
        {
            /* compute median of three */
            const size_t m = size >> 1;
            const bool c01 = compare(p[0], p[m]);
            const bool c12 = compare(p[m], p[size - 1]);
            /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
            if (c01 != c12)
            {
                /* otherwise, we'll need to swap something else to middle */
                const bool c = compare(p[0], p[size - 1]);
                /* 0>mid && mid<size:  0>size => size; 0<size => 0    */
                /* 0<mid && mid>size:  0>size => 0;    0<size => size */
                const size_t z = (c == c12) ? 0 : size - 1;
                Swap(p[z], p[m]);
            }
            /* now p[m] is the median-of-three */
            /* swap it to the beginning so it won't move around */
            Swap(p[0], p[m]);

            /* partition loop */
            size_t i = 1;
            size_t j = size - 1;
            for (;;)
            {
                /* handling of equality is crucial here */
                /* for sentinels & efficiency with duplicates */
                for (;; ++i)
                {
                    if (!compare(p[i], p[0])) break;
                }
                for (;; --j)
                {
                    if (!compare(p[0], p[j])) break;
                }
                /* make sure we haven't crossed */
                if (i >= j) break;
                Swap(p[i], p[j]);

                ++i;
                --j;
            }
            /* recurse on smaller side, iterate on larger */
            if (j < (size - i))
            {
                QuickSort(p, j);
                p = p + i;
                size = size - i;
            }
            else
            {
                QuickSort(p + i, size - i);
                size = j;
            }
        }
    }

    template <typename T>
    void QuickSort(T* p, size_t size)
    {
        QuickSort(p, size, [](const T& a, const T& b) { return a < b; });
    }

    template <typename T, typename TCompare, typename TPredicate>
    int64_t BinarySearch(T* p, size_t size, const T& key, TCompare compare, TPredicate predicate)
    {
        size_t low = 0;
        size_t high = size - 1;
        while (low <= high)
        {
            const size_t mid = low + (high - low) / 2;

            // Check if key is present at mid 
            if (predicate(p[mid], key))
            {
                return mid;
            }

            if (compare(p[mid], key))
            {
                low = mid + 1;
            }
            else
            {
                high = mid - 1;
            }
        }

        // if we reach here, then element was not present 
        return -1;
    }

    template <typename T>
    int64_t BinarySearch(T* p, size_t size, const T& key)
    {
        return BinarySearch(p, size, key,
                            [](const T& a, const T& b) { return a < b; },
                            [](const T& a, const T& b) { return a == b; });
    }
    //--------------------------------------------------//

    //------------------Math----------------------------//
    GEDO_DEF const double PI = 3.14159265358979323846264338327950288;

    union Vec2d
    {
        struct
        {
            double x, y;
        };
        double data[2];
    };

    union Vec3d
    {
        struct
        {
            double x, y, z;
        };
        double data[3];
    };

    // the matrices are column major.
    union Mat3
    {
        double elements[3][3];
        double data[9];
    };

    union Mat4
    {
        double elements[4][4];
        double data[16];
    };

    struct Matrix
    {
        static const size_t stackBufferSize = 9;
        double stackBuffer[stackBufferSize] = {};
        size_t rows = 0;
        size_t cols = 0;
        double* data = NULL;
    };

    GEDO_DEF double Deg2Rad(double v);
    GEDO_DEF double Rad2Deg(double v);

    GEDO_DEF Vec2d operator+(const Vec2d a, const Vec2d& b);
    GEDO_DEF Vec2d operator-(const Vec2d a, const Vec2d& b);
    GEDO_DEF Vec2d operator*(const Vec2d a, const Vec2d& b);
    GEDO_DEF Vec2d operator*(const Vec2d a, double x);
    GEDO_DEF Vec2d operator*(double x, const Vec2d a);
    GEDO_DEF Vec3d operator+(const Vec3d a, const Vec3d& b);
    GEDO_DEF Vec3d operator-(const Vec3d a, const Vec3d& b);
    GEDO_DEF Vec3d operator*(const Vec3d a, const Vec3d& b);
    GEDO_DEF Vec3d operator*(const Vec3d a, double x);
    GEDO_DEF Vec3d operator*(double x, const Vec3d a);
    GEDO_DEF Vec3d operator*(const Mat3 a, const Vec3d& v);
    GEDO_DEF Mat4 operator*(const Mat4& left, const Mat4& right);
    GEDO_DEF Mat4 operator*(Mat4 left, double x);
    GEDO_DEF Vec3d CrossProduct(const Vec3d a, const Vec3d& b);
    GEDO_DEF double DotProduct(const Vec3d a, const Vec3d& b);
    GEDO_DEF double DotProduct(const Vec2d a, const Vec2d& b);
    GEDO_DEF double DotProduct(const double* v0, const double* v1, size_t size);
    GEDO_DEF double Length(const Vec3d& v);
    GEDO_DEF double Length(const Vec2d& v);
    GEDO_DEF void Normalise(Vec3d& v);
    GEDO_DEF void Normalise(Vec2d& v);
    GEDO_DEF Vec3d Normalised(const Vec3d& v);
    GEDO_DEF Vec2d Normalised(const Vec2d& v);
    GEDO_DEF Mat3 Transpose(Mat3 m);
    GEDO_DEF Mat4 Transpose(Mat4 m);
    GEDO_DEF Mat4 Identity();
    GEDO_DEF Mat4 Translate(const Mat4& m, Vec3d translation);
    GEDO_DEF Mat4 Rotate(const Mat4& m, double angle, Vec3d v);

    /*
     * @param[in] fovy    Specifies the field of view angle, in degrees, in the y direction.
     * @param[in] aspect  Specifies the aspect ratio that determines the field of view in the x direction. The aspect ratio is the ratio of x (width) to y (height).
     * @param[in] zNear   Specifies the distance from the viewer to the near clipping plane (always positive).
     * @param[in] zFar    Specifies the distance from the viewer to the far clipping plane (always positive).
    */
    GEDO_DEF Mat4 Perspective(double fovy, double aspect, double zNear, double zFar);

    /*!
     * @brief set up view matrix
     *
     * NOTE: The UP vector must not be parallel to the line of sight from
     *       the eye point to the reference point
     *
     * @param[in]  eye    eye vector
     * @param[in]  center center vector
     * @param[in]  up     up vector
     * @param[out] dest   result matrix
     */
    GEDO_DEF Mat4 LookAt(const Vec3d& eye, const Vec3d& center, const Vec3d& up);

    GEDO_DEF double& At(Matrix& m, size_t i, size_t j);
    GEDO_DEF const double& At(const Matrix& m, size_t i, size_t j);
    GEDO_DEF void GetRow(const Matrix& m, size_t row, double* result);
    GEDO_DEF void GetCol(const Matrix& m, size_t col, double* result);
    GEDO_DEF Matrix CreateMatrix(size_t rows, size_t cols);
    GEDO_DEF void FreeMatrix(Matrix& m);
    GEDO_DEF Matrix Zeros(size_t rows, size_t cols);
    GEDO_DEF Matrix Ones(size_t rows, size_t cols);
    GEDO_DEF Matrix Eye(size_t rows, size_t cols);
    GEDO_DEF bool CanMultiply(const Matrix& m0, const Matrix& m1);
    GEDO_DEF bool CanAdd(const Matrix& m0, const Matrix& m1);
    GEDO_DEF bool CanSubtract(const Matrix& m0, const Matrix& m1);
    GEDO_DEF Matrix Multiply(const Matrix& m0, double scalar);
    GEDO_DEF Matrix Add(const Matrix& m0, double scalar);
    GEDO_DEF Matrix Subtract(const Matrix& m0, double scalar);
    GEDO_DEF Matrix Multiply(const Matrix& m0, const Matrix& m1);
    GEDO_DEF Matrix Add(const Matrix& m0, const Matrix& m1);
    GEDO_DEF Matrix Subtract(const Matrix& m0, const Matrix& m1);
    GEDO_DEF Matrix Abs(const Matrix& m);
    GEDO_DEF Matrix Sin(const Matrix& m);
    GEDO_DEF Matrix Cos(const Matrix& m);
    GEDO_DEF Matrix Tan(const Matrix& m);
    GEDO_DEF Matrix ASin(const Matrix& m);
    GEDO_DEF Matrix ACos(const Matrix& m);
    GEDO_DEF Matrix ATan(const Matrix& m);
    //--------------------------------------------------//

    //-----------------UUID-----------------------------//
    struct UUId
    {
        uint8_t data[16] = {};
    };

    GEDO_DEF UUId GenerateUUID();
    GEDO_DEF bool CompareUUID(const UUId& a, const UUId& b);
    //--------------------------------------------------//

    //-----------------Memory --------------------------//
    struct MemoryBlock
    {
        uint8_t* data = NULL;
        size_t size = 0;
    };

    struct Allocator
    {
        virtual void ResetAllocator() = 0;
        virtual MemoryBlock AllocateMemoryBlock(size_t bytes) = 0;
        virtual bool FreeMemoryBlock(MemoryBlock& block) = 0;
    };

    struct LinearAllocator : Allocator
    {
        size_t offset = 0;
        MemoryBlock arena;

        void ResetAllocator() override;
        MemoryBlock AllocateMemoryBlock(size_t bytes) override;
        bool FreeMemoryBlock(MemoryBlock& block) override;
    };

    struct MallocAllocator : Allocator
    {
        void ResetAllocator() override;
        MemoryBlock AllocateMemoryBlock(size_t bytes) override;
        bool FreeMemoryBlock(MemoryBlock& block) override;
    };

    GEDO_DEF Allocator& GetDefaultAllocator();
    GEDO_DEF void SetDefaultAllocator(Allocator& allocator);

    GEDO_DEF MemoryBlock Allocate(size_t bytes, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF bool Deallocate(MemoryBlock& block, Allocator& allocator = GetDefaultAllocator());

    // memory util functions.
    GEDO_DEF bool IsPointerInsideMemoryBlock(const uint8_t* ptr, MemoryBlock block);
    GEDO_DEF bool IsMemoryBlockInside(MemoryBlock big, MemoryBlock small);
    GEDO_DEF void ZeroMemoryBlock(MemoryBlock block);
    GEDO_DEF double BytesToMegaBytes(size_t bytes);
    GEDO_DEF double BytesToGigaBytes(size_t bytes);
    GEDO_DEF size_t MegaBytesToBytes(size_t megabytes);
    GEDO_DEF size_t GigaBytesToBytes(size_t gigabytes);
    // end of memory util functions.

    GEDO_DEF LinearAllocator* CreateLinearAllocator(size_t bytes);
    GEDO_DEF void FreeLinearAllocator(LinearAllocator* allocator);

    GEDO_DEF MallocAllocator* CreateMallocAllocator();
    GEDO_DEF void FreeMallocAllocator(MallocAllocator* allocator);
    //------------------------------------------------------------//

    //--------------------------------File IO---------------------//
    enum class PathType
    {
        FAILURE,
        FILE,
        DIRECTORY
    };

    // File names are utf8 encoded.
    GEDO_DEF MemoryBlock ReadFile(const char* fileName, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF bool WriteFile(const char* fileName, MemoryBlock block, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF bool DoesFileExist(const char* fileName, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF int64_t GetFileSize(const char* fileName, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF PathType GetPathType(const char* path, Allocator& allocator = GetDefaultAllocator());
    //------------------------------------------------------------//

    //------------------------------Containers--------------------//
    template<typename T>
    struct ArrayView
    {
        const T* data = NULL;
        size_t size = 0;

        T* begin()
        {
            return data;
        }
        T* end()
        {
            return data + size;
        }
        const T* begin() const
        {
            return data;
        }
        const T* end() const
        {
            return data + size;
        }
    };

    template<typename T, size_t N>
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
            GEDO_ASSERT(i < size());
            return data()[i];
        }
        const T& operator[](size_t i) const
        {
            GEDO_ASSERT(i < size());
            return data()[i];
        }
        void clear()
        {
            count = 0;
        }
        void push_back(const T& d)
        {
            GEDO_ASSERT(size() < N);
            data()[count++] = d;
        }
        void pop_back()
        {
            count--;
        }
        void resize(size_t s)
        {
            GEDO_ASSERT(s <= N);
            count = s;
        }
    };

    template<typename T>
    struct Array
    {
        Allocator* allocator = &GetDefaultAllocator();
        MemoryBlock block;
        size_t count = 0;

        Array() = default;
        ~Array()
        {
            if (block.data)
            {
                Deallocate(block, *allocator);
            }
        }
        Array(const Array& s)
        {
            allocator = s.allocator;
            block = Allocate(s.block.size, *allocator);
            GEDO_MEMCPY(block.data, s.block.data, block.size);
            count = s.count;
        }
        Array& operator=(const Array& s)
        {
            allocator = s.allocator;
            block = Allocate(s.block.size, *allocator);
            GEDO_MEMCPY(block.data, s.block.data, block.size);
            count = s.count;
            return *this;
        }
        Array(Array&& s) noexcept
        {
            allocator = s.allocator;
            block = s.block;
            count = s.count;
            s.allocator = NULL;
            s.block = MemoryBlock{};
            s.count = 0;
        }
        Array& operator=(Array&& s) noexcept
        {
            allocator = s.allocator;
            block = s.block;
            count = s.count;
            s.allocator = NULL;
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
            GEDO_ASSERT(i < size());
            return data()[i];
        }
        const T& operator[](size_t i) const
        {
            GEDO_ASSERT(i < size());
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
                MemoryBlock newBlock = Allocate(s * sizeof(T), *allocator);
                if (block.data)
                {
                    GEDO_MEMCPY(newBlock.data, block.data, block.size);
                    Deallocate(block, *allocator);
                }
                block = newBlock;
            }
        }
    };

    template<typename TKey, typename TValue>
    struct HashTable
    {
    };

    template <typename T>
    ArrayView<T> CreateArrayView(const T arr[])
    {
        ArrayView<T> result;
        result.data = arr;
        result.size = ArrayCount(arr);
        return result;
    }
    //-------------------------------------------------------------//

    //--------------------------Strings----------------------------//
     struct StringView
    {
        const char* data = NULL;
        size_t size = 0;

        const char* begin()
        {
            return data;
        }
        const char* end()
        {
            return data + size;
        }
        const char* begin() const
        {
            return data;
        }
        const char* end() const
        {
            return data + size;
        }
    };

    using String = Array<char>;

    GEDO_DEF String CreateString(const char* string);
    GEDO_DEF StringView CreateStringView(const char* string);
    GEDO_DEF size_t StringLength(const char* string);
    GEDO_DEF const char* GetFileExtension(const char* string);
    GEDO_DEF bool CompareStrings(const char* str1, const char* str2);
    GEDO_DEF bool CompareStrings(const char* str1, const String& str2);
    GEDO_DEF bool CompareStrings(const String& str1, const char* str2);
    GEDO_DEF bool CompareStrings(const String& str1, const String& str2);
    GEDO_DEF bool CompareStrings(const StringView str1, const StringView str2);
    GEDO_DEF void Append(String& string, const char* s);
    GEDO_DEF void Append(String& string, const char* s, size_t length);
    GEDO_DEF void Append(String& string, const String& s);
    GEDO_DEF void Append(String& string, char c);
    // if separator != 0 it will be added in between the strings.
    // e.g.
    //  String lines [] {"line1", line2};
    //  String data = ConcatStrings(lines, 2, '\n');
    //  GEDO_ASSERT(CompareStrings(data, "line1\nline2"));
    GEDO_DEF String ConcatStrings(const String* strings, size_t stringsCount, char seperator = 0);
    GEDO_DEF Array<String> SplitString(const char* string, char delim, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF Array<StringView> SplitStringView(const char* string, char delim, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF Array<String> SplitStringIntoLines(const char* string, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF Array<StringView> SplitStringViewIntoLines(const char* string, char delim, Allocator& allocator = GetDefaultAllocator());
    //-------------------------------------------------------------//

    //-----------------------------Parsing-------------------------//
    struct Buffer
    {
        const char* data = NULL;
        size_t size = 0;
        size_t cursor = 0;
    };

    GEDO_DEF Buffer CreateBufferFromString(const char* text);
    GEDO_DEF char Peek(const Buffer& buffer);
    GEDO_DEF bool IsLetter(char c);
    GEDO_DEF bool IsDigit(char c);
    GEDO_DEF bool IsWhiteSpace(char c);
    GEDO_DEF bool IsLetterOrDigit(char c);
    GEDO_DEF void SkipToNextLine(Buffer& buffer);
    GEDO_DEF void SkipSingleLineComment(Buffer& buffer);
    GEDO_DEF void SkipWhiteSpaces(Buffer& buffer);
    GEDO_DEF bool ParseFloat(Buffer& buffer, double& result);
    GEDO_DEF bool ParseIdentifier(Buffer& buffer, String& result);
    GEDO_DEF bool ParseStringLiteral(Buffer& buffer, String& result);
    GEDO_DEF bool CompareWordAndSkip(Buffer& buffer, const char* word);

    GEDO_DEF bool StringToFloat(const char* string, double& result);
    GEDO_DEF bool StringToInt(const char* string, int64_t& result);
    //-------------------------------------------------------------//

    //------------------------------Bitmap-------------------------//
    struct Color
    {
        // 0xRRGGBBAA
        uint8_t a = 0;
        uint8_t b = 0;
        uint8_t g = 0;
        uint8_t r = 0;
    };

    struct Rect
    {
        size_t x = 0;
        size_t y = 0;
        size_t width = 0;
        size_t height = 0;
    };

    struct Bitmap
    {
        size_t width = 0;
        size_t height = 0;
        uint8_t* data = NULL;
    };

    struct ColorBitmap
    {
        size_t width = 0;
        size_t height = 0;
        Color* data = NULL;
    };

    GEDO_DEF void FillRectangle(ColorBitmap& dest, Rect fillArea, const ColorBitmap& src);
    GEDO_DEF void FillRectangle(ColorBitmap& dest, Rect fillArea, const Bitmap& mask, Color c);
    GEDO_DEF void FillRectangle(ColorBitmap& dest, Rect fillArea, Color color);

    GEDO_DEF Bitmap CreateBitmap(size_t width, size_t height, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF void FreeBitmap(Bitmap& bitmap, Allocator& allocator = GetDefaultAllocator());

    GEDO_DEF ColorBitmap CreateColorBitmap(size_t width, size_t height, Allocator& allocator = GetDefaultAllocator());
    GEDO_DEF void FreeColorBitmap(ColorBitmap& bitmap, Allocator& allocator = GetDefaultAllocator());

    GEDO_DEF Color CreateColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

    GEDO_DEF const Color RED = CreateColor(255, 0, 0, 255);
    GEDO_DEF const Color GREEN = CreateColor(0, 255, 0, 255);
    GEDO_DEF const Color GREEN_BLUE = CreateColor(78, 201, 176, 255);
    GEDO_DEF const Color YELLOW = CreateColor(255, 255, 0, 255);
    GEDO_DEF const Color BLUE = CreateColor(0, 0, 255, 255);
    GEDO_DEF const Color WHITE = CreateColor(255, 255, 255, 255);
    GEDO_DEF const Color BLACK = CreateColor(0, 0, 0, 255);
    GEDO_DEF const Color DARK_GREY = CreateColor(30, 30, 30, 255);
    //------------------------------------------------------------//

    //---------------------IO-------------------------------------//
    enum class ConsoleColor
    {
        WHITE,
        RED,
        GREEN,
        BLUE
    };
    // utf8 text.
    GEDO_DEF void PrintToConsole(const char* text, ConsoleColor color = ConsoleColor::WHITE);
    GEDO_DEF void PrintToConsole(char c, ConsoleColor color = ConsoleColor::WHITE);
    GEDO_DEF void ReadFromConsole(char* buffer, size_t bufferSize);
    GEDO_DEF void ClearConsole();
    //------------------------------------------------------------//

    //-----------------------Time  -------------------------------//
#define GEDO_TIME_BLOCK(BlockName)                                             \
  StopWatch ___timer;                                                          \
  StartStopWatch(___timer);                                                    \
  defer({                                                                      \
    StopStopWatch(___timer);                                                   \
    char buffer[200] = {};                                                     \
    sprintf(buffer, "Time spent in (%s): %f seconds.\n", BlockName,            \
            ElapsedSeconds(___timer));                                         \
    PrintToConsole(buffer);                                                    \
  });

    struct StopWatch
    {
        int64_t start = 0;
        int64_t end = 0;
    };
    GEDO_DEF void StartStopWatch(StopWatch& in);
    GEDO_DEF void StopStopWatch(StopWatch& in);
    GEDO_DEF double ElapsedSeconds(const StopWatch &in);
    //------------------------------------------------------------//
}
