#include "../common/win32_utils.cpp"


struct HashNode {
    int key;
    ulong32 destRangeStart;
    ulong32 sourceRangeStart;

    ulong32 rangeLength;

    HashNode* nextNode;
};

struct Seed {
    ulong32 seedId;
    ulong32 seedRange;
};

void PopulateSeeds(char*& s, Array<Seed>* seeds, Array<ulong32>* seedIds, MemoryArena* memoryArena)
{
    EatResult eatResult = {};
    eatResult = EatWord(s, memoryArena);
    eatResult = EatDelimeter(s, ':');

    while (*s != '\r')
    {
        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        ulong32 seedId = StringToUlong32(eatResult.result);

        PushArray(seedIds, seedId, memoryArena);

        eatResult = EatSpaces(s);
        eatResult = EatNumber(s, memoryArena);
        ulong32 seedRange = StringToUlong32(eatResult.result);

        PushArray(seedIds, seedRange, memoryArena);

        Seed seed = {};
        seed.seedId = seedId;
        seed.seedRange = seedRange;

        PushArray(seeds, seed, memoryArena);
    }

    // advance \r\n
    s += 2;
}

void PopulateMap(char*& s, HashMap<HashNode>* hashMap, MemoryArena* memoryArena)
{
    bool parsed = false;
    EatResult eatResult = {};
    eatResult = EatEntireLine(s);

    while (!parsed)
    {
        while (*s != '\r' && *s != '\0')
        {
            eatResult = EatNumber(s, memoryArena);
            ulong32 destRangeStart = StringToInt(eatResult.result);
            eatResult = EatSpaces(s);

            eatResult = EatNumber(s, memoryArena);
            ulong32 sourceRangeStart = StringToInt(eatResult.result);
            eatResult = EatSpaces(s);

            eatResult = EatNumber(s, memoryArena);
            ulong32 rangeLength = StringToInt(eatResult.result);
            eatResult = EatSpaces(s);


            HashNode newHashNodeValue = {};
            int key = (destRangeStart - sourceRangeStart) / 1000;
            if (key < 0)
            {
                key *= -1;
            }
            newHashNodeValue.key = key;
            newHashNodeValue.destRangeStart = destRangeStart;
            newHashNodeValue.sourceRangeStart = sourceRangeStart;
            newHashNodeValue.rangeLength = rangeLength;
            AddToHashMap(hashMap, key, newHashNodeValue, memoryArena);

            if (*s != '\0')
            {
                s += 2;
            }
        }
        if (*s == '\r' || *s == '\0')
        {
            parsed = true;
        }
    }

    // advance \r\n
    if (*s == '\r')
    {
        s += 2;
    }
}

ulong32 MapSourceToDest(HashMap<HashNode>* hashMap, ulong32 source)
{
    // @SPEED: This is very inefficient to do but it will do for now
    for (ulong32 hashIndex = 0; hashIndex < HASH_SIZE; hashIndex++)
    {
        HashNode* current = hashMap->hashNodes[hashIndex];
        while (current) {
            if (source >= current->sourceRangeStart 
                    && source < current->sourceRangeStart + current->rangeLength)
            {
                return source + (current->destRangeStart - current->sourceRangeStart);
            }
            current = current->nextNode;
        }
    }
    return source;
}

ulong32 MapDestToSource(HashMap<HashNode>* hashMap, ulong32 dest)
{
    // @SPEED: This is very inefficient to do but it will do for now
    for (ulong32 hashIndex = 0; hashIndex < HASH_SIZE; hashIndex++)
    {
        HashNode* current = hashMap->hashNodes[hashIndex];
        while (current) {
            if (dest >= current->destRangeStart 
                    && dest < current->destRangeStart + current->rangeLength)
            {
                return dest + (current->sourceRangeStart - current->destRangeStart);
            }
            current = current->nextNode;
        }
    }
    return dest;
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena) 
{

    // temp hack to avoid sequential memory usage in our string
    // give some padding to our memory
    AllocateMemory(memoryArena, 1);

    Array<Seed> seeds = {};
    InitArrayData(&seeds, memoryArena);

    Array<ulong32> seedIds = {};
    InitArrayData(&seedIds, memoryArena);


    HashMap<HashNode> seedToSoil = {};
    HashMap<HashNode> soilToFertilizer = {};
    HashMap<HashNode> fertilizerToWater = {};
    HashMap<HashNode> waterToLight = {};
    HashMap<HashNode> lightToTemperature = {};
    HashMap<HashNode> temperatureToHumidity = {};
    HashMap<HashNode> humidityToLocation = {};

    char *s = fileContents;

    PopulateSeeds(s, &seeds, &seedIds, memoryArena);
    // advance new line \r\n
    s += 2;
    PopulateMap(s, &seedToSoil, memoryArena);
    s += 2;
    PopulateMap(s, &soilToFertilizer, memoryArena);
    s += 2;
    PopulateMap(s, &fertilizerToWater, memoryArena);
    s += 2;
    PopulateMap(s, &waterToLight, memoryArena);
    s += 2;
    PopulateMap(s, &lightToTemperature, memoryArena);
    s += 2;
    PopulateMap(s, &temperatureToHumidity, memoryArena);
    s += 2;
    PopulateMap(s, &humidityToLocation, memoryArena);

    // part 1
    ulong32 minLocation = 0xFFFFFFFF;
    ulong32 minSeed = 0;
    for (int index = 0; index < seedIds.currentCapacity; index++)
    {
        ulong32 seed = seedIds[index];
        ulong32 soil = MapSourceToDest(&seedToSoil, seed);
        ulong32 fert = MapSourceToDest(&soilToFertilizer, soil);
        ulong32 water = MapSourceToDest(&fertilizerToWater, fert);
        ulong32 light = MapSourceToDest(&waterToLight, water);
        ulong32 temp = MapSourceToDest(&lightToTemperature, light);
        ulong32 humid = MapSourceToDest(&temperatureToHumidity, temp);
        ulong32 location = MapSourceToDest(&humidityToLocation, humid);

        if (location < minLocation)
        {
            minLocation = location;
            minSeed = seed;
        }

    }

    Print("The minimum location is %ul\n", minLocation);
    Print("The seed for the min location is %ul\n", minSeed);


    // part 2 solution
    // do a reverse lookup to see if any of our seeds of the lowest location
    // fall within the current seeds
    bool found = false;
    ulong32 minSeedPart2 = 0;
    ulong32 currentLocation = 0;
    while (!found)
    {
        ulong32 humid = MapDestToSource(&humidityToLocation, currentLocation);
        ulong32 temp = MapDestToSource(&temperatureToHumidity, humid);
        ulong32 light = MapDestToSource(&lightToTemperature, temp);
        ulong32 water = MapDestToSource(&waterToLight, light);
        ulong32 fert = MapDestToSource(&fertilizerToWater, water);
        ulong32 soil = MapDestToSource(&soilToFertilizer, fert);
        ulong32 seedId = MapDestToSource(&seedToSoil, soil);

        for (int index = 0; index < seeds.currentCapacity; index++)
        {
            Seed seed = seeds[index];
            if (seedId >= seed.seedId && seedId <= seed.seedId + seed.seedRange)
            {
                found = true;
                minSeedPart2 = seedId;
                break;
            }
        }

        if (!found)
        {
            if (currentLocation % 100000 == 0)
            {
                Print("Currently considering: %ul\n", currentLocation);
            }
            currentLocation++;
        }
    }

    Print("Part two seed for min location is %ul\n", minSeedPart2);
    Print("Part two min location is %ul\n", currentLocation);
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
