#include "Gedo.h"

#include <math.h>

#if defined GEDO_OS_WINDOWS
#define UNICODE
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define WIN32_MEAN_AND_LEAN
#define VC_EXTRALEAN
#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#include <windows.h>
#include <Rpc.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#undef WIN32_MEAN_AND_LEAN
#undef VC_EXTRALEAN
#elif defined GEDO_OS_LINUX
#include <time.h>
#include <stdio.h>
#include <uuid/uuid.h> // user will have to link against libuuid.
#include <sys/stat.h>
#else
#error "Not supported OS"
#endif

namespace gedo
{
    //----------------------------Memory-------------------------//
    static Allocator* defaultAllocator = CreateMallocAllocator();

    Allocator& GetDefaultAllocator()
    {
        return *defaultAllocator;
    }

    void SetDefaultAllocator(Allocator& allocator)
    {
        defaultAllocator = &allocator;
    }

    MemoryBlock Allocate(size_t bytes, Allocator& allocator)
    {
        return allocator.AllocateMemoryBlock(bytes);
    }

    bool Deallocate(MemoryBlock& block, Allocator& allocator)
    {
        return allocator.FreeMemoryBlock(block);
    }

    bool IsPointerInsideMemoryBlock(const uint8_t* ptr, MemoryBlock block)
    {
        const uint8_t* begin = block.data;
        const uint8_t* end = block.data + block.size;
        return (ptr >= begin && ptr < end);
    }

    bool IsMemoryBlockInside(MemoryBlock big, MemoryBlock small)
    {
        return IsPointerInsideMemoryBlock(small.data, big) &&
            IsPointerInsideMemoryBlock(small.data + small.size, big);
    }

    void ZeroMemoryBlock(MemoryBlock block)
    {
        GEDO_MEMSET(block.data, 0, block.size);
    }

    double BytesToMegaBytes(size_t bytes)
    {
        return bytes / double(1024ULL * 1024ULL);
    }

    double BytesToGigaBytes(size_t bytes)
    {
        return bytes / double(1024ULL * 1024ULL * 1024ULL);
    }

    size_t MegaBytesToBytes(size_t megabytes)
    {
        return (megabytes * 1024ULL * 1024ULL);
    }

    size_t GigaBytesToBytes(size_t gigabytes)
    {
        return (gigabytes * 1024ULL * 1024ULL * 1024ULL);
    }

    LinearAllocator* CreateLinearAllocator(size_t bytes)
    {
        LinearAllocator* allocator = new LinearAllocator();
        allocator->arena.data = (uint8_t*)GEDO_MALLOC(bytes);
        allocator->arena.size = bytes;
        ZeroMemoryBlock(allocator->arena);
        return allocator;
    }

    void FreeLinearAllocator(LinearAllocator* allocator)
    {
        GEDO_ASSERT(allocator);
        GEDO_ASSERT(allocator->arena.data);
        allocator->offset = 0;
        GEDO_FREE(allocator->arena.data);
        allocator->arena = MemoryBlock{};
        delete allocator;
    }

    void LinearAllocator::ResetAllocator()
    {
        offset = 0;
    }

    MemoryBlock LinearAllocator::AllocateMemoryBlock(size_t bytes)
    {
        MemoryBlock result;
        if ((offset + bytes) <= arena.size)
        {
            result.size = bytes;
            result.data = offset + arena.data;
            offset += bytes;
            ZeroMemoryBlock(result);
            return result;
        }
        GEDO_ASSERT_MSG("don't have enough space.");
        return result;
    }

    bool LinearAllocator::FreeMemoryBlock(MemoryBlock& block)
    {
        // no-op for the allocator.
        if (IsMemoryBlockInside(arena, block))
        {
            block.size = 0;
            block.data = NULL;
            return true;
        }
        return false;
    }

    MallocAllocator* CreateMallocAllocator()
    {
        MallocAllocator* allocator = new MallocAllocator();
        return allocator;
    }

    void FreeMallocAllocator(MallocAllocator* allocator)
    {
        GEDO_ASSERT(allocator);
        delete allocator;
    }

    void MallocAllocator::ResetAllocator()
    {
    }

    MemoryBlock MallocAllocator::AllocateMemoryBlock(size_t bytes)
    {
        MemoryBlock result;
        result.data = (uint8_t*)GEDO_MALLOC(bytes);
        if (!result.data)
        {
            GEDO_ASSERT_MSG("don't have enough space.");
        }
        result.size = bytes;
        ZeroMemoryBlock(result);
        return result;
    }

    bool MallocAllocator::FreeMemoryBlock(MemoryBlock& block)
    {
        GEDO_ASSERT(block.data);
        GEDO_FREE(block.data);
        block.size = 0;
        block.data = NULL;
        return true;
    }
    //-----------------------------------------------------------//

