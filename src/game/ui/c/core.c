
#include "../local.h"

//:::::::::::::::::::::::
uiStatic_t uis;
//:::::::::::::::::::

//:::::::::::::::::::
// uiConsoleCommand
//:::::::::::::::::::
bool uiConsoleCommand(int realTime) {
  uis.frametime = realTime - uis.realtime;
  uis.realtime  = realTime;
  char* cmd     = uiArgv(0);
  // ensure minimum menu data is available
  // menuCache(); Com_Printf("%s: --> requesting menuCache()\n", __func__);  // sk.rmv -> TODO: Fix font re-registering and activate this after.

  // clang-format off
  // if (!Q_stricmp(cmd, "levelselect"))   { uiSPLevelMenu_f(); return true; }
  // if (!Q_stricmp(cmd, "postgame"))      { uiSPPostgameMenu_f(); return true; }
  // if (!Q_stricmp(cmd, "ui_cache"))      { uiCache_f(); return true; }
  // if (!Q_stricmp(cmd, "ui_cinematics")) { uiCinematicsMenu_f(); return true; }
  // if (!Q_stricmp(cmd, "ui_teamOrders")) { uiTeamOrdersMenu_f(); return true; }
  // if (!Q_stricmp(cmd, "iamacheater"))   { uiSPUnlock_f(); return true; }
  // if (!Q_stricmp(cmd, "iamamonkey"))    { uiSPUnlockMedals_f(); return true; }
  // if (!Q_stricmp(cmd, "ui_cdkey"))      { uiCDKeyMenu_f(); return true; }
  return false;  // clang-format on
}


//:::::::::::::::::::
// uiShutdown
//   Called on engine shutdown request
//:::::::::::::::::::
void uiShutdown(void) {}

//:::::::::::::::::::
#define HRES 640
#define VRES 480
// Bias: Difference in width at the margins, between a 4:3 screen and the current screen res (e.g. a 16:9 screen)
//       vidHeight * aspectRatio gives you what you would expect your vidWidth should be
//       Subtracting that from what vidWidth actually is, will result in
//        the difference between the expected 4:3 width and the real widescreen width.
//       Then it is divided by two to get the offset in only one direction.
void uiOldScaling_init(void) {
  bool widescreen = (uis.glconfig.vidWidth * VRES > uis.glconfig.vidHeight * HRES);
  if (widescreen) {
    // wide screen
    uis.bias   = 0.5 * (uis.glconfig.vidWidth - (uis.glconfig.vidHeight * ((float)HRES / VRES)));
    uis.xscale = uis.yscale;
  } else {
    // no wide screen
    uis.bias = 0;
  }
}
//:::::::::::::::::::
// uiInit
//   Called by the engine when initializing the UI
//:::::::::::::::::::
void uiInit(void) {
  uiCvarsRegisterAll();
  uiGameinfoInit();
  // cache redundant calulations
  id3GetGlconfig(&uis.glconfig);
  // for 640x480 virtualized screen
  uis.xscale = uis.glconfig.vidWidth * (1.0 / HRES);
  uis.yscale = uis.glconfig.vidHeight * (1.0 / VRES);
  uiOldScaling_init();  // This was the old process that applied a 4:3 bias for every non-conforming resolution
  // initialize the menu system
  menuCache();
  uis.activemenu = NULL;
  uis.menusp     = 0;
#ifndef NDEBUG
  uis.debug = true;
#endif
  Com_Printf(":: Finished initializing UI --------\n");
}

//:::::::::::::::::::
// uiIsFullscreen
//   Checks if there is an activemenu and a keycatcher active
//   If so, it returns the state of activemenu->fullscreen
//   else, it always returns false, even if the window is fullscreen
//:::::::::::::::::::
bool uiIsFullscreen(void) {
  if (uis.activemenu && (id3Key_GetCatcher() & KEYCATCH_UI)) { return uis.activemenu->fullscreen; }
  return false;
}

//:::::::::::::::::::
// uiSetActiveMenu
//   This should be the ONLY way the menu system is brought up
//   Ensure minimum menu data is cached
//:::::::::::::::::::
void uiSetActiveMenu(MenuCmd menu) {
  menuCache();
  Com_Printf("%s: Requested menu %d\n", __func__, menu);
  switch (menu) {
    case MENU_NONE: menuForceOff(); return;
    case MENU_START:
      menuStart_init();
      return;  // uiMainMenu(); return;
    // case MENU_NEEDKEY: uiConfirmMenu("Insert the CD", 0, NeedCDAction); return;
    // case MENU_BADKEY: uiConfirmMenu("Bad CD Key", 0, NeedCDKeyAction); return;
    // case MENU_INGAME:
    //   id3Cvar_Set("cl_paused", "1");
    //   uiInGameMenu();
    //   return;
    // // case UIMENU_GRANK: uiRankingsMenu(); return;  // GRank
    // case MENU_TEAM:
    // case MENU_POSTGAME:
    default: Com_Printf("%s: Requested menu %d, unrecognized\n", __func__, menu); break;
  }
}

//:::::::::::::::::::
// UI_KeyEvent
//:::::::::::::::::::
void uiEvent_key(int key, int down) {
  if (!uis.activemenu) { return; }
  if (!down) { return; }

  sfxHandle_t s;
  if (uis.activemenu->key) {
    s = uis.activemenu->key(key);
  } else {
    s = menuDefaultKey(uis.activemenu, key);
  }
  if ((s > 0) && (s != q3sound.menu_null)) { id3S_StartLocalSound(s, CHAN_LOCAL_SOUND); }
}

