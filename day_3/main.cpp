#include "../common/win32_utils.cpp"

struct HashNode {
    int key;

    int numberOfParts;
    int numbers[2];
    HashNode* nextNode;
};

bool CheckSquareForSymbol(int rowOffset, int columnOffset, 
        char* fileContents, int index, int rowLength, int num, HashMap<HashNode>* hashMap, 
        MemoryArena* memoryArena, bool *wasAlreadyCounted)
{
    int key = index + rowOffset + columnOffset * rowLength;
    char square = fileContents[key];

    if (square != '.' && (square < 48 || square >57))
    {
        if (square == '*' && !*wasAlreadyCounted)
        {
            HashNode* hashNode = RetrieveFromHashMap(hashMap, key);
            if (hashNode)
            {
                hashNode->numberOfParts += 1;
                if (hashNode->numberOfParts <= 2)
                {
                    hashNode->numbers[hashNode->numberOfParts - 1] = num;
                }
            } else 
            {

                HashNode newHashNodeValue = {};
                newHashNodeValue.key = key;
                newHashNodeValue.numberOfParts = 1;
                newHashNodeValue.numbers[0] = num;
                AddToHashMap(hashMap, key, newHashNodeValue, memoryArena);
            }
            *wasAlreadyCounted = true;
        }
        return true;
    }
    return false;
}

bool CheckIfAdjacentToSymbol(char* fileContents, int numLength, int index, 
        int rowLength, DWORD fileSize, int num, HashMap<HashNode>* hashMap, MemoryArena* memoryArena)
{
    // NOTE: We don't return early so we can identify all gear parts!
    bool success = false;
    bool wasAlreadyCounted = false;
    for (int numIndex = 0; numIndex < numLength; numIndex++)
    {
        int totalOffset = index + numIndex;

        if (totalOffset > rowLength && totalOffset % rowLength > 0)
        {
            // top left
            if(CheckSquareForSymbol(-1, -1, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

        if (totalOffset % rowLength > 0)
        {
            // left
            if(CheckSquareForSymbol(-1, 0, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

        if (totalOffset % rowLength > 0 && totalOffset + rowLength < fileSize)
        {
            // bottom left
            if(CheckSquareForSymbol(-1, 1, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

        if (totalOffset > rowLength)
        {
            // top
            if(CheckSquareForSymbol(0, -1, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

        if (totalOffset + rowLength < fileSize)
        {
            // bottom
            if(CheckSquareForSymbol(0, 1, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

        if (totalOffset > rowLength && (totalOffset+3) % rowLength != 0)
        {
            // top right
            if(CheckSquareForSymbol(1, -1, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

        // +3 to account for 0-based index and \r\n characters as they are
        // not yet encountered in this row
        if ((totalOffset+3) % rowLength != 0)
        {
            // right
            if(CheckSquareForSymbol(1, 0, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

        if ((totalOffset+3) % rowLength != 0 && totalOffset + rowLength < fileSize)
        {
            // bottom right
            if(CheckSquareForSymbol(1, 1, fileContents, totalOffset, rowLength, num, hashMap,
                        memoryArena, &wasAlreadyCounted))
            {
                success = true;
            }
        }

    }
    return success;
}

int ParseHashMap(HashMap<HashNode>* hashMap)
{
    int result = 0;
    for (int slot = 0; slot < HASH_SIZE; slot++)
    {
        HashNode* current = hashMap->hashNodes[slot];

        while (current)
        {
            if (current->numberOfParts == 2)
            {
                result += current->numbers[0] * current->numbers[1];
            }
            current = current->nextNode;
        }
    }
    return result;
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena)
{
    // add +2 for \r\n
    int rowLength = RowLength(fileContents) + 2;
    int result = 0;

    HashMap<HashNode> hashMap = {};
    
    for (DWORD index = 0; index < fileSize; index++)
    {
        char current = fileContents[index];

        if (current != '.')
        {
            if (current >= 48 && current <= 57)
            {
                EatResult eatResult = ExtractNumber(&fileContents[index], memoryArena);
                int numLength = Length(eatResult.result);
                int num = StringToInt(eatResult.result);

                if (CheckIfAdjacentToSymbol(fileContents, numLength, index, 
                            rowLength, fileSize, num, &hashMap, memoryArena))
                {
                    result += num;
                }
                index += numLength - 1;
            }
        }
    }
    int gearParts = ParseHashMap(&hashMap);
    Print("The sum of all parts is: %d", result);
    Print("The sum of gear part ratios is: %d", gearParts);
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
