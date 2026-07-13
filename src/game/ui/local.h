#ifndef UI_LOCAL_H
#define UI_LOCAL_H
//:::::::::::::::::

//:::::::::::::::::
// Engine dependencies
#include "../qcommon/q_shared.h"
#include "../rendc/tr_types.h"
#include "../client/keycodes.h"
// UI local dependencies
#include "shared.h"  // UI data, shared with the engine (used to be ui_public)
#include "color/schemes.h"
#include "framework/elements.h"
//:::::::::::::::::

//:::::::::::::::::
// Engine Interface
//:::::::::::::::::
// dllCallback_t
//   Pointer to a function that returns an intptr and takes arg with varargs  (aka callNum)
//   Abstracts away the confusion from repeating this prototype syntax everywhere
typedef intptr_t(QDECL* dllCallback_t)(intptr_t, ...);
// Entry functions
Q_EXPORT intptr_t vmMain(int, int, int, int, int, int, int, int, int, int, int, int, int);
Q_EXPORT void     dllEntry(dllCallback_t);
//:::::::::::::::::

//:::::::::::::::::
// ui/core.c   Data Structures
#include "types.h"
//:::::::::::::::::::
extern uiStatic_t uis;
extern Q3sound    q3sound;
extern Sounds     uiSound;
extern Songs      song;
extern bool       m_entersound;
extern bool       m_enterSong;
//:::::::::::::::::

//::::::::::::::::
// Text tools
//.........................................
// TODO: All 9 positions
// typedef enum { TEXT_ALIGN_LEFT = 1, TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT } TextAlignment;
//.........................................
// #define TEXT_ALIGN_DEFAULT TEXT_ALIGN_LEFT  // Sets the type that will be used when called with align 0
#define FONT_SCALE_DEFAULT 1
//..................
#define FONT_FILE_DEFAULT "ui/font/default.ttf"
#define FONT_SIZE_DEFAULT 16
//..................
#define FONT_FILE_ACTION "ui/font/default.ttf"
#define FONT_SIZE_ACTION 48
#define FONT_FILE_ACTIONKEY "ui/font/heyNovember.ttf"
#define FONT_SIZE_ACTIONKEY 96 // 128 was really cool for main logo
//:::::::::::::::::


//:::::::::::::::::
// ui/core.c      Methods
void uiInit(void);              // Requested right after asking for apiVersion
void uiShutdown(void);          // Requested when engine shutsdown the UI
bool uiIsFullscreen(void);      // Engine requests to check for fullscreen state
void uiSetActiveMenu(MenuCmd);  // Engine connection with ui/menu.c
void uiRefresh(int);            // Called on every ui frame
void uiDrawConnectScreen(bool); // Called by the engine to draw the loading/connect background
// Input Events
void uiEvent_key(int, int);    // Requested when there is a key input event
void uiEvent_mouse(int, int);  // Requested when there is a mouse event
// Commands
char* uiArgv(int);
bool  uiConsoleCommand(int);
//:::::::::::::::::
#include "framework/tools.h"
//:::::::::::::::::
// ui/cvars.c     Methods
void uiCvarsRegisterAll(void);
void uiCvarsUpdateAll(void);
//:::::::::::::::::
// ui/gameinfo.c  Methods
void uiGameinfoInit(void);
//:::::::::::::::::
// ui/menu.c      Methods
void  menuCache(void);         // Initialize Menu assets Cache
void  menuForceOff(void);      // Force disable all menus
void* cursorGetItem(MenuFw*);  // Get menu item under cursor
// Menu page loaders
void menuStart_init(void);    // Start menu setup  (start = name, not verb)
void menuCredits(void);       // Credits menu

//:::::::::::::::::
// Engine Callbacks
// ui/callbacks.c
void        id3Print(const char*);
void        id3Error(const char*) NR;
int         id3Milliseconds(void);
void        id3CvarRegister(vmCvar_t*, const char*, const char*, int);
void        id3Cvar_Set(const char*, const char*);
void        id3Cvar_VariableStringBuffer(const char*, char*, int);
void        id3Cmd_ExecuteText(int, const char*);
void        id3GetClientState(uiClientState_t*);
void        id3GetGlconfig(glconfig_t*);
qhandle_t   id3R_RegisterModel(const char*);
qhandle_t   id3R_RegisterShaderNoMip(const char*);
void        id3R_ClearScene(void);
void        id3R_RenderScene(const refdef_t*);
void        id3R_SetColor(const float*);
void        id3R_DrawStretchPic(float, float, float, float, float, float, float, float, qhandle_t);
void        id3S_StartLocalSound(sfxHandle_t, int);
sfxHandle_t id3S_RegisterSound(const char*, bool);
int         id3Key_GetCatcher(void);
void        id3Key_SetCatcher(int);
void        id3Key_ClearStates(void);
bool        id3Key_GetOverstrikeMode(void);
void        id3Key_SetOverstrikeMode(bool);
//:::::::::::::::::

//:::::::::::::::::
#endif
