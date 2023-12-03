#include "win32_utils.hpp"

#define Print(format, ...) logA(format, __VA_ARGS__)
#define Assert(Expression, msg)  \
    if (!(Expression))           \
    {                            \
        Print(msg);              \
        __debugbreak();          \
    }                            \

// #################################################
//                   IO UTILS 
// #################################################
DWORD GetFileSize(const char* fileName)
{
    _WIN32_FILE_ATTRIBUTE_DATA fileData;

    bool success = GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileData);

    return fileData.nFileSizeLow;
}

FileResult ReadTextFile(const char* fileName, DWORD fileSize, MemoryArena* memoryArena) 
{


    HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ,
            0, OPEN_EXISTING, 0, 0);

    LPVOID buffer = (char*)memoryArena->memory + memoryArena->offset;
    memoryArena->offset += fileSize;
    DWORD bytesRead;
    FileResult fileResult = {};
    fileResult.success = false;
    fileResult.fileSize = fileSize;
    if (ReadFile(fileHandle, buffer, fileSize, &bytesRead, 0))
    {
        if (bytesRead != fileSize)
        {
            VirtualFree(buffer, 0, MEM_RELEASE);
            return fileResult;
        }

        fileResult.success = true;
        fileResult.result = buffer;
    }

    CloseHandle(fileHandle);
    return fileResult;
}


// #################################################
//                   MEMORY UTILS 
// #################################################
inline void* AllocateMemory(MemoryArena* memoryArena, int size)
{
    Assert(memoryArena->offset + size <= memoryArena->totalSize, "out of mem");
    void* memory = (void*)((char*)memoryArena->memory + memoryArena->offset);
    memoryArena->offset += size;
    return memory;
}

inline void FreeMemory(void* memory, int size)
{
    // TODO: this will leave gaps in our arena
    // for now we can just allocate more memory in our memory arena at startup
    // to avoid problems...
    VirtualFree(memory, size, MEM_DECOMMIT);
}

inline void FreeAllMemory(MemoryArena* memoryArena)
{
    VirtualFree(memoryArena->memory, 0, MEM_RELEASE);
}


// #################################################
//                   LOGGING UTILS 
// #################################################
inline void logA(const char* format, ...)
{
    char buf[1024];
    wvsprintfA(buf, format, ((char*)&format) + sizeof(void*));
    OutputDebugStringA(buf);
}

// #################################################
//                   MATH UTILS 
// #################################################
inline int Pow(int number, int power)
{
    if (power == 0)
    {
        return 1;
    }

    int result = number;
    while (power > 1)
    {
        result *= number;
        power--;
    }

    return result;
}


// #################################################
//                   STRING UTILS 
// #################################################
inline int CharToInt(char c) {
    return c - '0';
}


inline EatResult EatDelimeter(char*& s, char delim)
{
    EatResult eatResult = {};
    if (s[0] == delim)
    {
        s++;
        eatResult.success = true;
    }

    return eatResult;
}

inline EatResult EatSpaces(char*& s)
{
    EatResult eatResult = {};
    char current = s[0];
    while (current == 32)
    {
        s++;
        current = s[0];
        eatResult.success = true;
    }

    return eatResult;
}

EatResult EatWord(char*& s, MemoryArena *memoryArena)
{
    EatResult eatResult = {};
    char* startingString = s;
    char current = s[0];
    int length = 0;
    while ((current >= 97 && current <= 122) ||
        (current >= 65 && current <= 90))
    {
        length++;
        s++;
        current = s[0];
        eatResult.success = true;
    }

    eatResult.result = (char*)AllocateMemory(memoryArena, length + 1);
    CopyMemory(eatResult.result, startingString, length + 1);
    eatResult.result[length] = '\0';
    return eatResult;
}

EatResult EatNumber(char*& s, MemoryArena *memoryArena)
{
    EatResult eatResult = {};
    char* startingString = s;
    char current = s[0];
    int length = 0;
    while ((current >= 48 && current <= 57))
    {
        length++;
        s++;
        current = s[0];
        eatResult.success = true;
    }
    eatResult.result = (char*)AllocateMemory(memoryArena, length + 1);
    CopyMemory(eatResult.result, startingString, length + 1);
    eatResult.result[length] = '\0';
    return eatResult;
}

inline bool CompareStringWithLiteral(char* str, const char* literal)
{
    int index = 0;
    while (str[index] != 0 || literal[index] != 0)
    {
        if (str[index] != literal[index]) return false;
        index++;
    }

    if (str[index] == 0 && literal[index] == 0)
    {
        return true;
    }

    return false;
}

inline int Length(char* s)
{
    int len = 0;
    while (*s)
    {
        len++;
        s++;
    }
    return len;
}

inline int StringToInt(char* s)
{
    int result = 0;
    int len = Length(s);
    int index = 0;
    while (*s)
    {
        result += Pow(10, len - 1) * CharToInt(s[index]);
        s++;
        len--;
    }
    return result;
}
