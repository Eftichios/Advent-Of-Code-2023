#include "../common/win32_utils.cpp"

#define MAX_WINNING_NUMBERS 1024

struct CardResult {
    int cardId;
    bool success;

    int numberOfWinners;
};

struct HashNode {
    int key;
    HashNode* nextNode;
};

CardResult ParseOneCard(char*& s, MemoryArena* memoryArena)
{

    HashMap<HashNode> hashMap = {};
    HashNode* freeTable[MAX_WINNING_NUMBERS];
    int allocatedHashNodes = 0;

    CardResult cardResult = {};

    EatResult eatResult = {};
    eatResult = EatWord(s, memoryArena);
    eatResult = EatSpaces(s);
    eatResult = EatNumber(s, memoryArena);
    cardResult.cardId = StringToInt(eatResult.result);

    eatResult = EatDelimeter(s, ':');

    // winning numbers
    while (*s != '|')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        int winningNumber = StringToInt(eatResult.result);

        HashNode newHashNode = {};
        newHashNode.key = winningNumber;
        HashNode* hashNode = AddToHashMap(&hashMap, winningNumber, newHashNode, memoryArena);
        freeTable[allocatedHashNodes] = hashNode;
        allocatedHashNodes += 1;
    }

    eatResult = EatDelimeter(s, '|');

    while (*s != '\r' && *s != '\0')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        int ourNumber = StringToInt(eatResult.result);
        HashNode* hashNode = RetrieveFromHashMap(&hashMap, ourNumber);
        if (hashNode){
            cardResult.success = true;
            cardResult.numberOfWinners += 1;
        }
    }

    // advance \r\n
    if (*s != '\0')
    {
        s+=2;
    }

    for (int freeIndex = 0; freeIndex < allocatedHashNodes; freeIndex++)
    {
        HashNode* current = freeTable[freeIndex];
        while (current) {
            HashNode* next = current->nextNode;
            FreeMemory(current, sizeof(HashNode));
            current = next;
        }
    }

    return cardResult;
}

template <typename T>
void ProcessWinningCard(CardResult cardResult, int* result, Array<T>* cardCopies,
        MemoryArena* memoryArena)
{
    if (cardResult.success)
    {
        if (cardResult.numberOfWinners == 1)
        {
            *result += 1;
        }
        else
        {
            *result += Pow(2, cardResult.numberOfWinners - 1);
        }
        for (int cardCopyIndex = cardResult.cardId + 1;
             cardCopyIndex <= cardResult.cardId + cardResult.numberOfWinners;
             cardCopyIndex++)
        {
            PushArray(cardCopies, cardCopyIndex, memoryArena);
        }
    }
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena) 
{
    char *s = fileContents;
    int part1answer = 0;

    int fileRowLength = RowLength(fileContents);
    fileRowLength += 2;

    int maxCards = fileSize / fileRowLength;

    // temp hack to avoid sequential memory usage in our string
    // give some padding to our memory
    AllocateMemory(memoryArena, 1);

    Array<int> cardCopies = {};
    InitArrayData(&cardCopies, memoryArena);

    Array<CardResult> cardResults = {};
    InitArrayData(&cardResults, memoryArena, maxCards);
    
    while (*s)
    {
        CardResult cardResult = ParseOneCard(s, memoryArena);
        PushArray(&cardResults, cardResult, memoryArena);
        ProcessWinningCard(cardResult, &part1answer, &cardCopies, memoryArena);
    }

    Print("The sum of the winning number points is: %d", part1answer);

    int scratchCardCopies = 0;
    while (cardCopies.currentCapacity > 0)
    {
        int cardResultId = PopArray(&cardCopies);
        //CardResult cardResult = GetArray(&cardResults, cardResultId-1);
        CardResult cardResult = cardResults.data[cardResultId-1];
        ProcessWinningCard(cardResult, &part1answer, &cardCopies, memoryArena);
        scratchCardCopies += 1;
    }

    int part2answer = scratchCardCopies + cardResults.currentCapacity;

    Print("The sum of the scratchcards is: %d", part2answer);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nShowCmd)
{
    const char* fileName = "input.txt";
    DWORD fileSize = GetFileSize(fileName);

    MemoryArena memoryArena = {};
    memoryArena.totalSize = fileSize + Kilobytes(100);
    memoryArena.memory = VirtualAlloc(0, memoryArena.totalSize, 
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    FileResult fileResult = ReadTextFile(fileName, fileSize, &memoryArena);

    ParseFileContents((char*)fileResult.result, fileSize, &memoryArena);

    FreeAllMemory(&memoryArena);
    return 0;
}
