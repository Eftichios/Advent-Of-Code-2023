#include "../common/win32_utils.cpp"

enum HandType {
    FiveOfAKind,
    FourOfAKind,
    FullHouse,
    ThreeOfAKind,
    TwoPair,
    OnePair,
    HighCard,

    MAX_TYPES
};

struct Hand {
    int bid;
    char* cards;
    HandType handType;
};

struct HashNode {
    int key;
    Array<Hand> handArray = {};

    HashNode* nextNode;
};

#define LOWEST_CARD_POWER 1
#define CARDS 13
#define HAND_SIZE 5

int CardToPower(char card)
{
    if  (card >= 48 && card <= 57)
    {
        return card - 48 - LOWEST_CARD_POWER;
    }
    else
    {
        if (card == 'J') return 0;
        if (card == 'T') return 9;
        if (card == 'Q') return 10;
        if (card == 'K') return 11;
        if (card == 'A') return 12;
    }
    return 0;
}

HandType DetermineHandType(char* s)
{
    int cardSeen[CARDS] = {};

    int maxCount = 0;
    int maxIndex = 0;

    int jokerPower = CardToPower('J');
    for (int cardIndex = 0; cardIndex < HAND_SIZE; cardIndex++)
    {
        char current = s[cardIndex];

        int cardPower = CardToPower(current);
        if (cardSeen[cardPower] == 0)
        {
            cardSeen[cardPower] = 1;
        }
        else
        {
            int newValue = ++cardSeen[cardPower];
            if (newValue > maxCount && cardPower != jokerPower)
            {
                maxCount = newValue;
                maxIndex = cardPower;
            }
        }
    }

    int secondMaxCount = 0;
    int secondMaxIndex = 0;
    for (int cardSeenIndex = 0; 
            cardSeenIndex < sizeof(cardSeen) / sizeof(cardSeen[0]);
            cardSeenIndex++)
    {
        if (cardSeenIndex == maxIndex) continue;
        int value = cardSeen[cardSeenIndex];
        if (value > secondMaxCount && cardSeenIndex != jokerPower)
        {
            secondMaxCount = value;
            secondMaxIndex = cardSeenIndex;
        }
    }

    int jokerCount = cardSeen[CardToPower('J')];

    if (maxCount == 5) return FiveOfAKind;
    if (maxCount == 4) 
    {
        if (jokerCount == 4) return FiveOfAKind;
        if (jokerCount == 1) return FiveOfAKind;
        
        return FourOfAKind;
    }

    if (maxCount == 3) {
        if (jokerCount == 3) return FourOfAKind;
        if (jokerCount == 2) return FiveOfAKind;
        if (jokerCount == 1) return FourOfAKind;
        if (secondMaxCount == 2) return FullHouse;
        return ThreeOfAKind;
    }

    if (maxCount == 2) {
        if (jokerCount == 3) return FiveOfAKind;
        if (jokerCount == 2) return FourOfAKind;
        if (secondMaxCount == 2) {
            if (jokerCount == 1) return FullHouse;
            return TwoPair;
        }
        if (jokerCount == 1) return ThreeOfAKind;
        return OnePair;
    }

    if (jokerCount == 5) return FiveOfAKind;
    if (jokerCount == 4) return FiveOfAKind;
    if (jokerCount == 3) return FourOfAKind;
    if (jokerCount == 2) return ThreeOfAKind;
    if (jokerCount == 1) return OnePair;
    return HighCard;
}

bool CompareCards(char* a, char *b)
{
    // if a > b return true otherwise false
    for (int cardIndex = 0; cardIndex < HAND_SIZE; cardIndex++)
    {
        char aPower = CardToPower(a[cardIndex]);
        char bPower = CardToPower(b[cardIndex]);

        if (aPower > bPower) return true;
        if (bPower > aPower) return false;
    }

    // they are equal but this should not happen...
    return true;
}

void PlaceHandInArray(Array<Hand>* handArray, Hand hand, MemoryArena* memoryArena)
{
    for (int handIndex = 0; handIndex < handArray->currentCapacity; handIndex++)
    {
        Hand currentHand = handArray->data[handIndex];
        bool isOurHandBiggerThanCurrent = CompareCards(hand.cards, currentHand.cards);
        if (isOurHandBiggerThanCurrent)
        {
            handArray->data[handIndex] = hand;

            // shift all other hands 1 place to the right
            Hand nextHand = currentHand; 
            while (handIndex < handArray->currentCapacity - 1)
            {
                Hand tempHand = handArray->data[handIndex + 1];
                handArray->data[handIndex + 1] = nextHand;
                handIndex += 1;

                nextHand = tempHand;

            }
            PushArray(handArray, nextHand, memoryArena);
            return;
        }
    }
    // it was smaller than all so put at the end
    PushArray(handArray, hand, memoryArena);
}

void ParseAndSortHands(char*& s, HashMap<HashNode>* typesToHandsMap, MemoryArena* memoryArena)
{
    int handsCount = 0;
    EatResult eatResult = {};
    while (*s != '\0')
    {
        eatResult = EatString(s, memoryArena);
        char* cards = eatResult.result;

        HandType handType = DetermineHandType(cards);

        eatResult = EatSpaces(s);

        eatResult = EatNumber(s, memoryArena);
        int bid = StringToInt(eatResult.result);

        Hand hand = {};
        hand.cards = cards;
        hand.bid = bid;
        hand.handType = handType;

        HashNode* hashNode = RetrieveFromHashMap(typesToHandsMap, handType);
        if (hashNode)
        {
            // keep the array sorted!
            PlaceHandInArray(&hashNode->handArray, hand, memoryArena);
        }
        else
        {
            Array<Hand> handArray = {};
            InitArrayData(&handArray, memoryArena);

            PushArray(&handArray, hand, memoryArena);

            HashNode typeToHands = {};
            typeToHands.handArray = handArray;
            typeToHands.key = handType;

            AddToHashMap(typesToHandsMap, handType, typeToHands, memoryArena);
        }

        if (*s !='\0')
        {
            // advance \r\n
            s+=2;
        }
        handsCount += 1;
    }

    int result = 0;
    int rank = handsCount;
    for (int handType = 0; handType < MAX_TYPES; handType++)
    {
        HashNode* typeNode = RetrieveFromHashMap(typesToHandsMap, handType);
        if (!typeNode) continue;
        for (int handIndex = 0; handIndex < typeNode->handArray.currentCapacity;
                handIndex++)
        {
            Hand hand = typeNode->handArray.data[handIndex];
            result += rank * hand.bid;
            rank--;
        }
    }

    Print("Total winnings with the Joker are %d", result);
}

void ParseFileContents(char* fileContents, DWORD fileSize, MemoryArena* memoryArena) 
{
    // temp hack to avoid sequential memory usage in our string
    // give some padding to our memory
    AllocateMemory(memoryArena, 1);

    HashMap<HashNode> typeToHandsMap = {};
    char* s = fileContents;

    ParseAndSortHands(s, &typeToHandsMap, memoryArena);

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
