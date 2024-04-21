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
*/
#include "cg_syscall.h"

#include "bbox.h"
#include "cg_entity.h"
#include "cg_gl.h"
#include "cg_local.h"
#include "cg_rl.h"

static intptr_t(QDECL* syscall)(intptr_t, ...) = (intptr_t(QDECL*)(intptr_t, ...)) - 1;

void dllEntry(intptr_t(QDECL* syscallptr)(intptr_t arg, ...))
{
  syscall = syscallptr;
}

static inline int32_t FloatAsInt(float f)
{
  int32_t i;
  memcpy(&i, &f, sizeof(int32_t));
  return i;
}

void trap_Print(char const* fmt)
{
  syscall(CG_PRINT, fmt);
}

void trap_Error(char const* fmt)
{
  syscall(CG_ERROR, fmt);
}

int32_t trap_Milliseconds(void)
{
  return (int32_t)syscall(CG_MILLISECONDS);
}

void trap_Cvar_Register(vmCvar_t* vmCvar, char const* varName, char const* defaultValue, int32_t flags)
{
  syscall(CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags);
}

void trap_Cvar_Update(vmCvar_t* vmCvar)
{
  syscall(CG_CVAR_UPDATE, vmCvar);
}

void trap_Cvar_Set(char const* var_name, char const* value)
{
  syscall(CG_CVAR_SET, var_name, value);
}

void trap_Cvar_VariableStringBuffer(char const* var_name, char* buffer, int32_t bufsize)
{
  syscall(CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize);
}

int32_t trap_Argc(void)
{
  return (int32_t)syscall(CG_ARGC);
}

void trap_Argv(int32_t n, char* buffer, int32_t bufferLength)
{
  syscall(CG_ARGV, n, buffer, bufferLength);
}

void trap_Args(char* buffer, int32_t bufferLength)
{
  syscall(CG_ARGS, buffer, bufferLength);
}

int32_t trap_FS_FOpenFile(char const* qpath, fileHandle_t* f, fsMode_t mode)
{
  return (int32_t)syscall(CG_FS_FOPENFILE, qpath, f, mode);
}

void trap_FS_Read(void* buffer, int32_t len, fileHandle_t f)
{
  syscall(CG_FS_READ, buffer, len, f);
}

void trap_FS_Write(void const* buffer, int32_t len, fileHandle_t f)
{
  syscall(CG_FS_WRITE, buffer, len, f);
}

void trap_FS_FCloseFile(fileHandle_t f)
{
  syscall(CG_FS_FCLOSEFILE, f);
}

int32_t trap_FS_Seek(fileHandle_t f, long offset, int32_t origin)
{
  return (int32_t)syscall(CG_FS_SEEK, f, offset, origin);
}

void trap_AddCommand(char const* cmdName)
{
  syscall(CG_ADDCOMMAND, cmdName);
}

void trap_RemoveCommand(char const* cmdName)
{
  syscall(CG_REMOVECOMMAND, cmdName);
}

int32_t trap_CM_NumInlineModels(void)
{
  return (int32_t)syscall(CG_CM_NUMINLINEMODELS);
}

int32_t trap_CM_PointContents(vec3_t const p, clipHandle_t model)
{
  return (int32_t)syscall(CG_CM_POINTCONTENTS, p, model);
}

void trap_CM_BoxTrace(
  trace_t*     results,
  vec3_t const start,
  vec3_t const end,
  vec3_t const mins,
  vec3_t const maxs,
  clipHandle_t model,
  int32_t      brushmask)
{
  syscall(CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask);
  // CG_Trace
  results->entityNum = results->fraction != 1.f ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
}

int32_t trap_CM_MarkFragments(
  int32_t         numPoints,
  vec3_t const*   points,
  vec3_t const    projection,
  int32_t         maxPoints,
  vec3_t          pointBuffer,
  int32_t         maxFragments,
  markFragment_t* fragmentBuffer)
{
  return (int32_t)syscall(
    CG_CM_MARKFRAGMENTS, numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer);
}

qhandle_t trap_R_RegisterModel(char const* name)
{
  return (qhandle_t)syscall(CG_R_REGISTERMODEL, name);
}

qhandle_t trap_R_RegisterSkin(char const* name)
{
  return (qhandle_t)syscall(CG_R_REGISTERSKIN, name);
}

qhandle_t trap_R_RegisterShader(char const* name)
{
  return (qhandle_t)syscall(CG_R_REGISTERSHADER, name);
}

