#pragma once
#include <windows.h>

#define Kilobytes(value) ((value) * 1024)

struct FileResult
{
    bool success;
    void* result;
    DWORD fileSize;
};

struct MemoryArena
{
    void* memory;
    int offset;
    int totalSize;
};

struct EatResult
{
    bool success;
    char* result;
};

inline void* AllocateMemory(MemoryArena* memoryArena, int size);
inline void FreeMemory(void* memory, int size);
inline void FreeAllMemory(MemoryArena* memoryArena);

DWORD GetFileSize(const char* fileName);
FileResult ReadTextFile(const char* fileName, DWORD fileSize, MemoryArena* memoryArena);

inline void logA(const char* format, ...);

inline int Pow(int number, int power);

inline int CharToInt(char c);
inline EatResult EatDelimeter(char*& s, char delim);
inline EatResult EatSpaces(char*& s);
EatResult EatWord(char*& s, MemoryArena *memoryArena);
EatResult EatNumber(char*& s, MemoryArena *memoryArena);
inline bool CompareStringWithLiteral(char* str, const char* literal);
inline int Length(char* s);
inline int StringToInt(char* s);
