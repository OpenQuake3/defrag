#include "../local.h"


//:::::::::::::::::::
// Memory Allocation
//:::::::::::::::::::
#define POOLSIZE 128 * 1024
//:::::::::::::::::::
static char memoryPool[POOLSIZE];
static int  allocPoint;
static bool outOfMemory;
//:::::::::::::::::::
void* uiAlloc(int size) {
  if (allocPoint + size > POOLSIZE) {
    outOfMemory = true;
    return NULL;
  }
  char* p = &memoryPool[allocPoint];
  allocPoint += (size + 31) & ~31;
  return p;
}
//:::::::::::::::::::
void uiInitMemory(void) {
  allocPoint  = 0;
  outOfMemory = false;
}
//:::::::::::::::::::



//:::::::::::::::::::
void uiGameinfoInit(void) {
  uiInitMemory();
  // uiLoadArenas();
  // uiLoadBots();
  uis.demoversion = false;
}
