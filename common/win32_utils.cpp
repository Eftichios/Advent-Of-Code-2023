#include "win32_utils.hpp"

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

#define Print(format, ...) logA(format, __VA_ARGS__)

