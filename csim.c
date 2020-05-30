//#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char flag = 0;  // marks whether to enable -v option
int setBits = 0;
int associativity = 0;
int blockBits = 0;
char *traceName = NULL;

int setSize;
int blockSize;

int hitTime = 0;
;
int missTime = 0;
int evictTime = 0;

#define VALID(x) (*(char *)((char *)(x) + 4))
#define DIRTY(x) (*(char *)((char *)(x) + 5))
#define USEDTIMES(x) (*(short *)((char *)(x) + 6))
#define TAGNUMBER(x) (*(int *)(x))
#define BYTE(x, y) (*(char *)(x + 8 + i))
// block structure: tagNumber [0:3] validbit[4] dirtybit [5] usedtimes[6:7]
// data[8:8+blocksize-1]

void printHelpInfo();
int getBlockNumber(unsigned long address, int blockBits);
int getSetNumber(unsigned long address, int setBits, int blockBits);
int getTagNumber(unsigned long address, int setBits, int blockBits);
void store(char v, char *cache, unsigned long address, int oprtsize);
void load(char v, char *cache, unsigned long address, int oprtsize);
void modify(char v, char *cache, unsigned long address, int oprtsize);

int main(int argc, char **argv) {
  const char *optString = "vs:E:b:t:";

  if (argc == 2) {
    if (argv[1][1] == 'h')
      printHelpInfo();

    else
      printf("Wrong!\n");
    return 0;
  }

  char opt = getopt(argc, argv, optString);
  while (opt != -1) {
    switch (opt) {
      case 'v':
        flag = 1;
        break;
      case 's':
        setBits = atoi(optarg);
        break;
      case 'E':
        associativity = atoi(optarg);
        break;
      case 'b':
        blockBits = atoi(optarg);
        break;
      case 't':
        traceName = optarg;
        break;
    }
    opt = getopt(argc, argv, optString);
  }

  if (!setBits || !associativity || !blockBits || !traceName) {
    printf("Lack of arguments.\n");
    return 0;
  }

  FILE *trace = fopen(traceName, "r");
  if (!trace) {
    printf("Can not open file \"%s\".\n", traceName);
    return 0;
  }
  int cachesize;
  char *cache = (char *)malloc(
      cachesize = ((1 << setBits)) * (associativity) *
                  (blockSize = (1 << blockBits) + 4 + sizeof(int)));
  setSize = blockSize * associativity;
  // printf("setSize:%d,blockSize:%d\n",setSize,blockSize);
  memset(cache, 0, cachesize);
  char order, comma;
  unsigned long address;
  int oprtsize;
  while (!feof(trace)) {
    fscanf(trace, "%c%lx%c%d", &order, &address, &comma, &oprtsize);
    switch (order) {
      case 'I':
        break;
      case 'S':
        // printf("S %lx%c%d\n",address,comma,oprtsize);
        store(flag, cache, address, oprtsize);
        break;
      case 'L':
        // printf("L %lx%c%d\n", address, comma, oprtsize);
        load(flag, cache, address, oprtsize);
        break;
      case 'M':
        // printf("M %lx%c%d\n", address, comma, oprtsize);
        modify(flag, cache, address, oprtsize);
        break;
    }

    char *tagptr;
    for (tagptr = cache; tagptr < cache + cachesize; tagptr += blockSize)
      USEDTIMES(tagptr) -= 1;
  }
  printf("hits:%d misses:%d evictions:%d\n", hitTime, missTime, evictTime);
  fclose(trace);
  free(cache);
  return 0;
}

