#include "../common/win32_utils.cpp"

struct HashNode {
    int key;

    char* node;
    char* left;
    char* right;

    HashNode* nextNode;
};

int CreateHashKeyFromNode(char *s)
{
    int result = 0;
    char current = s[0];
    while (current) 
    {
        result = result * 26 + (current - 'A');
        s++;
        current = s[0];
    }
    return result;
}

bool NodeEndsInChar(char* node, char c)
{
    int length = Length(node);
    if (node[length - 1] == c) return true;
    return false;
}

int Part1Solution(HashMap<HashNode>* nodes, char* path)
{
    int steps = 0;
    int pathCounter = 0;

    int pathLength = Length(path);

    int startHashKey = CreateHashKeyFromNode((char*)"AAA");
    HashNode* current = RetrieveFromHashMap(nodes, startHashKey);
    while (!CompareStringWithLiteral(current->node, "ZZZ"))
    {
            char turn = path[pathCounter];


            int hashKey;
            if (turn == 'L')
            {
                hashKey = CreateHashKeyFromNode(current->left);
                current = RetrieveFromHashMap(nodes, hashKey);
            }
            else
            {
                hashKey = CreateHashKeyFromNode(current->right);
                current = RetrieveFromHashMap(nodes, hashKey);
            }


            steps += 1;
            pathCounter += 1;

            if (path[pathCounter] == '\0') pathCounter = 0;
    }

    return steps;
}

ulong64 Part2Solution(HashMap<HashNode>* nodes, Array<int>* hashNodesThatEndInA, 
        char* path, MemoryArena* memoryArena)
{
    int steps = 0;
    int pathCounter = 0;

    int pathLength = Length(path);

    Array<int> currentStartNodeHashKeys = *hashNodesThatEndInA;
    Array<bool> nodeEndedInZ = {};
    InitArrayData(&nodeEndedInZ, memoryArena, currentStartNodeHashKeys.currentCapacity);

    Array<int> stepsToEndInZ = {};
    InitArrayData(&stepsToEndInZ, memoryArena, currentStartNodeHashKeys.currentCapacity);

    ArrayPopulateAllWithValue(&stepsToEndInZ, 0, memoryArena);
    ArrayPopulateAllWithValue(&nodeEndedInZ, false, memoryArena);

    while (!ArrayAllAreEqualToValue(&nodeEndedInZ, true))
    {
        for (int currHashKeyIndex = 0; currHashKeyIndex < currentStartNodeHashKeys.currentCapacity; 
                currHashKeyIndex++)
        {
            int currHashKey = currentStartNodeHashKeys[currHashKeyIndex];
            HashNode* startNode = RetrieveFromHashMap(nodes, currHashKey);

            HashNode* current = startNode;

            char turn = path[pathCounter];

            if (nodeEndedInZ[currHashKeyIndex]) continue;

            int hashKey;
            if (turn == 'L')
            {
                hashKey = CreateHashKeyFromNode(current->left);
                current = RetrieveFromHashMap(nodes, hashKey);
            }
            else
            {
                hashKey = CreateHashKeyFromNode(current->right);
                current = RetrieveFromHashMap(nodes, hashKey);
            }

            if (NodeEndsInChar(current->node, 'Z'))
            {
                nodeEndedInZ.data[currHashKeyIndex] = true;
                stepsToEndInZ.data[currHashKeyIndex] = steps + 1;
            }

            currentStartNodeHashKeys.data[currHashKeyIndex] = hashKey;

        }
        steps += 1;
        pathCounter += 1;

        if (path[pathCounter] == '\0') pathCounter = 0;
    }
    
    // find lowest common denominator of where each start node ended
    int first = stepsToEndInZ.data[0];
    int second = stepsToEndInZ.data[1];

    ulong64 lcm = LCM(first, second);
    for (int index = 2; index < stepsToEndInZ.currentCapacity; index++)
    {
        ulong64 curr = stepsToEndInZ.data[index];
        lcm = LCM(lcm, curr);
    }

    return lcm;
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena) 
{
    // temp hack to avoid sequential memory usage in our string
    // give some padding to our memory
    AllocateMemory(memoryArena, 1);

    HashMap<HashNode> nodes = {};
    Array<int> hashNodesThatEndInA = {};
    InitArrayData(&hashNodesThatEndInA, memoryArena);

    char* s = fileContents;

    EatResult eatResult = {};
    eatResult = EatWord(s, memoryArena);

    char* path = eatResult.result;

    s += 4;

    while (*s != '\0')
    {
        HashNode node = {};
        eatResult = EatWord(s, memoryArena);

        int hashKey = CreateHashKeyFromNode(eatResult.result);
        node.key = hashKey;
        node.node = eatResult.result;

        EatSpaces(s);
        EatDelimeter(s, '=');
        EatSpaces(s);
        EatDelimeter(s, '(');

        eatResult = EatWord(s, memoryArena);
        node.left = eatResult.result;
        EatDelimeter(s, ',');
        EatSpaces(s);
        eatResult = EatWord(s, memoryArena);
        node.right = eatResult.result;
        EatDelimeter(s, ')');

        AddToHashMap(&nodes, hashKey, node, memoryArena);

        if (*s != '\0')
        {
            s += 2;
        }

        if (NodeEndsInChar(node.node, 'A')) 
        {
            PushArray(&hashNodesThatEndInA, hashKey, memoryArena);
        }
    }

    int part1answer = Part1Solution(&nodes, path);
    ulong64 part2answer = Part2Solution(&nodes, &hashNodesThatEndInA,  path,  memoryArena);

    Print("The number of steps starting from AAA to ZZZ is %d\n", part1answer);
    Print("All starting nodes will end at z in %d steps\n", part2answer);
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