    //-------------------------Bitmap manipulation---------------//
    void FillRectangle(ColorBitmap& dest, Rect fillArea, const ColorBitmap& src)
    {
        uint32_t srcIdx = 0;
        for (uint32_t y = fillArea.y; y < (fillArea.y + fillArea.height); ++y)
        {
            for (uint32_t x = fillArea.x; x < (fillArea.x + fillArea.width); ++x)
            {
                const uint32_t index = y * dest.width + x;
                dest.data[index] = src.data[srcIdx++];
            }
        }
    }

    void FillRectangle(ColorBitmap& dest, Rect fillArea, const Bitmap& mask, Color c)
    {
        uint32_t srcIdx = 0;
        for (uint32_t y = fillArea.y; y < (fillArea.y + fillArea.height); ++y)
        {
            for (uint32_t x = fillArea.x; x < (fillArea.x + fillArea.width); ++x)
            {
                const uint32_t index = y * dest.width + x;
                if (mask.data[srcIdx++])
                {
                    dest.data[index] = c;
                }
            }
        }
    }

    void FillRectangle(ColorBitmap& dest, Rect fillArea, Color color)
    {
        for (uint32_t y = fillArea.y; y < (fillArea.y + fillArea.height); ++y)
        {
            for (uint32_t x = fillArea.x; x < (fillArea.x + fillArea.width); ++x)
            {
                const uint32_t index = y * dest.width + x;
                dest.data[index] = color;
            }
        }
    }

    Color CreateColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    {
        Color c;
        c.r = red;
        c.g = green;
        c.b = blue;
        c.a = alpha;
        return c;
    }

    ColorBitmap CreateColorBitmap(size_t width, size_t height, Allocator& allocator)
    {
        ColorBitmap result;
        result.width = width;
        result.height = height;
        MemoryBlock block = Allocate(sizeof(Color) * width * height, allocator);
        GEDO_ASSERT(block.data);
        result.data = (Color*)block.data;
        return result;
    }

    void FreeColorBitmap(ColorBitmap& bitmap, Allocator& allocator)
    {
        GEDO_ASSERT(bitmap.data);
        MemoryBlock block;
        block.data = (uint8_t*)bitmap.data;
        block.size = bitmap.width * bitmap.height * sizeof(Color);
        Deallocate(block, allocator);
        bitmap.data = NULL;
    }

    Bitmap CreateBitmap(size_t width, size_t height, Allocator& allocator)
    {
        Bitmap result;
        result.width = width;
        result.height = height;
        MemoryBlock block = Allocate(sizeof(uint8_t) * width * height, allocator);
        GEDO_ASSERT(block.data);
        result.data = (uint8_t*)block.data;
        return result;
    }

    void FreeBitmap(Bitmap& bitmap, Allocator& allocator)
    {
        GEDO_ASSERT(bitmap.data);
        MemoryBlock block;
        block.data = (uint8_t*)bitmap.data;
        block.size = bitmap.width * bitmap.height * sizeof(uint8_t);
        Deallocate(block, allocator);
        bitmap.data = NULL;
    }
    //-----------------------------------------------------------//

    //--------------------------------File IO---------------------//
#if defined GEDO_OS_WINDOWS
    struct Utf16String
    {
        Allocator* allocator = &GetDefaultAllocator();
        MemoryBlock memory;
        wchar_t* text = NULL;
        size_t size = 0;
    };

    static void FreeUtf16String(Utf16String& w)
    {
        if (w.memory.data)
        {
            Deallocate(w.memory, *w.allocator);
        }
    }

