#include "../common/win32_utils.cpp"

struct Sequence {
    Array<int> sequence;
};

struct Prediction {
    int predictionAtEnd;
    int predictionAtStart;
};

Prediction ParseHistory(Array<int>* history)
{
    MemoryArena memoryArena = {};
    memoryArena.totalSize = Kilobytes(100);
    memoryArena.memory = VirtualAlloc(0, memoryArena.totalSize, 
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    Array<Sequence*> sequences = {};
    InitArrayData(&sequences, &memoryArena);

    Sequence* historySequence = (Sequence*)AllocateMemory(&memoryArena, sizeof(Sequence));
    historySequence->sequence = *history;

    PushArray(&sequences, historySequence, &memoryArena);

    Array<int>* currentSequence = history;

    Sequence* sequence;
    while (!ArrayAllAreEqualToValue(currentSequence, 0))
    {
        sequence = (Sequence*)AllocateMemory(&memoryArena, sizeof(Sequence));
        InitArrayData(&sequence->sequence, &memoryArena);

        for (int historyIndex = 0; historyIndex < currentSequence->currentCapacity - 1;
                historyIndex++)
        {
            int diff = currentSequence->data[historyIndex + 1] - currentSequence->data[historyIndex];
            PushArray(&sequence->sequence, diff, &memoryArena);
        }

        PushArray(&sequences, sequence, &memoryArena);
        currentSequence = &sequence->sequence;
    }

    // reverse lookup to build end prediction value
    int predictedValueEnd = 0;
    for (int i = sequences.currentCapacity - 1; i >= 0; i--)
    {
        Sequence* current = sequences[i];
        predictedValueEnd += current->sequence.data[current->sequence.currentCapacity-1];

    }

    // reverse lookup to build start prediction value
    int predictedValueStart = 0;
    for (int i = sequences.currentCapacity - 1; i >= 0; i--)
    {
        Sequence* current = sequences[i];
        predictedValueStart  = current->sequence.data[0] - predictedValueStart;

    }

    FreeAllMemory(&memoryArena);

    Prediction prediction = {};
    prediction.predictionAtEnd = predictedValueEnd;
    prediction.predictionAtStart = predictedValueStart;
    return prediction;
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena) 
{
    // temp hack to avoid sequential memory usage in our string
    // give some padding to our memory
    AllocateMemory(memoryArena, 1);

    int sumOfEndPredictions = 0;
    int sumOfStartPredictions = 0;
    char* s = fileContents;

    Array<int> history = {};
    InitArrayData(&history, memoryArena);
    while (*s != '\0')
    {
        int historyCount = 0;
        while (*s != '\r' && *s != '\0')
        {
            EatResult eatResult = {};
            eatResult = EatNumber(s, memoryArena);

            int historyValue = StringToInt(eatResult.result);
            if (historyCount >= history.currentCapacity)
            {
                PushArray(&history, historyValue, memoryArena);
            }
            else 
            {
                history.data[historyCount] = historyValue;
            }

            historyCount++;
            EatSpaces(s);
        }
        Prediction prediction = ParseHistory(&history);
        sumOfEndPredictions += prediction.predictionAtEnd;
        sumOfStartPredictions += prediction.predictionAtStart;
        if (*s != '\0')
        {
            s += 2;
        }
    }

    Print("Sum of predicted values at the end is %d", sumOfEndPredictions);
    Print("Sum of predicted values at the start is %d", sumOfStartPredictions);
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
