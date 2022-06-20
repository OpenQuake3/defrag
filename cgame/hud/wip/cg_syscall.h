#ifndef CG_SYSCALL_H
#define CG_SYSCALL_H

#include "ExportImport.h"

#include <stdint.h>

EXPORTIMPORT void dllEntry(intptr_t(QDECL* syscallptr)(intptr_t arg, ...));

intptr_t QDECL CG_SysCalls(uint8_t* memoryBase, int32_t cmd, int32_t* args);

#endif // CG_SYSCALL_H
