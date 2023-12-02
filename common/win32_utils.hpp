#pragma once
#include <windows.h>

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

inline void FreeAllMemory(MemoryArena* memoryArena);

DWORD GetFileSize(const char* fileName);

FileResult ReadTextFile(const char* fileName, DWORD fileSize, MemoryArena* memoryArena);

inline void Print(const char* msg);

