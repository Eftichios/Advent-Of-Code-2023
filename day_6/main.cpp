#include "../common/win32_utils.cpp"

int Part1Solution(char*& s, MemoryArena* memoryArena)
{
    Array<int> times = {};
    InitArrayData(&times, memoryArena, 16);
    Array<int> bestDistances = {};
    InitArrayData(&bestDistances, memoryArena, 16);

    EatResult eatResult = {};
    eatResult = EatWord(s, memoryArena);
    eatResult = EatDelimeter(s, ':');

    while (*s != '\r')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        int time = StringToInt(eatResult.result);

        PushArray(&times, time, memoryArena);
    }
    s += 2;

    eatResult = EatWord(s, memoryArena);
    eatResult = EatDelimeter(s, ':');
    while (*s != '\r' && *s != '\0')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        int bestDistance = StringToInt(eatResult.result);

        PushArray(&bestDistances, bestDistance, memoryArena);
    }

    if (*s == '\r') s += 2;

    int result = 1;

    for (int raceId = 0; raceId < times.currentCapacity; raceId++)
    {
        int time = times[raceId];
        int bestDistance = bestDistances[raceId];

        int numberOfWins = 0;
        for (int windUp = 0; windUp  <= time; windUp++)
        {
            int distance = windUp * (time - windUp);
            if (distance > bestDistance)
            {
                numberOfWins++;
            }
        }
        result *= numberOfWins;
    }
    return result;
}

int Part2Solution(char*& s, MemoryArena* memoryArena)
{
    char* timeInString = 0;
    char* bestDistanceInString = 0;

    EatResult eatResult = {};
    eatResult = EatWord(s, memoryArena);
    eatResult = EatDelimeter(s, ':');

    while (*s != '\r')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        timeInString = ConcatStrings(timeInString, eatResult.result, memoryArena);
    }
    ulong32 time = StringToUlong32(timeInString);
    s += 2;

    eatResult = EatWord(s, memoryArena);
    eatResult = EatDelimeter(s, ':');
    while (*s != '\r' && *s != '\0')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        bestDistanceInString = ConcatStrings(bestDistanceInString, eatResult.result, memoryArena);
    }
    ulong64 bestDistance = StringToUlong64(bestDistanceInString);

    if (*s == '\r') s += 2;

    int numberOfWins = 0;
    for (ulong64 windUp = 0; windUp <= time; windUp++)
    {
        ulong64 distance = windUp * (time - windUp);
        if (distance > bestDistance)
        {
            numberOfWins++;
        }
    }
    return numberOfWins;
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena) 
{

    // temp hack to avoid sequential memory usage in our string
    // give some padding to our memory
    AllocateMemory(memoryArena, 1);
    char* s = fileContents;
    char* t = fileContents;

    Array<char> arrayOfNumbersToConcat = {};
    InitArrayData(&arrayOfNumbersToConcat, memoryArena, 16);

    int part1result = Part1Solution(s, memoryArena);
    int part2result = Part2Solution(t, memoryArena);
    
    Print("Result of multiplying ways to win of each race is %d", part1result);
    Print("Result of single race ways to win %d", part2result);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nShowCmd)
{
    const char* fileName = "input.txt";
    DWORD fileSize = GetFileSize(fileName);

    MemoryArena memoryArena = {};
    memoryArena.totalSize = fileSize + Kilobytes(1);
    memoryArena.memory = VirtualAlloc(0, memoryArena.totalSize, 
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    FileResult fileResult = ReadTextFile(fileName, fileSize, &memoryArena);

    ParseFileContents((char*)fileResult.result, fileSize, &memoryArena);

    FreeAllMemory(&memoryArena);
    return 0;
}
