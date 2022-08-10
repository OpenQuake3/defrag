#ifndef UI_CALLBACKS_H
#define UI_CALLBACKS_H
//:::::::::::::::::

#include "types.h"

//:::::::::::::::::
// Engine Callbacks
// ui/callbacks.c
void        id3Print(const char*);
void        id3Error(const char*) NR;
int         id3Milliseconds(void);
void        id3CvarRegister(vmCvar_t*, const char*, const char*, int);
void        id3Cvar_Set(const char*, const char*);
void        id3Cvar_Update(vmCvar_t*);
void        id3Cvar_VariableStringBuffer(const char*, char*, int);
void        id3Cmd_ExecuteText(int, const char*);
void        id3GetClientState(uiClientState_t*);
void        id3GetClipboardData(char* buf, int bufsize);
void        id3GetGlconfig(glconfig_t*);
qhandle_t   id3R_RegisterModel(const char*);
qhandle_t   id3R_RegisterShaderNoMip(const char*);
void        id3R_ClearScene(void);
void        id3R_RenderScene(const refdef_t*);
void        id3R_SetColor(const float*);
void        id3R_DrawStretchPic(float, float, float, float, float, float, float, float, qhandle_t);
void        id3UpdateScreen(void);
void        id3S_StartLocalSound(sfxHandle_t, int);
sfxHandle_t id3S_RegisterSound(const char*, bool);
int         id3Key_GetCatcher(void);
void        id3Key_SetCatcher(int);
void        id3Key_ClearStates(void);
bool        id3Key_GetOverstrikeMode(void);
void        id3Key_SetOverstrikeMode(bool);
bool        id3Key_IsDown(int);
//:::::::::::::::::

//:::::::::::::::::
#endif  // UI_CALLBACKS_H
