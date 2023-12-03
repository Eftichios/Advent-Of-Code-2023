#include "../common/win32_utils.cpp"

struct GameSet {
    int red;
    int green;
    int blue;
};

struct GameResult {
    int gameId;
    bool success;

    int maxRed;
    int maxGreen;
    int maxBlue;
};


void UpdateBall(GameSet* gameSet, char* ballColour, int ballValue)
{
    if (CompareStringWithLiteral(ballColour, "red"))
    {
        gameSet->red = ballValue;
    }
    else if (CompareStringWithLiteral(ballColour, "green"))
    {
        gameSet->green = ballValue;
    }
    else if (CompareStringWithLiteral(ballColour, "blue"))
    {
        gameSet->blue = ballValue;
    }
}

bool CheckIfValid(GameSet givenSet, GameSet currentSet)
{
    if (currentSet.red > givenSet.red ||
        currentSet.green > givenSet.green ||
        currentSet.blue > givenSet.blue)
    {
        return false;
    }
    return true;
}

GameResult ParseOneGame(char*& s, MemoryArena *memoryArena, GameSet givenSet)
{
    GameResult gameResult = {};


    EatResult eatResult = {};
    eatResult = EatWord(s, memoryArena);
    eatResult = EatSpaces(s);
    eatResult = EatNumber(s, memoryArena);

    gameResult.gameId = StringToInt(eatResult.result);

    eatResult = EatDelimeter(s, ':');

    bool success = true;
    GameSet currentSet = {};
    while (*s != '\r' && *s != '\0')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        int ballValue = StringToInt(eatResult.result);

        eatResult = EatSpaces(s);
        eatResult = EatWord(s, memoryArena);
        char* ballColour = eatResult.result;
        eatResult = EatDelimeter(s, ',');
        
        UpdateBall(&currentSet, ballColour, ballValue);
        if (*s == ';' || *s == '\r' || *s == '\0')
        {
            // game set yo
            if (!CheckIfValid(givenSet, currentSet))
            {
                success = false;
            }
            EatDelimeter(s, ';');
            if (currentSet.red > gameResult.maxRed)
            {
                gameResult.maxRed = currentSet.red;
            }
            if (currentSet.green > gameResult.maxGreen)
            {
                gameResult.maxGreen = currentSet.green;
            }
            if (currentSet.blue > gameResult.maxBlue)
            {
                gameResult.maxBlue = currentSet.blue;
            }

            currentSet.red = 0;
            currentSet.green = 0;
            currentSet.blue = 0;
        }
    }

    // advance \r\n
    if (*s != '\0')
    {
        s+=2;
    }

    gameResult.success = success;
    return gameResult;
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena)
{

    // temp hack to avoid sequential memory usage in our string
    // give some padding to our memory
    AllocateMemory(memoryArena, 1);

    int idResult = 0;
    int minPowerResult = 0;
    char *s = fileContents;
    GameSet givenSet = {};
    givenSet.red = 12;
    givenSet.green = 13;
    givenSet.blue = 14;

    while (*s)
    {
        GameResult gameResult = ParseOneGame(s, memoryArena, givenSet);
        minPowerResult += gameResult.maxRed * gameResult.maxGreen * gameResult.maxBlue;

        if (gameResult.success)
        {
            idResult += gameResult.gameId;
        }
    }

    Print("The result of all the successful game ids is", idResult);
    Print("The result of all the miminum powers is", minPowerResult);

}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nShowCmd)
{
    const char* fileName = "input.txt";
    DWORD fileSize = GetFileSize(fileName);

    MemoryArena memoryArena = {};
    memoryArena.totalSize = fileSize + Kilobytes(10);
    memoryArena.memory = VirtualAlloc(0, memoryArena.totalSize, 
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    FileResult fileResult = ReadTextFile(fileName, fileSize, &memoryArena);

    ParseFileContents((char*)fileResult.result, fileSize, &memoryArena);
}