void printHelpInfo() {
  printf("./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
  printf("\t-v\tOptional verbose flag that displays trace info\n");
  printf("\t-s <s>\tNumber of set index bits\n");
  printf("\t-E <E>\tAssociativity\n");
  printf("\t-b <b>\tNumber of block bits\n");
  printf("\t-t <tracefile>\tName of valgrind trace to display\n");
}

int getBlockNumber(unsigned long address, int blockBits) {
  int mask = 1;
  int i;
  for (i = 1; i <= blockBits - 1; i++) {
    mask <<= 1;
    mask++;
  }
  return address & mask;
}

int getSetNumber(unsigned long address, int setBits, int blockBits) {
  int mask = 1 << blockBits;
  int i;
  for (i = 1; i <= setBits - 1; i++) {
    mask <<= 1;
    mask += (1 << blockBits);
  }
  return (address & mask) >> blockBits;
}

int getTagNumber(unsigned long address, int setBits, int blockBits) {
  int mask = 1;
  int i;
  for (i = 1; i <= setBits + blockBits - 1; i++) {
    mask <<= 1;
    mask += 1;
  }
  mask = ~mask;
  return (address & mask) >> (setBits + blockBits);
}

void store(char v, char *cache, unsigned long address, int oprtsize) {
  int setNumber = getSetNumber(address, setBits, blockBits);
  int tagNumber = getTagNumber(address, setBits, blockBits);
  char *setbaseptr = cache + setSize * setNumber;
  char *tagptr = setbaseptr;
  char flag = 0;
  int i;

  for (i = 0; i < associativity; i++) {
    if (TAGNUMBER(tagptr) == tagNumber && VALID(tagptr)) {
      flag = 1;
      break;
    }
    tagptr += blockSize;
  }

  if (flag) {
    hitTime++;
    USEDTIMES(tagptr) += 1;
    if (v) printf("S %lx,%d hit\n", address, oprtsize);
    return;
  } else {
    missTime++;
    int leastused = 0;
    short leastusedTimes = 32767;
    for (tagptr = setbaseptr, i = 0; i < associativity; i++) {
      if (!VALID(tagptr)) {
        VALID(tagptr) = 1;
        USEDTIMES(tagptr) = 1;
        flag = 1;
        if (v) printf("S %lx,%d miss\n", address, oprtsize);
        return;
      } else {
        if (USEDTIMES(tagptr) <= leastusedTimes) leastused = i;
      }
      tagptr += blockSize;
    }
    if (!flag) {
      evictTime++;
      if (v) printf("S %lx,%d miss eviction\n", address, oprtsize);
      tagptr = setbaseptr + leastused * blockSize;
      TAGNUMBER(tagptr) = getTagNumber(address, setBits, blockBits);
      VALID(tagptr) = 1;
      USEDTIMES(tagptr) = 32767;
    }
  }
}

void load(char v, char *cache, unsigned long address, int oprtsize) {
  int setNumber = getSetNumber(address, setBits, blockBits);
  int tagNumber = getTagNumber(address, setBits, blockBits);
  char *setbaseptr = cache + setSize * setNumber;
  char *tagptr = setbaseptr;
  char flag = 0;
  int i;
  for (i = 0; i < associativity; i++) {
    if (TAGNUMBER(tagptr) == tagNumber && VALID(tagptr)) {
      flag = 1;
      break;
    }
    tagptr += blockSize;
  }
  if (flag) {
    hitTime++;
    USEDTIMES(tagptr) += 1;
    if (v) printf("L %lx,%d hit\n", address, oprtsize);
    return;
  } else {
    missTime++;
    int leastused = 0;
    short leastusedTimes = 32767;
    for (tagptr = setbaseptr, i = 0; i < associativity; i++) {
      if (!VALID(tagptr)) {
        flag = 1;
        VALID(tagptr) = 1;
        TAGNUMBER(tagptr) = tagNumber;
        USEDTIMES(tagptr) = 1;
        if (v) printf("L %lx,%d miss\n", address, oprtsize);
        return;
      } else {
        if (USEDTIMES(tagptr) <= leastusedTimes) leastused = i;
      }
      tagptr += blockSize;
    }
    if (!flag) {
      evictTime++;
      if (v) printf("L %lx,%d miss eviction\n", address, oprtsize);
      tagptr = setbaseptr + leastused * blockSize;

      TAGNUMBER(tagptr) = tagNumber;
      VALID(tagptr) = 1;
      USEDTIMES(tagptr) = 32767;
    }
  }
}

void modify(char v, char *cache, unsigned long address, int oprtsize) {
  int setNumber = getSetNumber(address, setBits, blockBits);
  int tagNumber = getTagNumber(address, setBits, blockBits);
  char *setbaseptr = cache + setSize * setNumber;

  char *tagptr = setbaseptr;
  char flag = 0;
  int i;
  for (i = 0; i < associativity; i++) {
    if (TAGNUMBER(tagptr) == tagNumber && VALID(tagptr)) {
      flag = 1;
      break;
    }
    tagptr += blockSize;
  }

  if (flag) {
    hitTime++;
    USEDTIMES(tagptr) += 1;
    if (v) printf("M %lx,%d hit\n", address, oprtsize);
    return;
  } else {
    missTime++;
    hitTime++;
    int leastused = 0;
    short leastusedTimes = 32767;
    for (tagptr = setbaseptr, i = 0; i < associativity; i++) {
      if (!VALID(tagptr)) {
        flag = 1;
        VALID(tagptr) = 1;
        if (v) printf("M %lx,%d miss hit\n", address, oprtsize);
        return;
      } else {
        if (USEDTIMES(tagptr) <= leastusedTimes) leastused = i;
      }
      tagptr += blockSize;
    }
    if (!flag) {
      evictTime++;
      if (v) printf("M %lx,%d miss eviction hit\n", address, oprtsize);
      tagptr = setbaseptr + leastused * blockSize;

      TAGNUMBER(tagptr) = tagNumber;
      VALID(tagptr) = 1;
      USEDTIMES(tagptr) = 32767;
    }
  }
}