    static Utf16String UTF8ToUTF16(const char* fileName, Allocator& allocator)
    {
        const size_t len = strlen(fileName);
        const size_t wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, fileName, len, NULL, 0);
        Utf16String result;
        result.allocator = &allocator;
        result.memory = Allocate(sizeof(wchar_t) * (wlen + 1), allocator);
        result.text = (wchar_t*)result.memory.data;
        result.size = wlen;
        ZeroMemoryBlock(result.memory);
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, fileName, len, result.text, result.size);
        return result;
    }

    static HANDLE GetFileHandle(const char* fileName, bool read, Allocator& allocator)
    {
        if (!fileName)
        {
            return NULL;
        }
        Utf16String string = UTF8ToUTF16(fileName, allocator);
        defer(FreeUtf16String(string));
        HANDLE handle = NULL;
        if (read)
        {
            handle = CreateFile(string.text,     // file to open
                                GENERIC_READ,    // open for reading
                                FILE_SHARE_READ, // share for reading
                                NULL,            // default security
                                OPEN_EXISTING,   // existing file only
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                                NULL);
        }
        else
        {
            handle = CreateFile(string.text,           // name of the write
                                GENERIC_WRITE,         // open for writing
                                0,                     // do not share
                                NULL,                  // default security
                                CREATE_NEW,            // create new file only
                                FILE_ATTRIBUTE_NORMAL, // normal file
                                NULL);
        }
        return handle;
    }

    MemoryBlock ReadFile(const char* fileName, Allocator& allocator)
    {
        MemoryBlock result;
        const int64_t fileSize = GetFileSize(fileName);
        if (fileSize < 0)
        {
            return result;
        }
        result = Allocate(fileSize + 1, allocator);
        HANDLE handle = GetFileHandle(fileName, true, allocator);
        defer(CloseHandle(handle));
        OVERLAPPED overlapped = {};
        const bool success = ReadFileEx(handle,
                                        result.data,
                                        result.size - 1,
                                        &overlapped,
                                        NULL);
        GEDO_ASSERT(success);
        return result;
    }

    bool WriteFile(const char* fileName, MemoryBlock block, Allocator& allocator)
    {
        HANDLE handle = GetFileHandle(fileName, false, allocator);
        if (handle)
        {
            defer(CloseHandle(handle));
            OVERLAPPED overlapped = {};
            return WriteFileEx(handle, block.data, block.size - 1, &overlapped, NULL);
        }
        return false;
    }

    bool DoesFileExist(const char* fileName, Allocator& allocator)
    {
        HANDLE handle = GetFileHandle(fileName, true, allocator);
        const bool found = handle != INVALID_HANDLE_VALUE;
        if (found)
        {
            CloseHandle(handle);
        }
        return found;
    }

    int64_t GetFileSize(const char* fileName, Allocator& allocator)
    {
        HANDLE handle = GetFileHandle(fileName, true, allocator);
        const bool found = handle != INVALID_HANDLE_VALUE;
        if (found)
        {
            LARGE_INTEGER size;
            GetFileSizeEx(handle, &size);
            CloseHandle(handle);
            return size.QuadPart;
        }
        return -1;
    }

    PathType GetPathType(const char* path, Allocator& allocator)
    {
        HANDLE handle = GetFileHandle(path, true, allocator);
        const bool found = handle != INVALID_HANDLE_VALUE;
        if (found)
        {
            defer(CloseHandle(handle));
            FILE_BASIC_INFO basicInfo;
            GetFileInformationByHandleEx(handle, FileBasicInfo, &basicInfo, sizeof(basicInfo));
            return (basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ? PathType::DIRECTORY
                : PathType::FILE;
        }
        return PathType::FAILURE;
    }
#elif defined GEDO_OS_LINUX
    MemoryBlock ReadFile(const char* fileName, Allocator& allocator)
    {
        FILE* fp = fopen(fileName, "r");
        MemoryBlock result;
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            const int64_t size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            result = Allocate(size + 1, allocator);
            fread(result.data, 1, size, fp);
            result.data[size] = 0;
            fclose(fp);
        }
        return result;
    }

    bool WriteFile(const char* fileName, MemoryBlock block, Allocator& allocator)
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

    bool DoesFileExist(const char* fileName, Allocator& allocator)
    {
        FILE* fp = fopen(fileName, "r");
        if (fp)
        {
            fclose(fp);
            return true;
        }
        return false;
    }

    int64_t GetFileSize(const char* fileName, Allocator& allocator)
    {
        FILE* fp = fopen(fileName, "r");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            fclose(fp);
            const int64_t size = ftell(fp);
            return size;
        }
        return -1;
    }

    PathType GetPathType(const char* path, Allocator& allocator)
    {
        struct stat s;
        if (stat(path, &s) == 0)
        {
            if (s.st_mode & S_IFDIR)
            {
                return PathType::DIRECTORY;
            }
            else if (s.st_mode & S_IFREG)
            {
                return PathType::FILE;
            }
        }
        return PathType::FAILURE;
    }
