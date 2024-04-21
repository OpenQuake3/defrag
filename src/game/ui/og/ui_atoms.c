//::::::::::::::::::::::::::
// uiATOMS.C
//   User interface building blocks and support functions.
//::::::::::::::::::::::::::
#include "ui_local.h"

// uis
// m_entersound

// Com_Error
// Com_Printf

/*
=================
uiClampCvar
=================
*/
float uiClampCvar(float min, float max, float value) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

void uiStartDemoLoop(void) { id3Cmd_ExecuteText(EXEC_APPEND, "d1\n"); }

// uiPushMenu
// uiPopMenu

// uiForceMenuOff
// uiLerpColor

// propMap
// propMapB

// uiDrawBannerString2
// uiDrawBannerString

// uiProportionalStringWidth
// uiDrawProportionalString2
// uiProportionalSizeScale
// uiDrawProportionalString
// uiDrawProportionalString_AutoWrapped
// uiDrawString2
// uiDrawString
// uiDrawChar
// uiIsFullscreen

static void NeedCDAction(bool result) {
  if (!result) { id3Cmd_ExecuteText(EXEC_APPEND, "quit\n"); }
}

static void NeedCDKeyAction(bool result) {
  if (!result) { id3Cmd_ExecuteText(EXEC_APPEND, "quit\n"); }
}

// uiSetActiveMenu

// uiKeyEvent
// uiMouseEvent


char* uiArgv(int arg) {
  static char buffer[MAX_STRING_CHARS];
  id3Argv(arg, buffer, sizeof(buffer));
  return buffer;
}

char* uiCvar_VariableString(const char* var_name) {
  static char buffer[MAX_STRING_CHARS];
  id3Cvar_VariableStringBuffer(var_name, buffer, sizeof(buffer));
  return buffer;
}

void uiCache_f(void) {
  MainMenu_Cache();
  InGame_Cache();
  ConfirmMenu_Cache();
  PlayerModel_Cache();
  PlayerSettings_Cache();
  Controls_Cache();
  Demos_Cache();
  uiCinematicsMenu_Cache();
  Preferences_Cache();
  ServerInfo_Cache();
  SpecifyServer_Cache();
  ArenaServers_Cache();
  StartServer_Cache();
  ServerOptions_Cache();
  DriverInfo_Cache();
  GraphicsOptions_Cache();
  uiDisplayOptionsMenu_Cache();
  uiSoundOptionsMenu_Cache();
  uiNetworkOptionsMenu_Cache();
  uiSPLevelMenu_Cache();
  uiSPSkillMenu_Cache();
  uiSPPostgameMenu_Cache();
  TeamMain_Cache();
  uiAddBots_Cache();
  uiRemoveBots_Cache();
  uiSetupMenu_Cache();
  //	uiLoadConfig_Cache();
  //	uiSaveConfigMenu_Cache();
  uiBotSelectMenu_Cache();
  uiCDKeyMenu_Cache();
  uiModsMenu_Cache();
}

bool uiConsoleCommand(int realTime) {
  uis.frametime = realTime - uis.realtime;
  uis.realtime  = realTime;
  char* cmd     = uiArgv(0);
  // ensure minimum menu data is available
  Menu_Cache();

  if (Q_stricmp(cmd, "levelselect") == 0) {
    uiSPLevelMenu_f();
    return true;
  }
  if (Q_stricmp(cmd, "postgame") == 0) {
    uiSPPostgameMenu_f();
    return true;
  }
  if (Q_stricmp(cmd, "ui_cache") == 0) {
    uiCache_f();
    return true;
  }
  if (Q_stricmp(cmd, "ui_cinematics") == 0) {
    uiCinematicsMenu_f();
    return true;
  }
  if (Q_stricmp(cmd, "ui_teamOrders") == 0) {
    uiTeamOrdersMenu_f();
    return true;
  }
  if (Q_stricmp(cmd, "iamacheater") == 0) {
    uiSPUnlock_f();
    return true;
  }
  if (Q_stricmp(cmd, "iamamonkey") == 0) {
    uiSPUnlockMedals_f();
    return true;
  }
  if (Q_stricmp(cmd, "ui_cdkey") == 0) {
    uiCDKeyMenu_f();
    return true;
  }
  return false;
}

// uiShutdown
// uiInit
// uiAdjustFrom640

// uiDrawNamedPic
// uiDrawHandlePic
// uiFillRect

//:::::::::::::::::::
// uiDrawRect
//   Coordinates are 640*480 virtual values
//:::::::::::::::::::
void uiDrawRect(float x, float y, float width, float height, const float* color) {
  id3R_SetColor(color);

  uiAdjustFrom640(&x, &y, &width, &height);

  id3R_DrawStretchPic(x, y, width, 1, 0, 0, 0, 0, uis.whiteShader);
  id3R_DrawStretchPic(x, y, 1, height, 0, 0, 0, 0, uis.whiteShader);
  id3R_DrawStretchPic(x, y + height - 1, width, 1, 0, 0, 0, 0, uis.whiteShader);
  id3R_DrawStretchPic(x + width - 1, y, 1, height, 0, 0, 0, 0, uis.whiteShader);

  id3R_SetColor(NULL);
}

// uiSetColor
// uiUpdateScreen
// uiRefresh

void uiDrawTextBox(int x, int y, int width, int lines) {
  uiFillRect(x + BIGCHAR_WIDTH / 2, y + BIGCHAR_HEIGHT / 2, (width + 1) * BIGCHAR_WIDTH, (lines + 1) * BIGCHAR_HEIGHT, colorBlack);
  uiDrawRect(x + BIGCHAR_WIDTH / 2, y + BIGCHAR_HEIGHT / 2, (width + 1) * BIGCHAR_WIDTH, (lines + 1) * BIGCHAR_HEIGHT, colorWhite);
}

// uiCursorInRect