qhandle_t trap_R_RegisterShaderNoMip(char const* name)
{
  return (qhandle_t)syscall(CG_R_REGISTERSHADERNOMIP, name);
}

void trap_R_ClearScene(void)
{
  syscall(CG_R_CLEARSCENE);
}

void trap_R_AddRefEntityToScene(refEntity_t const* re)
{
  syscall(CG_R_ADDREFENTITYTOSCENE, re);
}

void trap_R_AddPolyToScene(qhandle_t hShader, int32_t numVerts, polyVert_t const* verts)
{
  syscall(CG_R_ADDPOLYTOSCENE, hShader, numVerts, verts);
}

void trap_R_AddPolysToScene(qhandle_t hShader, int32_t numVerts, polyVert_t const* verts, int32_t num)
{
  syscall(CG_R_ADDPOLYSTOSCENE, hShader, numVerts, verts, num);
}

int32_t trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir)
{
  return (int32_t)syscall(CG_R_LIGHTFORPOINT, point, ambientLight, directedLight, lightDir);
}

void trap_R_AddLightToScene(vec3_t const org, float intensity, float r, float g, float b)
{
  syscall(CG_R_ADDLIGHTTOSCENE, org, FloatAsInt(intensity), FloatAsInt(r), FloatAsInt(g), FloatAsInt(b));
}

void trap_R_AddAdditiveLightToScene(vec3_t const org, float intensity, float r, float g, float b)
{
  syscall(CG_R_ADDADDITIVELIGHTTOSCENE, org, FloatAsInt(intensity), FloatAsInt(r), FloatAsInt(g), FloatAsInt(b));
}

void trap_R_RenderScene(refdef_t const* fd)
{
  syscall(CG_R_RENDERSCENE, fd);
}

void trap_R_SetColor(float const* rgba)
{
  syscall(CG_R_SETCOLOR, rgba);
}

void trap_R_DrawStretchPic(
  float     x,
  float     y,
  float     w,
  float     h,
  float     s1,
  float     t1,
  float     s2,
  float     t2,
  qhandle_t hShader)
{
  syscall(
    CG_R_DRAWSTRETCHPIC,
    FloatAsInt(x),
    FloatAsInt(y),
    FloatAsInt(w),
    FloatAsInt(h),
    FloatAsInt(s1),
    FloatAsInt(t1),
    FloatAsInt(s2),
    FloatAsInt(t2),
    hShader);
}

void trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs)
{
  syscall(CG_R_MODELBOUNDS, model, mins, maxs);
}

void trap_GetGlconfig(glconfig_t* glconfig)
{
  syscall(CG_GETGLCONFIG, glconfig);
}

void trap_GetGameState(gameState_t* gamestate)
{
  syscall(CG_GETGAMESTATE, gamestate);
}

void trap_GetCurrentSnapshotNumber(int32_t* snapshotNumber, int32_t* serverTime)
{
  syscall(CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime);
}

qboolean trap_GetSnapshot(int32_t snapshotNumber, snapshot_t* snapshot)
{
  return (qboolean)syscall(CG_GETSNAPSHOT, snapshotNumber, snapshot);
}

int32_t trap_GetCurrentCmdNumber(void)
{
  return (int32_t)syscall(CG_GETCURRENTCMDNUMBER);
}

qboolean trap_GetUserCmd(int32_t cmdNumber, usercmd_t* ucmd)
{
  return (qboolean)syscall(CG_GETUSERCMD, cmdNumber, ucmd);
}

qboolean trap_GetEntityToken(char* buffer, int32_t bufferSize)
{
  return (qboolean)syscall(CG_GET_ENTITY_TOKEN, buffer, bufferSize);
}

//=================================================

// handling syscalls from QVM (passing them to engine)
// this adds the base VM address to a given value
#define add(x) ((x) ? (void*)((x) + (intptr_t)memoryBase) : NULL)
// this subtracts the base VM address from a given value
#define sub(x) ((x) ? (void*)((x) - (intptr_t)memoryBase) : NULL)
// this gets an argument value
#define arg(x) (args[x])
// this adds the base VM address to an argument value
#define ptr(x) (add(arg(x)))

#define _ptr(x) (add(x)) // ???

