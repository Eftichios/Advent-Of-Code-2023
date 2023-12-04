#pragma once
#include <windows.h>

#define Kilobytes(value) ((value) * 1024)
#define INITIAL_ARRAY_SIZE 256

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

template <typename T>
struct Array {
    T* data;
    int currentCapacity;
    int maxCapacity = INITIAL_ARRAY_SIZE;

    T operator[](int index) {
        return *(data + index);
    }
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
EatResult ExtractNumber(char* s, MemoryArena *memoryArena);
EatResult EatNumber(char*& s, MemoryArena *memoryArena);
inline bool CompareStringWithLiteral(char* str, const char* literal);
inline int Length(char* s);
inline int RowLength(char* s);
inline int StringToInt(char* s);

template <typename T>
void PushArray(Array<T>* array, T value, MemoryArena* memoryArena);
template <typename T>
T PopArray(Array<T>* array);