#endif
    //------------------------------------------------------------//

    //------------------Strings----------------------------------//
    static String CopyString(const char* string, size_t from, size_t to, Allocator& allocator)
    {
        String s;
        s.allocator = &allocator;
        s.reserve(to - from + 1);
        for (size_t i = from; i < to; ++i)
        {
            Append(s, string[i]);
        }
        return s;
    }

    size_t StringLength(const char* string)
    {
        size_t result = 0;
        while (*string++)
        {
            result++;
        }
        return result;
    }

    const char* GetFileExtension(const char* string)
    {
        const size_t len = StringLength(string);
        if (!len)
        {
            return NULL;
        }

        for (int64_t i = len - 1; i >= 0; --i)
        {
            if (string[i] == '.')
            {
                return string + i;
            }
        }
        return NULL;
    }

    bool CompareStrings(const char* str1, const char* str2, size_t size0, size_t size1)
    {
        if (size0 != size1)
        {
            return false;
        }
        for (size_t i = 0; i < size0; ++i)
        {
            if (str1[i] != str2[i])
            {
                return false;
            }
        }
        return true;
    }

    bool CompareStrings(const char* str1, const String& str2)
    {
        return CompareStrings(str2, str1);
    }

    bool CompareStrings(const String& str1, const char* str2)
    {
        return CompareStrings(str1.data(), str2, str1.size(), StringLength(str2));
    }

    bool CompareStrings(const String& str1, const String& str2)
    {
        return CompareStrings(str1.data(), str2.data(), str1.size(), str2.size());
    }

    bool CompareStrings(const StringView str1, const StringView str2)
    {
        return CompareStrings(str1.data, str2.data, str1.size, str2.size);
    }

    bool CompareStrings(const char* str1, const char* str2)
    {
        const size_t len1 = StringLength(str1);
        const size_t len2 = StringLength(str2);
        return CompareStrings(str1, str2, len1, len2);
    }

    String ConcatStrings(const String* strings, size_t stringsCount, char seperator)
    {
        const bool addSeperator = seperator != 0;
        size_t resultSize = 0;
        for (size_t i = 0; i < stringsCount; ++i)
        {
            resultSize += strings[i].size();
        }
        if (addSeperator)
        {
            resultSize += (stringsCount - 1);
        }

        String result;
        result.reserve(resultSize);
        for (size_t i = 0; i < stringsCount; ++i)
        {
            Append(result, strings[i]);
            if (addSeperator && i != stringsCount - 1)
            {
                Append(result, seperator);
            }
        }
        return result;
    }

    String CreateString(const char* string)
    {
        String result;
        Append(result, string);
        return result;
    }

    void Append(String& string, const char* s)
    {
        Append(string, s, StringLength(s));
    }

    void Append(String& string, const char* s, size_t length)
    {
        string.reserve(string.size() + length);
        for (size_t i = 0; i < length; ++i)
        {
            Append(string, s[i]);
        }
    }

    void Append(String& string, const String& s)
    {
        Append(string, s.data(), s.size());
    }

    void Append(String& string, char c)
    {
        string.push_back(c);
    }

    StringView CreateStringView(const char* string)
    {
        StringView result;
        result.data = string;
        result.size = StringLength(string);
        return result;
    }

    Array<StringView> SplitStringView(const char* string, char delim, Allocator& allocator)
    {
        Array<StringView> result;
        result.allocator = &allocator;
        const size_t len = StringLength(string);
        size_t previousOffset = 0;
        size_t i = 0;

        size_t partsCount = 0;
        while (i < len)
        {
            if (string[i] == delim)
            {
                if (i - previousOffset)
                {
                    partsCount++;
                }
                while (string[i] == delim && i < len)
                {
                    i++;
                }
                previousOffset = i;
            }
            else
            {
                i++;
            }
        }
        if (i - previousOffset)
        {
            partsCount++;
        }
        result.reserve(partsCount);

        previousOffset = 0;
        i = 0;
        while (i < len)
        {
            if (string[i] == delim)
            {
                const size_t slen = i - previousOffset;
                if (slen)
                {
                    StringView l;
                    l.data = string + previousOffset;
                    l.size = slen;
                    result.push_back(l);
                }
                while (string[i] == delim && i < len)
                {
                    i++;
                }
                previousOffset = i;
            }
            else
            {
                i++;
            }
        }
        const size_t slen = i - previousOffset;
        if (slen)
        {
            StringView l;
            l.data = string + previousOffset;
            l.size = slen;
            result.push_back(l);
        }
        return result;
    }

    Array<String> SplitString(const char* string, char delim, Allocator& allocator)
    {
        Array<String> result;
        result.allocator = &allocator;
        const size_t len = StringLength(string);
        size_t previousOffset = 0;
        size_t i = 0;
        size_t partsCount = 0;
        while (i < len)
        {
            if (string[i] == delim)
            {
                if (i - previousOffset)
                {
                    partsCount++;
                }
                while (string[i] == delim && i < len)
                {
                    i++;
                }
                previousOffset = i;
            }
            else
            {
                i++;
            }
        }
        if (i - previousOffset)
        {
            partsCount++;
        }
        result.reserve(partsCount);

        i = 0;
        previousOffset = 0;
        while (i < len)
        {
            if (string[i] == delim)
            {
                const size_t slen = i - previousOffset;
                if (slen)
                {
                    String l = CopyString(string, previousOffset, i, allocator);
                    result.push_back(l);
                }
                while (string[i] == delim && i < len)
                {
                    i++;
                }
                previousOffset = i;
            }
            else
            {
                i++;
            }
        }

        const size_t slen = i - previousOffset;
        if (slen)
        {
            String l = CopyString(string, previousOffset, i, allocator);
            result.push_back(l);
        }
        return result;
    }

    Array<String> SplitStringIntoLines(const char* string, Allocator& allocator)
    {
        Array<String> result;
        result.allocator = &allocator;
        const size_t len = StringLength(string);
        size_t partsCount = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (string[i] == '\n')
            {
                partsCount++;
            }
        }
        result.reserve(partsCount);

        size_t previousOffset = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (string[i] == '\n')
            {
                if (i - previousOffset)
                {
                    String l = CopyString(string, previousOffset, i, allocator);
                    result.push_back(l);
                }
                previousOffset = i;
            }
        }
        if (len - previousOffset)
        {
            String l = CopyString(string, previousOffset, len, allocator);
            result.push_back(l);
        }
        return result;
    }

    Array<StringView> SplitStringViewIntoLines(const char* string, char delim, Allocator& allocator)
    {
        Array<StringView> result;
        result.allocator = &allocator;
        const size_t len = StringLength(string);
        size_t partsCount = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (string[i] == '\n')
            {
                partsCount++;
            }
        }
        result.reserve(partsCount);

        size_t previousOffset = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (string[i] == '\n')
            {
                if (i - previousOffset)
                {
                    StringView l;
                    l.data = string + previousOffset;
                    l.size = i - previousOffset;
                    result.push_back(l);
                }
                previousOffset = i;
            }
        }
        if (len - previousOffset)
        {
            StringView l;
            l.data = string + previousOffset;
            l.size = len - previousOffset;
            result.push_back(l);
        }
        return result;
    }
    //------------------------------------------------------------//

    //------------------------Parsing-----------------------------//

    char Peek(const Buffer& buffer)
    {
        GEDO_ASSERT(buffer.cursor < buffer.size);
        return buffer.data[buffer.cursor];
    }

    bool IsLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool IsDigit(char c)
    {
        return (c >= '0' && c <= '9');
    }

    bool IsWhiteSpace(char c)
    {
        switch (c)
        {
        case ' ':
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
            return true;
        }
        return false;
    }

    bool IsLetterOrDigit(char c)
    {
        return IsLetter(c) || IsDigit(c);
    }

    Buffer CreateBufferFromString(const char* text)
    {
        Buffer result;
        result.data = text;
        result.size = StringLength(text);
        result.cursor = 0;
        return result;
    }

    void SkipToNextLine(Buffer& buffer)
    {
        while (buffer.cursor < buffer.size && Peek(buffer) != '\n')
        {
            buffer.cursor++;
        }
    }

    void SkipWhiteSpaces(Buffer& buffer)
    {
        while (buffer.cursor < buffer.size && IsWhiteSpace(Peek(buffer)))
        {
            buffer.cursor++;
        }
    }

    void SkipSingleLineComment(Buffer& buffer)
    {
        if (buffer.data[buffer.cursor + 0] == '\\' &&
            buffer.data[buffer.cursor + 1] == '\\')
        {
            buffer.cursor += 2;
            SkipToNextLine(buffer);
        }
    }

    bool ParseFloat(Buffer& buffer, double& result)
    {
        if (IsDigit(Peek(buffer)))
        {
            String data;
            bool dotAdded = false;
            while (buffer.cursor < buffer.size &&
                   (IsDigit(Peek(buffer)) || Peek(buffer) == '.'))
            {
                if (Peek(buffer) == '.')
                {
                    if (dotAdded)
                    {
                        return false;
                    }
                    dotAdded = true;
                }
                Append(data, Peek(buffer));
                buffer.cursor++;
            }
            Append(data, (char)0); // since String is not null terminated.
            return StringToFloat(data.data(), result);
        }
        return false;
    }

    bool ParseIdentifier(Buffer& buffer, String& result)
    {
        if (IsLetter(Peek(buffer)))
        {
            result.clear();
            while (buffer.cursor < buffer.size &&
                   (IsLetterOrDigit(Peek(buffer)) || Peek(buffer) == '_'))
            {
                Append(result, Peek(buffer));
                buffer.cursor++;
            }
            return true;
        }
        return false;
    }

    bool ParseStringLiteral(Buffer& buffer, String& result)
    {
        if (Peek(buffer) == '"')
        {
            result.clear();
            buffer.cursor++;
            while (buffer.cursor < buffer.size && Peek(buffer) != '"')
            {
                Append(result, Peek(buffer));
                buffer.cursor++;
            }
            buffer.cursor++;
            return true;
        }
        return false;
    }

    bool CompareWordAndSkip(Buffer& buffer, const char* word)
    {
        const size_t length = StringLength(word);
        if (buffer.cursor + length < buffer.size)
        {
            for (size_t i = 0; i < length; ++i)
            {
                if (buffer.data[buffer.cursor + i] != word[i])
                {
                    return false;
                }
            }
            buffer.cursor += length;
            return true;
        }
        return false;
    }

    bool StringToFloat(const char* string, double& result)
    {
        const size_t length = StringLength(string);
        if (!length)
        {
            return false;
        }

        size_t dot = length;
        for (size_t i = 0; i < length; ++i)
        {
            if (string[i] == '.')
            {
                if (dot != length)
                {
                    return false;
                }
                dot = i;
            }
        }

        result = 0.0;
        size_t power = dot - 1;
        for (size_t i = 0; i < length; ++i)
        {
            if (string[i] != '.')
            {
                if (string[i] != '0')
                {
                    double basePower = 1.0;
                    if (power >= 0)
                    {
                        for (int k = 0; k < power; k++)
                        {
                            basePower *= 10.0;
                        }
                    }
                    else if (power < 0)
                    {
                        for (int k = 0; k < -power; k++)
                        {
                            basePower *= 0.1;
                        }
                    }
                    result += (string[i] - '0') * basePower;
                }
                power--;
            }
            else
            {
                power = -1;
            }
        }
        return true;
    }

    bool StringToInt(const char* string, int64_t& result)
    {
        const size_t length = StringLength(string);
        if (!length)
        {
            return false;
        }

        bool isNegative = false;

        for (size_t i = 0; i < length; ++i)
        {
            if (string[i] == '-')
            {
                if (isNegative)
                {
                    return false;
                }
                isNegative = true;
            }
        }

        result = 0.0;
        size_t power = length - 1;
        for (size_t i = 0; i < length; ++i)
        {
            if (string[i] != '0')
            {
                int64_t basePower = 1.0;
                for (int k = 0; k < power; k++)
                {
                    basePower *= 10.0;
                }
                result += (string[i] - '0') * basePower;
            }
            power--;
        }
        if (isNegative)
        {
            result *= -1;
        }
        return true;
    }

    //------------------------------------------------------------//

    //--------------------UUID------------------------------------//