intptr_t QDECL CG_SysCalls(uint8_t* memoryBase, int32_t cmd, int32_t* args)
{
  switch (cmd)
  {
  case CG_PRINT: // void trap_Printf( char const *fmt )
    syscall(cmd, ptr(0));
    return 0;

  case CG_ERROR: // void trap_Error( char const *fmt )
    syscall(cmd, ptr(0));
    return 0;

  case CG_MILLISECONDS: // int32_t trap_Milliseconds( void )
    return syscall(cmd);

  case CG_ARGC: // int32_t trap_Argc( void )
    return syscall(cmd);

  case CG_ARGV: // void trap_Argv( int32_t n, char *buffer, int32_t bufferLength )
    syscall(cmd, arg(0), ptr(1), arg(2));
    return 0;

  case CG_ARGS:
    syscall(cmd, ptr(0), arg(1));
    return 0;

  case CG_FS_FOPENFILE: // int32_t   trap_FS_FOpenFile( char const *qpath, fileHandle_t *f, fsMode_t mode )
    return syscall(cmd, ptr(0), ptr(1), arg(2));

  case CG_FS_READ: // void  trap_FS_Read( void *buffer, int32_t len, fileHandle_t f )
    syscall(cmd, ptr(0), arg(1), arg(2));
    return 0;

  case CG_FS_WRITE: // void  trap_FS_Write( void const *buffer, int32_t len, fileHandle_t f )
    syscall(cmd, ptr(0), arg(1), arg(2));
    return 0;

  case CG_FS_FCLOSEFILE: // void  trap_FS_FCloseFile( fileHandle_t f )
    syscall(cmd, arg(0));
    return 0;

  case CG_FS_SEEK: // int32_t trap_FS_Seek( fileHandle_t f, long offset, int32_t origin )
    return syscall(cmd, arg(0), arg(1), arg(2));

  case CG_SENDCONSOLECOMMAND: // void  trap_SendConsoleCommand( int32_t exec_when, char const *text )
    syscall(cmd, ptr(0));
    return 0;

  case CG_CVAR_REGISTER: // void  trap_Cvar_Register( vmCvar_t *cvar, char const *var_name, char const *value, int32_t
                         // flags )
    syscall(cmd, ptr(0), ptr(1), ptr(2), arg(3));
    return 0;

  case CG_CVAR_UPDATE: // void  trap_Cvar_Update( vmCvar_t *cvar )
    syscall(cmd, ptr(0));
    return 0;

  case CG_CVAR_SET: // void trap_Cvar_Set( char const *var_name, char const *value )
    syscall(cmd, ptr(0), ptr(1));
    return 0;

  case CG_CVAR_VARIABLESTRINGBUFFER: // void trap_Cvar_VariableStringBuffer( char const *var_name, char *buffer, int32_t
                                     // bufsize )
    syscall(cmd, ptr(0), ptr(1), arg(2));
    return 0;

  case CG_ADDCOMMAND: // void CL_AddCgameCommand( char const *cmdName )
    syscall(cmd, ptr(0));
    return 0;

  case CG_REMOVECOMMAND:
    syscall(cmd, ptr(0));
    return 0;
  case CG_SENDCLIENTCOMMAND:
    syscall(cmd, ptr(0));
    return 0;
  case CG_UPDATESCREEN:
    syscall(cmd);
    return 0;
  case CG_CM_LOADMAP:
    syscall(cmd, ptr(0));
    return 0;
  case CG_CM_NUMINLINEMODELS:
    return syscall(cmd);
  case CG_CM_INLINEMODEL:
    return syscall(cmd, arg(0));
  case CG_CM_TEMPBOXMODEL:
    return syscall(cmd, ptr(0), ptr(1));
  case CG_CM_TEMPCAPSULEMODEL:
    return syscall(cmd, ptr(0), ptr(1));
  case CG_CM_POINTCONTENTS:
    return syscall(cmd, ptr(0), arg(1));
  case CG_CM_TRANSFORMEDPOINTCONTENTS:
    return syscall(cmd, ptr(0), arg(1), ptr(2), ptr(3));
  case CG_CM_BOXTRACE:
    syscall(cmd, ptr(0), ptr(1), ptr(2), ptr(3), ptr(4), arg(5), arg(6));
    return 0;
  case CG_CM_CAPSULETRACE:
    syscall(cmd, ptr(0), ptr(1), ptr(2), ptr(3), ptr(4), arg(5), arg(6));
    return 0;
  case CG_CM_TRANSFORMEDBOXTRACE:
    syscall(cmd, ptr(0), ptr(1), ptr(2), ptr(3), ptr(4), arg(5), arg(6), ptr(7), ptr(8));
    return 0;
  case CG_CM_TRANSFORMEDCAPSULETRACE:
    syscall(cmd, ptr(0), ptr(1), ptr(2), ptr(3), ptr(4), arg(5), arg(6), ptr(7), ptr(8));
    return 0;
  case CG_CM_MARKFRAGMENTS:
    return syscall(cmd, arg(0), ptr(1), ptr(2), arg(3), ptr(4), arg(5), ptr(6));
  case CG_S_STARTSOUND:
    if (should_filter_sound(arg(1), 0)) return 0;
    syscall(cmd, ptr(0), arg(1), arg(2), arg(3));
    return 0;
  case CG_S_STARTLOCALSOUND:
    syscall(cmd, arg(0), arg(1));
    return 0;
  case CG_S_CLEARLOOPINGSOUNDS:
    syscall(cmd, arg(0));
    return 0;
  case CG_S_ADDLOOPINGSOUND:
    if (should_filter_sound(arg(0), 1)) return 0;
    syscall(cmd, arg(0), ptr(1), ptr(2), arg(3));
    return 0;
  case CG_S_ADDREALLOOPINGSOUND:
    syscall(cmd, arg(0), ptr(1), ptr(2), arg(3));
    return 0;
  case CG_S_STOPLOOPINGSOUND:
    syscall(cmd, arg(0));
    return 0;
  case CG_S_UPDATEENTITYPOSITION:
    syscall(cmd, arg(0), ptr(1));
    return 0;
  case CG_S_RESPATIALIZE:
    syscall(cmd, arg(0), ptr(1), ptr(2), arg(3));
    return 0;
  case CG_S_REGISTERSOUND:
    return syscall(cmd, ptr(0), arg(1));
  case CG_S_STARTBACKGROUNDTRACK:
    syscall(cmd, ptr(0), ptr(1));
    return 0;
  case CG_R_LOADWORLDMAP:
    syscall(cmd, ptr(0));
    return 0;
  case CG_R_REGISTERMODEL:
    return syscall(cmd, ptr(0));
  case CG_R_REGISTERSKIN:
    return syscall(cmd, ptr(0));
  case CG_R_REGISTERSHADER:
    return syscall(cmd, ptr(0));

  case CG_R_REGISTERSHADERNOMIP:
    return syscall(cmd, ptr(0));
  case CG_R_REGISTERFONT:
    syscall(cmd, ptr(0), arg(1), ptr(2));
    return 0;
  case CG_R_CLEARSCENE:
    syscall(cmd);
    return 0;
  case CG_R_ADDREFENTITYTOSCENE:
    syscall(cmd, ptr(0));
    return 0;
  case CG_R_ADDPOLYTOSCENE:
    syscall(cmd, arg(0), arg(1), ptr(2));
    return 0;
  case CG_R_ADDPOLYSTOSCENE:
    syscall(cmd, arg(0), arg(1), ptr(2), arg(3));
    return 0;
  case CG_R_LIGHTFORPOINT:
    return syscall(cmd, ptr(0), ptr(1), ptr(2), ptr(3));
  case CG_R_ADDLIGHTTOSCENE:
    syscall(cmd, ptr(0), arg(1), arg(2), arg(3), arg(4));
    return 0;
  case CG_R_ADDADDITIVELIGHTTOSCENE:
    syscall(cmd, ptr(0), arg(1), arg(2), arg(3), arg(4));
    return 0;
  case CG_R_RENDERSCENE:
    draw_gl();
    draw_rl();
    draw_bbox();

    syscall(cmd, ptr(0));
    return 0;
  case CG_R_SETCOLOR:
    syscall(cmd, ptr(0));
    return 0;
  case CG_R_DRAWSTRETCHPIC:
    syscall(cmd, arg(0), arg(1), arg(2), arg(3), arg(4), arg(5), arg(6), arg(7), arg(8));
    return 0;
  case CG_R_MODELBOUNDS:
    syscall(cmd, arg(0), ptr(1), ptr(2));
    return 0;
  case CG_R_LERPTAG:
    return syscall(cmd, ptr(0), arg(1), arg(2), arg(3), arg(4), ptr(5));
  case CG_GETGLCONFIG:
    syscall(cmd, ptr(0));
    return 0;
  case CG_GETGAMESTATE: // void CL_GetGameState( gameState_t *gs )
    syscall(cmd, ptr(0));
    return 0;

  case CG_GETCURRENTSNAPSHOTNUMBER: // void  CL_GetCurrentSnapshotNumber( int32_t *snapshotNumber, int32_t *serverTime )
    syscall(cmd, ptr(0), ptr(1));
    return 0;

  case CG_GETSNAPSHOT: // qboolean  CL_GetSnapshot( int32_t snapshotNumber, snapshot_t *snapshot )
    return syscall(cmd, arg(0), ptr(1));

  case CG_GETSERVERCOMMAND: // qboolean CL_GetServerCommand( int32_t serverCommandNumber )
    return syscall(cmd, arg(0));

  case CG_GETCURRENTCMDNUMBER:
    return syscall(cmd);

  case CG_GETUSERCMD: // qboolean CL_GetUserCmd( int32_t cmdNumber, usercmd_t *ucmd )
    return syscall(cmd, arg(0), ptr(1));

  case CG_SETUSERCMDVALUE: // void CL_SetUserCmdValue( int32_t userCmdValue, float sensitivityScale )
    syscall(cmd, arg(0), arg(1));
    return 0;

  case CG_MEMORY_REMAINING:
    return syscall(cmd);
  case CG_KEY_ISDOWN:
    return syscall(cmd, arg(0));
  case CG_KEY_GETCATCHER:
    return syscall(cmd);
  case CG_KEY_SETCATCHER:
    syscall(cmd, arg(0));
    return 0;
  case CG_KEY_GETKEY:
    return syscall(cmd, ptr(0));
  case CG_MEMSET:
    syscall(cmd, ptr(0), arg(1), arg(2));
    return 0;
  case CG_MEMCPY:
    syscall(cmd, ptr(0), ptr(1), arg(2));
    return 0;
  case CG_STRNCPY:
    return syscall(cmd, ptr(0), ptr(1), arg(2));
  case CG_SIN:
    return syscall(cmd, arg(0));
  case CG_COS:
    return syscall(cmd, arg(0));
  case CG_ATAN2:
    return syscall(cmd, arg(0), arg(1));
  case CG_SQRT:
    return syscall(cmd, arg(0));
  case CG_FLOOR:
    return syscall(cmd, arg(0));
  case CG_CEIL:
    return syscall(cmd, arg(0));
  case CG_ACOS:
    return syscall(cmd, arg(0));

  case CG_PC_ADD_GLOBAL_DEFINE:
    return syscall(cmd, ptr(0));
  case CG_PC_LOAD_SOURCE:
    return syscall(cmd, ptr(0));
  case CG_PC_FREE_SOURCE:
    return syscall(cmd, arg(0));
  case CG_PC_READ_TOKEN:
    return syscall(cmd, arg(0), ptr(1));
  case CG_PC_SOURCE_FILE_AND_LINE:
    return syscall(cmd, arg(0), ptr(1), ptr(2));

  case CG_S_STOPBACKGROUNDTRACK:
    syscall(cmd);
    return 0;

  case CG_REAL_TIME:
    return syscall(cmd, ptr(0));
  case CG_SNAPVECTOR:
    syscall(cmd, ptr(0));
    return 0;

  case CG_CIN_PLAYCINEMATIC:
    return syscall(cmd, ptr(0), arg(1), arg(2), arg(3), arg(4), arg(5));

  case CG_CIN_STOPCINEMATIC:
    return syscall(cmd, arg(0));

  case CG_CIN_RUNCINEMATIC:
    return syscall(cmd, arg(0));

  case CG_CIN_DRAWCINEMATIC:
    syscall(cmd, arg(0));
    return 0;

  case CG_CIN_SETEXTENTS:
    syscall(cmd, arg(0), arg(1), arg(2), arg(3), arg(4));
    return 0;

  case CG_R_REMAP_SHADER:
    syscall(cmd, ptr(0), ptr(1), ptr(2));
    return 0;

    /*
      case CG_LOADCAMERA:
        return loadCamera(ptr(0));

      case CG_STARTCAMERA:
        startCamera(args[1]);
        return 0;

      case CG_GETCAMERAINFO:
        return getCameraInfo(args[1), ptr(1), ptr(2));
    */
  case CG_GET_ENTITY_TOKEN:
    return syscall(cmd, ptr(0), arg(1));
  case CG_R_INPVS:
    return syscall(cmd, ptr(0), ptr(1));

  default:
    return 0;
  }
}
