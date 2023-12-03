#include "../common/win32_utils.cpp"

#define MAX_CHARACTERS 28
#define MAX_TRIE_LEVELS 6

enum LookUpCode {
    CHAR_NOT_IN_TRIE,
    CHAR_IN_TREE_BUT_NOT_WORD,
    CHAR_IN_TREE_AND_WORD,
    SPECIAL_CASE,
    INVALID_WORD
};

struct LookUpResult {
    LookUpCode lookUpCode;
    int value;
};

struct TrieNode
{
    TrieNode* children[MAX_CHARACTERS];
    char value;
    bool isWord = false;
    char num = 0;
    bool isRoot = false;
};

TrieNode* AllocateTrieNode(MemoryArena* memoryArena)
{
    TrieNode* trieNode = (TrieNode*)((char*)memoryArena->memory + memoryArena->offset);
    memoryArena->offset += sizeof(TrieNode);
    return trieNode;
}

TrieNode* CreateOrReturnTrieNode(TrieNode* parent, char value, MemoryArena* memoryArena)
{
    TrieNode* child = parent->children[value - 'a'];
    if (!child)
    {
        parent->children[value - 'a'] = AllocateTrieNode(memoryArena);
        return parent->children[value - 'a'];
    }
    return child;
}

void AddWordToTrie(TrieNode* trieNode, const char* word, int length, char value, MemoryArena* memoryArena)
{
    TrieNode* current = CreateOrReturnTrieNode(trieNode, word[0], memoryArena);
    for (int index = 0; index <= length; index++)
    {
        if (current->value == 0)
        {
            current->value = word[index];
        }
        if (index == length - 1)
        {
            current->num = value;
            current->isWord = true;
            return;
        }
        current = CreateOrReturnTrieNode(current, word[index + 1], memoryArena);
    }
}

LookUpResult CheckWordInTrie(TrieNode* trieNode, char* word, int length)
{
    LookUpResult lookUpResult = {};
    lookUpResult.lookUpCode = INVALID_WORD;

    if (word[0] == '\r')
    {
        return lookUpResult;
    }

    TrieNode* current = trieNode->children[word[0] - 'a'];

    for (int index = 0; index <= length; index++)
    {
        if (!current)
        {
            lookUpResult.lookUpCode = CHAR_NOT_IN_TRIE;
            return lookUpResult;
        } 
        else 
        {
            if (index == length - 1) 
            {
                if (current->isWord) 
                {
                    if (trieNode->children[current->value - 'a']){
                        lookUpResult.lookUpCode = SPECIAL_CASE;
                        lookUpResult.value = current->num;
                        return lookUpResult;
                    }
                    lookUpResult.lookUpCode = CHAR_IN_TREE_AND_WORD;
                    lookUpResult.value = current->num;
                    return lookUpResult;
                }
                lookUpResult.lookUpCode = CHAR_IN_TREE_BUT_NOT_WORD;
                return lookUpResult;
            } 
            else 
            {    
                if (word[index+1] == '\r')
                {
                    return lookUpResult;
                }
                current = current->children[word[index+1] - 'a'];
            }
        }
    }
    return lookUpResult;
}

TrieNode* InitTrie(MemoryArena* memoryArena)
{
    TrieNode* trieNode = (TrieNode*)VirtualAlloc(0, sizeof(TrieNode),
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); 
    trieNode->isRoot = true;
    AddWordToTrie(trieNode, "one", 3, '1', memoryArena);
    AddWordToTrie(trieNode, "two", 3, '2', memoryArena);
    AddWordToTrie(trieNode, "three", 5, '3', memoryArena);
    AddWordToTrie(trieNode, "four", 4, '4', memoryArena);
    AddWordToTrie(trieNode, "five", 4, '5', memoryArena);
    AddWordToTrie(trieNode, "six", 3, '6', memoryArena);
    AddWordToTrie(trieNode, "seven", 5, '7', memoryArena);
    AddWordToTrie(trieNode, "eight", 5, '8', memoryArena);
    AddWordToTrie(trieNode, "nine", 4, '9', memoryArena);
    return trieNode;
}

int ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena) 
{
    int result = 0;

#define MAX_NUMBERS 64
    char numberString[MAX_NUMBERS];
    int currentStringIndex = 0;

    int wordIndex = 0;

    // running word stuff
    TrieNode* trieNode = InitTrie(memoryArena);
    int runningWordLength = 1;
    int runningWordOffset = 0;

    for (DWORD currentIndex = 0; currentIndex < fileSize; currentIndex++)
    {

        char currentChar = fileContents[currentIndex];

        if ( currentChar >= 48 && currentChar <= 57)
        {
            numberString[currentStringIndex] = currentChar;
            currentStringIndex++;

            // reset running word stuff
            runningWordOffset = currentIndex + 1;
            runningWordLength = 1;
        } else {
            LookUpResult lookUpResult = CheckWordInTrie(trieNode, 
                    &fileContents[runningWordOffset], runningWordLength);
            switch (lookUpResult.lookUpCode){
                case CHAR_NOT_IN_TRIE:
                    currentIndex -= (runningWordLength - 1);
                    runningWordOffset = currentIndex + 1;
                    runningWordLength = 1;
                    break;
                case INVALID_WORD:
                    runningWordOffset++;
                    runningWordLength = 1;
                    break;
                case CHAR_IN_TREE_BUT_NOT_WORD:
                    runningWordLength++;
                    break;
                case CHAR_IN_TREE_AND_WORD:
                    numberString[currentStringIndex] = lookUpResult.value;
                    currentStringIndex++;
                    runningWordOffset = currentIndex + 1;
                    runningWordLength = 1;
                    break;
                case SPECIAL_CASE:
                    numberString[currentStringIndex] = lookUpResult.value;
                    currentStringIndex++;
                    runningWordLength = 1;
                    runningWordOffset = currentIndex;
                    currentIndex--;
                    break;
                default:
                    OutputDebugStringA("well this should not happen");
            }
        }

        // note windows new lines are usually \r\n ... -.-
        if (fileContents[currentIndex] == '\r' || currentIndex == fileSize - 1)
        {
            int calValue = 0;
            calValue += CharToInt(numberString[0]) * 10 + CharToInt(numberString[currentStringIndex-1]);
            result += calValue;

            // skip \n as we are counting \r as the new line
            currentIndex++;
            currentStringIndex = 0;

            // reset running word stuff
            runningWordLength = 1;
            runningWordOffset = currentIndex + 1;
        }   
    }
    return result;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine, int nShowCmd)
{

    const char* fileName = "input.txt";
    DWORD fileSize = GetFileSize(fileName);

    MemoryArena memoryArena = {};
    memoryArena.totalSize = (sizeof(TrieNode) * MAX_CHARACTERS * MAX_TRIE_LEVELS) + fileSize;
    memoryArena.memory = VirtualAlloc(0, memoryArena.totalSize, 
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    FileResult fileResult = ReadTextFile(fileName, fileSize, &memoryArena);
    int answer = ParseFileContents((char *)fileResult.result, fileResult.fileSize, &memoryArena);

    Print("Answer is %d\n", answer);

    FreeAllMemory(&memoryArena);
    return 0;
}
