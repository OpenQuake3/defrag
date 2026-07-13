
#include "../local.h"
//:::::::::::::::::::::::::
// vmMain
//   This is the only way control passes into the module.
//   This must be the very first function compiled into the .qvm file
//:::::::::::::::::::::::::
#include <stdio.h>  //TODO: Remove
void tmp(char*);
void tmp(char* msg) {
  printf("%s", msg);
  return;
}
#define FN __func__
//:::::::::::::::::::::::::

Q_EXPORT intptr_t vmMain(int command, int UU arg0, int UU arg1, int UU arg2, int UU arg3, int UU arg4, int UU arg5, int UU arg6, int UU arg7, int UU arg8,
                         int UU arg9, int UU arg10, int UU arg11) {
  switch (command) {  // clang-format off
    case UI_GETAPIVERSION:        return UI_API_VERSION;
    case UI_INIT:                 uiInit(); return 0;
    case UI_SHUTDOWN:             uiShutdown(); return 0;
    case UI_IS_FULLSCREEN:        return uiIsFullscreen();
    case UI_SET_ACTIVE_MENU:      uiSetActiveMenu(arg0); return 0;
    case UI_REFRESH:              uiRefresh(arg0); return 0;
    case UI_KEY_EVENT:            uiEvent_key(arg0, arg1); return 0;
    case UI_MOUSE_EVENT:          uiEvent_mouse(arg0, arg1); return 0;
    case UI_CONSOLE_COMMAND:      return uiConsoleCommand(arg0);
    case UI_HASUNIQUECDKEY:       return qfalse;  // change this to qfalse for mods!

    case UI_DRAW_CONNECT_SCREEN: uiDrawConnectScreen(arg0); return 0;
    default: tmp(va("UI.%s: asked for unhandled command %i\n", FN, command)); return 0;
  }  // clang-format on
  return -1;
}
