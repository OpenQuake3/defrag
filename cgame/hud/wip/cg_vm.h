/*
  ==============================
  Written by:
    id software :            Quake III Arena
    nightmare, hk, Jelvan1 : mdd cgame Proxymod
    sOkam! :                 Opensource Defrag

  This file is part of Opensource Defrag.

  Opensource Defrag is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Opensource Defrag is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Opensource Defrag.  If not, see <http://www.gnu.org/licenses/>.
  ==============================

  Note: This file contains code from 
    Kevin Masterson a.k.a. CyberMind <kevinm@planetquake.com>
    of the QMM - Q3 MultiMod
*/
#ifndef CG_VM_H
#define CG_VM_H

#include "q_shared.h"

// magic number is stored in file as 44 14 72 12
#define VM_MAGIC     0x12721444
#define VM_MAGIC_BIG 0x44147212

#define MAX_QPATH 64

typedef enum
{
  OP_UNDEF,
  OP_NOP,
  OP_BREAK,
  OP_ENTER,
  OP_LEAVE,
  OP_CALL,
  OP_PUSH,
  OP_POP,
  OP_CONST,
  OP_LOCAL,
  OP_JUMP,
  OP_EQ,
  OP_NE,
  OP_LTI,
  OP_LEI,
  OP_GTI,
  OP_GEI,
  OP_LTU,
  OP_LEU,
  OP_GTU,
  OP_GEU,
  OP_EQF,
  OP_NEF,
  OP_LTF,
  OP_LEF,
  OP_GTF,
  OP_GEF,
  OP_LOAD1,
  OP_LOAD2,
  OP_LOAD4,
  OP_STORE1,
  OP_STORE2,
  OP_STORE4,
  OP_ARG,
  OP_BLOCK_COPY,
  OP_SEX8,
  OP_SEX16,
  OP_NEGI,
  OP_ADD,
  OP_SUB,
  OP_DIVI,
  OP_DIVU,
  OP_MODI,
  OP_MODU,
  OP_MULI,
  OP_MULU,
  OP_BAND,
  OP_BOR,
  OP_BXOR,
  OP_BCOM,
  OP_LSH,
  OP_RSHI,
  OP_RSHU,
  OP_NEGF,
  OP_ADDF,
  OP_SUBF,
  OP_DIVF,
  OP_MULF,
  OP_CVIF,
  OP_CVFI
} vmOps_t;

typedef struct
{
  int32_t vmMagic;
  int32_t instructionCount;
  int32_t codeOffset;
  int32_t codeLength;
  int32_t dataOffset;
  int32_t dataLength;
  int32_t litLength; /* ( dataLength - litLength ) should be byteswapped on load */
  int32_t bssLength; /* zero filled memory appended to datalength */
} vmHeader_t;

typedef struct vm_s
{
  /* public interface */
  char name[MAX_QPATH];

  /* segments */
  int32_t* codeSegment;  /* code segment, each instruction is 2 ints */
  byte*    dataSegment;  /* data segment, partially filled on load */
  byte*    stackSegment; /* stack segment */

  /* status*/
  int32_t codeSegmentLen; /* size of codeSegment */
  int32_t dataSegmentLen; /* size of dataSegment */
  int32_t dataSegmentMask;

  /* registers */
  int32_t* opPointer;
  int32_t* opStack;
  int32_t  opBase;

  /* memory */
  int32_t memorySize;
  byte*   memory;

  qboolean   swapped;  /* was this file byteswapped? (is the server big-endian) */
  int32_t    fileSize; /* .qvm file size (for qmmvm_status) */
  vmHeader_t header;   /* store header information (useful later on) */

  /* non-API function hooking */
  int32_t hook_realfunc; /* address for a VM function to call after a hook completes (0 = don't call) */
} vm_t;

extern vm_t    g_VM;
extern char    vmpath[MAX_QPATH];
extern char    vmbase[16];
extern int32_t vm_stacksize;

intptr_t QDECL VM_Exec(
  vm_t*   vm,
  int32_t command,
  int32_t arg0,
  int32_t arg1,
  int32_t arg2,
  int32_t arg3,
  int32_t arg4,
  int32_t arg5,
  int32_t arg6,
  int32_t arg7,
  int32_t arg8,
  int32_t arg9,
  int32_t arg10,
  int32_t arg11);
qboolean VM_Create(vm_t* vm, char const* path, byte* oldmem);
void     VM_Destroy(vm_t* vm);
qboolean VM_Restart(vm_t* vm, qboolean savemem);
void*    VM_ArgPtr(int32_t intValue);
void*    VM_ExplicitArgPtr(vm_t const* vm, int32_t intValue);
int32_t  int_byteswap(int32_t i);
short    short_byteswap(short s);

#endif // CG_VM_H