#if defined (GEDO_OS_WINDOWS)
    UUId GenerateUUID()
    {
        GUID uuid;
        UuidCreate(&uuid);

        UUId result
        {
            (uint8_t)((uuid.Data1 >> 24) & 0xFF),
            (uint8_t)((uuid.Data1 >> 16) & 0xFF),
            (uint8_t)((uuid.Data1 >> 8) & 0xFF),
            (uint8_t)((uuid.Data1) & 0xff),

            (uint8_t)((uuid.Data2 >> 8) & 0xFF),
            (uint8_t)((uuid.Data2) & 0xff),

            (uint8_t)((uuid.Data3 >> 8) & 0xFF),
            (uint8_t)((uuid.Data3) & 0xFF),

            (uint8_t)uuid.Data4[0],
            (uint8_t)uuid.Data4[1],
            (uint8_t)uuid.Data4[2],
            (uint8_t)uuid.Data4[3],
            (uint8_t)uuid.Data4[4],
            (uint8_t)uuid.Data4[5],
            (uint8_t)uuid.Data4[6],
            (uint8_t)uuid.Data4[7]
        };
        return result;
    }

#elif defined (GEDO_OS_LINUX)
    UUId GenerateUUID()
    {
        UUId result;
        uuid_generate(result.data);
        return result;
    }