//:::::::::::::::::::
// UI_MouseEvent
//:::::::::::::::::::
void uiEvent_mouse(int dx, int dy) {
  if (!uis.activemenu) { return; }

  // update mouse screen position
  uis.cursorx += dx;
  uis.cursory += dy;
  // Clamp to screen size
  if (uis.cursorx < 0) uis.cursorx = 0;
  else if (uis.cursorx > GL_W) uis.cursorx = GL_W;
  if (uis.cursory < 0) uis.cursory = 0;
  else if (uis.cursory > GL_H) uis.cursory = GL_H;

  // region test the active menu items
  for (int i = 0; i < uis.activemenu->nitems; i++) {
    MenuCommon* m = (MenuCommon*)uis.activemenu->items[i];
    if (m->flags & (MFL_GRAYED | MFL_INACTIVE)) { continue; }
    // Item bounds  (convert from percentage to pixels)
    int left  = m->left * GL_W;
    int right = m->right * GL_W;
    int top   = m->top * GL_H;
    int bot   = m->bottom * GL_H;
    if ((uis.cursorx < left) || (uis.cursorx > right) || (uis.cursory < top) || (uis.cursory > bot)) { continue; }  // cursor out of item bounds

    // set focus to item at cursor
    if (uis.activemenu->cursor != i) {
      cursorSet(uis.activemenu, i);
      ((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~MFL_HASMOUSEFOCUS;
      if (!(((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor]))->flags & MFL_SILENT)) { id3S_StartLocalSound(uiSound.move, CHAN_LOCAL_SOUND); }
    }
    ((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor]))->flags |= MFL_HASMOUSEFOCUS;
    return;
  }
  if (uis.activemenu->nitems > 0) {  // out of any region
    ((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor]))->flags &= ~MFL_HASMOUSEFOCUS;
  }
}

//:::::::::::::::::::
// uiArgv
//   Gets the string value of the console command argument at index `arg`
//:::::::::::::::::::
char* uiArgv(int arg) {
  static char buffer[MAX_STRING_CHARS];
  id3Argv(arg, buffer, sizeof(buffer));
  return buffer;
}

static void uiSongPlayRandom(void) {
  int lower = 0;
  int upper = 1;
  int r = (rand() % (upper - lower + 1)) + lower;
  switch (r) {
    case 0: id3S_StartLocalSound(song.chronos, CHAN_LOCAL_SOUND); break;
    // case 1: id3S_StartLocalSound(song.succubus, CHAN_LOCAL_SOUND); break;
    default: id3S_StartLocalSound(uiSound.silence, CHAN_LOCAL_SOUND); break;
  }
}

//:::::::::::::::::::
// uiDrawConnectScreen
//:::::::::::::::::::
void uiDrawConnectScreen(bool overlay) {
  if (!overlay) {
    uiSetColor(NULL);
    uiDrawHandlePicPix(0, 0, GL_W, GL_H, uis.bgMain);
  }
}

//:::::::::::::::::::
// UI_UpdateScreen
//:::::::::::::::::::
void uiUpdateScreen(void) { id3UpdateScreen(); }

//:::::::::::::::::::
// UI_Refresh
//:::::::::::::::::::
void uiRefresh(int realtime) {
  // Update time
  uis.frametime = realtime - uis.realtime;
  uis.realtime  = realtime;
  if (!(id3Key_GetCatcher() & KEYCATCH_UI)) { return; }
  // Update cvars
  uiCvarsUpdateAll();
  // Draw active menu
  if (uis.activemenu) {
    // Draw the background
    if (uis.activemenu->fullscreen) { uiDrawHandlePicPix(0, 0, GL_W, GL_H, (uis.activemenu->isMain) ? uis.bgMain : uis.bgAlt); }
    // Draw the menu
    if (uis.activemenu->draw) uis.activemenu->draw();
    else uiDrawMenu(uis.activemenu);
    // Init the cursor position
    if (uis.firstdraw) {
      // uiEvent_mouse(GL_W * 0.5, GL_H * 0.5);  // Start with cursor at 0.5, 0.5 (was 0,0)
      uiEvent_mouse(0, 0);
      uis.firstdraw = false;
    }
  }
  // draw cursor
  uiSetColor(NULL);
  uiDrawHandlePicPix(uis.cursorx, uis.cursory, 32, 32, uis.cursor);
  if (uis.debug) {
    char* coordText = va("(%d,%d)", uis.cursorx, uis.cursory);
    float x         = (float)uis.cursorx / GL_W;
    float y         = ((float)uis.cursory / GL_H) + 0.05;
    uiTextDraw(coordText, &uis.font.normal, x, y, fontScale(&uis.font.normal), colorCyan, 0, 0, strlen(coordText), TEXT_ALIGN_LEFT);
  }
  // Play the enter sound after menu has been drawn, to avoid delay while caching images
  if (m_entersound) {
    // id3S_StartLocalSound(q3sound.menu_in, CHAN_LOCAL_SOUND);
    id3S_StartLocalSound(uiSound.move, CHAN_LOCAL_SOUND);
    m_entersound = false;
  }
  if (m_enterSong) { 
    uiSongPlayRandom();
    m_enterSong = false;
  }
}
