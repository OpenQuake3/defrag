
#include "../local.h"

// this file is only included when building a dll
// syscalls.asm is included instead when building a qvm
#ifdef Q3_VM
#  error "Do not use in VM build"
#endif

//::OSDF change
//:::::::::::::::::
//  Removed function pointer syntax mindfuck
//  Renamed `syscall` to the more modern day `callback`
//:::::::::::::::::
// callback
//   Will store a function pointer of shape dllCallback_t
//   Dummy initializer value. -1 is just a placeholder that will be erased
//   Typecasts -1 into the correct dll pointer prototype.
//   This means the pointer starting value is 0xFFFF(etc) instead of 0x0000(etc) (aka NULL)
static dllCallback_t callback = (dllCallback_t)-1;
//:::::::::::::::::
// dllEntry
//   Function that always exports as visible (even with -fvisibility=hidden)
//   Takes a function pointer (remote), and assigns it to a (local) function pointer
//   Will be called inside the engine (from its dlsym handle),
//    and will assign to the library local variable (static function pointer)
Q_EXPORT void dllEntry(dllCallback_t callbackptr) { callback = callbackptr; }
//:::::::::::::::::
//::OSDF end

//:::::::::::::::::
// General utility
void id3Print(const char* string) { callback(UI_PRINT, string); }
void id3Error(const char* string) { callback(UI_ERROR, string); }
int  id3Milliseconds(void) { return callback(UI_MILLISECONDS); }
void id3GetClipboardData(char* buf, int bufsize) { callback(UI_GETCLIPBOARDDATA, buf, bufsize); }
void id3GetClientState(uiClientState_t* state) { callback(UI_GETCLIENTSTATE, state); }
void id3Cmd_ExecuteText(int exec_when, const char* text) { callback(UI_CMD_EXECUTETEXT, exec_when, text); }
//:::::::::::::::::
// Cvars
void id3CvarRegister(vmCvar_t* cvar, const char* var_name, const char* value, int flags) { callback(UI_CVAR_REGISTER, cvar, var_name, value, flags); }
void id3Cvar_Set(const char* var_name, const char* value) { callback(UI_CVAR_SET, var_name, value); }
void id3Cvar_Update(vmCvar_t* cvar) { callback(UI_CVAR_UPDATE, cvar); }
void id3Cvar_VariableStringBuffer(const char* var_name, char* buffer, int bufsize) {
  callback(UI_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize);
}
//:::::::::::::::::
// Renderer
void      id3GetGlconfig(glconfig_t* glconfig) { callback(UI_GETGLCONFIG, glconfig); }
qhandle_t id3R_RegisterModel(const char* name) { return callback(UI_R_REGISTERMODEL, name); }
qhandle_t id3R_RegisterShaderNoMip(const char* name) { return callback(UI_R_REGISTERSHADERNOMIP, name); }
void      id3R_ClearScene(void) { callback(UI_R_CLEARSCENE); }
void      id3R_RenderScene(const refdef_t* fd) { callback(UI_R_RENDERSCENE, fd); }
void      id3R_SetColor(const float* rgba) { callback(UI_R_SETCOLOR, rgba); }
void      id3R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader) {
       callback(UI_R_DRAWSTRETCHPIC, Passf(x), Passf(y), Passf(w), Passf(h), Passf(s1), Passf(t1), Passf(s2), Passf(t2), hShader);
}
void id3UpdateScreen(void) { callback(UI_UPDATESCREEN); }

//:::::::::::::::::
// Sound
void        id3S_StartLocalSound(sfxHandle_t sfx, int channelNum) { callback(UI_S_STARTLOCALSOUND, sfx, channelNum); }
sfxHandle_t id3S_RegisterSound(const char* sample, bool compressed) { return callback(UI_S_REGISTERSOUND, sample, compressed); }
//:::::::::::::::::
// Keys
int  id3Key_GetCatcher(void) { return callback(UI_KEY_GETCATCHER); }
void id3Key_SetCatcher(int catcher) { callback(UI_KEY_SETCATCHER, catcher); }
void id3Key_ClearStates(void) { callback(UI_KEY_CLEARSTATES); }
bool id3Key_GetOverstrikeMode(void) { return callback(UI_KEY_GETOVERSTRIKEMODE); }
void id3Key_SetOverstrikeMode(bool state) { callback(UI_KEY_SETOVERSTRIKEMODE, state); }
bool id3Key_IsDown(int keynum) { return callback(UI_KEY_ISDOWN, keynum); }

//:::::::::::::::::