#endif
    bool CompareUUID(const UUId& a, const UUId& b)
    {
        for (size_t i = 0; i < 16; ++i)
        {
            if (a.data[i] != b.data[i])
            {
                return false;
            }
        }
        return true;
    }
    //------------------------------------------------------------//

    //--------------------Math-----------------------------------//
    Vec2d operator+(const Vec2d a, const Vec2d& b)
    {
        return Vec2d{ a.x + b.x, a.y + b.y };
    }

    Vec2d operator-(const Vec2d a, const Vec2d& b)
    {
        return Vec2d{ a.x - b.x, a.y - b.y };
    }

    Vec2d operator*(const Vec2d a, const Vec2d& b)
    {
        return Vec2d{ a.x * b.x, a.y * b.y };
    }

    Vec2d operator*(const Vec2d a, double x)
    {
        return Vec2d{ a.x * x, a.y * x };
    }

    Vec2d operator*(double x, const Vec2d a)
    {
        return Vec2d{ a.x * x, a.y * x };
    }

    Vec3d operator+(const Vec3d a, const Vec3d& b)
    {
        return Vec3d{ a.x + b.x, a.y + b.y, a.z + b.z };
    }

    Vec3d operator-(const Vec3d a, const Vec3d& b)
    {
        return Vec3d{ a.x - b.x, a.y - b.y, a.z - b.z };
    }

    Vec3d operator*(const Vec3d a, const Vec3d& b)
    {
        return Vec3d{ a.x * b.x, a.y * b.y, a.z * b.z };
    }

    Vec3d operator*(const Vec3d a, double x)
    {
        return Vec3d{ a.x * x, a.y * x, a.z * x };
    }

    Vec3d operator*(double x, const Vec3d a)
    {
        return Vec3d{ a.x * x, a.y * x, a.z * x };
    }

    double DotProduct(const Vec3d a, const Vec3d& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    double DotProduct(const Vec2d a, const Vec2d& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    Mat4 operator*(const Mat4& left, const Mat4& right)
    {
        Mat4 result;
        for (int columns = 0; columns < 4; ++columns)
        {
            for (int rows = 0; rows < 4; ++rows)
            {
                double sum = 0;
                for (int currentMatrice = 0; currentMatrice < 4; ++currentMatrice)
                {
                    sum += left.elements[currentMatrice][rows] * right.elements[columns][currentMatrice];
                }
                result.elements[columns][rows] = sum;
            }
        }
        return result;
    }

    Mat4 operator*(Mat4 left, double x)
    {
        for (int columns = 0; columns < 4; ++columns)
        {
            for (int rows = 0; rows < 4; ++rows)
            {
                left.elements[columns][rows] *= x;
            }
        }
        return left;
    }

    Vec3d operator*(const Mat3 a, const Vec3d& v)
    {
        Vec3d r;
        for (int rows = 0; rows < 3; ++rows)
        {
            double sum = 0;
            for (int columns = 0; columns < 3; ++columns)
            {
                sum += a.elements[columns][rows] * v.data[columns];
            }
            r.data[rows] = sum;
        }
        return r;
    }

    Vec3d CrossProduct(const Vec3d a, const Vec3d& b)
    {
        return Vec3d{ a.y * b.z - a.z * b.y,
                      a.z * b.x - a.x * b.z,
                      a.x * b.y - a.y * b.x };
    }

    double Length(const Vec2d& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y);
    }

    double Length(const Vec3d& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    void Normalise(Vec2d& v)
    {
        const double length = Length(v);
        v.x /= length;
        v.y /= length;
    }

    Vec2d Normalised(const Vec2d& v)
    {
        const double length = Length(v);
        return Vec2d{ v.x / length, v.y / length };
    }

    void Normalise(Vec3d& v)
    {
        const double length = Length(v);
        v.x /= length;
        v.y /= length;
        v.z /= length;
    }

    Vec3d Normalised(const Vec3d& v)
    {
        const double length = Length(v);
        return Vec3d{ v.x / length, v.y / length, v.z / length };
    }

    Mat4 Transpose(Mat4 m)
    {
        Swap(m.elements[0][1], m.elements[1][0]);
        Swap(m.elements[0][2], m.elements[2][0]);
        Swap(m.elements[0][3], m.elements[3][0]);
        Swap(m.elements[1][2], m.elements[2][1]);
        Swap(m.elements[1][3], m.elements[3][1]);
        Swap(m.elements[2][3], m.elements[3][2]);
        return m;
    }

    Mat3 Transpose(Mat3 m)
    {
        Swap(m.elements[0][1], m.elements[1][0]);
        Swap(m.elements[0][2], m.elements[2][0]);
        Swap(m.elements[1][2], m.elements[2][1]);
        return m;
    }

    Mat4 Translate(const Mat4& m, Vec3d translation)
    {
        Mat4 translate = Identity();
        translate.elements[3][0] = translation.x;
        translate.elements[3][1] = translation.y;
        translate.elements[3][2] = translation.z;
        return m * translate;
    }

    Mat4 Rotate(const Mat4& m, double angle, Vec3d axis)
    {
        const double sinTheta = sinf(angle);
        const double cosTheta = cosf(angle);
        const double cosValue = 1.0f - cosTheta;

        Normalise(axis);
        Mat4 rotate = Identity();

        rotate.elements[0][0] = (axis.x * axis.x * cosValue) + cosTheta;
        rotate.elements[0][1] = (axis.x * axis.y * cosValue) + (axis.z * sinTheta);
        rotate.elements[0][2] = (axis.x * axis.z * cosValue) - (axis.y * sinTheta);

        rotate.elements[1][0] = (axis.y * axis.x * cosValue) - (axis.z * sinTheta);
        rotate.elements[1][1] = (axis.y * axis.y * cosValue) + cosTheta;
        rotate.elements[1][2] = (axis.y * axis.z * cosValue) + (axis.x * sinTheta);

        rotate.elements[2][0] = (axis.z * axis.x * cosValue) + (axis.y * sinTheta);
        rotate.elements[2][1] = (axis.z * axis.y * cosValue) - (axis.x * sinTheta);
        rotate.elements[2][2] = (axis.z * axis.z * cosValue) + cosTheta;

        return m * rotate;
    }

    Mat4 Identity()
    {
        Mat4 m = { 0 };
        m.elements[0][0] = m.elements[1][1] = m.elements[2][2] = m.elements[3][3] = 1.0f;
        return m;
    }

    Mat4 LookAt(const Vec3d& eye, const Vec3d& center, const Vec3d& up)
    {
        const Vec3d f = Normalised(center - eye);
        const Vec3d s = Normalised(CrossProduct(f, up));
        const Vec3d u = CrossProduct(s, f);

        Mat4 dest;
        dest.elements[0][0] = s.x;
        dest.elements[0][1] = u.x;
        dest.elements[0][2] = -f.x;
        dest.elements[0][3] = 0;

        dest.elements[1][0] = s.y;
        dest.elements[1][1] = u.y;
        dest.elements[1][2] = -f.y;
        dest.elements[1][3] = 0;

        dest.elements[2][0] = s.z;
        dest.elements[2][1] = u.z;
        dest.elements[2][2] = -f.z;
        dest.elements[2][3] = 0;

        dest.elements[3][0] = -DotProduct(s, eye);
        dest.elements[3][1] = -DotProduct(u, eye);
        dest.elements[3][2] = DotProduct(f, eye);
        dest.elements[3][3] = 1.0f;
        return dest;
    }

    Mat4 Perspective(double fovy, double aspect, double zNear, double zFar)
    {
        const double f = 1.0f / tanf(fovy * 0.5f);
        const double fn = 1.0f / (zNear - zFar);
        Mat4 dest = { 0 };
        dest.elements[0][0] = f / aspect;
        dest.elements[1][1] = f;
        dest.elements[2][2] = (zNear + zFar) * fn;
        dest.elements[2][3] = -1.0f;
        dest.elements[3][2] = 2.0f * zNear * zFar * fn;
        return dest;
    }

    double Deg2Rad(double v)
    {
        return (PI / 180.0) * v;
    }

    double Rad2Deg(double v)
    {
        return (180.0 / PI) * v;
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
    //----------------------------------------------------------//

    //------------------------IO-------------------------------//
#if defined (GEDO_OS_WINDOWS)
    void PrintToConsole(const char* text, ConsoleColor color)
    {
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        GEDO_ASSERT(hStdout);
        Utf16String string = UTF8ToUTF16(text, GetDefaultAllocator());
        defer(FreeUtf16String(string));

        // Remember how things were when we started
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hStdout, &consoleInfo);
        defer(SetConsoleTextAttribute(hStdout, consoleInfo.wAttributes));

        int attributes = consoleInfo.wAttributes;
        switch (color)
        {
        case ConsoleColor::WHITE:  break;
        case ConsoleColor::RED:   attributes = FOREGROUND_RED;   break;
        case ConsoleColor::BLUE:  attributes = FOREGROUND_BLUE;  break;
        case ConsoleColor::GREEN: attributes = FOREGROUND_GREEN; break;
        }
        SetConsoleTextAttribute(hStdout, attributes);

        DWORD written = 0;
        WriteConsoleW(hStdout, string.text, string.size, &written, NULL);
    }

    void ReadFromConsole(char* buffer, size_t bufferSize)
    {
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        GEDO_ASSERT(hStdin);
        DWORD read = 0;
        ReadConsoleA(hStdin, buffer, bufferSize, &read, NULL);
        if (read >= 2 && buffer[read - 1] == '\n' && buffer[read - 2] == '\r')
        {
            buffer[read - 2] = 0;
        }
    }

    void ClearConsole()
    {
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        GEDO_ASSERT(hStdout);

        DWORD mode = 0;
        GetConsoleMode(hStdout, &mode);
        const DWORD originalMode = mode;
        defer(SetConsoleMode(hStdout, originalMode));

        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hStdout, mode);

        DWORD written = 0;
        WCHAR sequence[] = L"\x1b[2J";
        WriteConsoleW(hStdout, sequence, ARRAYSIZE(sequence), &written, NULL);
    }
#elif defined (GEDO_OS_LINUX)
    void PrintToConsole(const char* text, ConsoleColor color)
    {
#define GEDO_CONSOLE_COLOR_RESET  "\033[0m"
#define GEDO_CONSOLE_COLOR_RED    "\033[31m"
#define GEDO_CONSOLE_COLOR_GREEN  "\033[32m"
#define GEDO_CONSOLE_COLOR_BLUE   "\033[34m"
#define GEDO_CONSOLE_COLOR_WHITE  "\033[37m"
        switch (color)
        {
        case ConsoleColor::WHITE: printf(GEDO_CONSOLE_COLOR_WHITE); break;
        case ConsoleColor::RED:   printf(GEDO_CONSOLE_COLOR_RED);   break;
        case ConsoleColor::BLUE:  printf(GEDO_CONSOLE_COLOR_BLUE);  break;
        case ConsoleColor::GREEN: printf(GEDO_CONSOLE_COLOR_GREEN); break;
        }
        printf(text);
        printf(GEDO_CONSOLE_COLOR_RESET);

#undef GEDO_CONSOLE_COLOR_RESET
#undef GEDO_CONSOLE_COLOR_RED
#undef GEDO_CONSOLE_COLOR_GREEN
#undef GEDO_CONSOLE_COLOR_BLUE
#undef GEDO_CONSOLE_COLOR_WHITE
    }

    void ReadFromConsole(char* buffer, size_t bufferSize)
    {
        fgets(buffer, bufferSize, stdin);
    }

    void ClearConsole()
    {
        printf("\e[1;1H\e[2J");
    }
#endif
    void PrintToConsole(char c, ConsoleColor color)
    {
        char text[2] = {};
        text[0] = c;
        PrintToConsole(text, color);
    }
    //----------------------------------------------------------//

    //------------------------Time-------------------------------//
#if defined (GEDO_OS_WINDOWS)
    static double GetPerformanceFrequency()
    {
        LARGE_INTEGER PerfCountFrequencyResult;
        QueryPerformanceFrequency(&PerfCountFrequencyResult);
        return PerfCountFrequencyResult.QuadPart;
    }

    void StartStopWatch(StopWatch& in)
    {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        in.start = t.QuadPart;
    }

    void StopStopWatch(StopWatch& in)
    {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        in.end = t.QuadPart;
    }

    double ElapsedSeconds(const StopWatch& in)
    {
        const static double frequency = GetPerformanceFrequency();
        return (in.end - in.start) / frequency;
    }
#elif defined (GEDO_OS_LINUX)
    void StartStopWatch(StopWatch& in)
    {
        time_t t;
        time(&t);
        in.start = t;
    }

    void StopStopWatch(StopWatch& in)
    {
        time_t t;
        time(&t);
        in.end = t;
    }

    double ElapsedSeconds(const StopWatch& in)
    {
        return difftime(in.end, in.start);
    }
#endif
    //----------------------------------------------------------//
}
