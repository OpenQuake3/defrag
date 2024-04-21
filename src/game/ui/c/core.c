
#include "../local.h"

//:::::::::::::::::::::::
uiStatic_t uis;
//:::::::::::::::::::

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
      menuStart();
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
  if (uis.activemenu->key) s = uis.activemenu->key(key);
  else s = menuDefaultKey(uis.activemenu, key);
  if ((s > 0) && (s != q3sound.menu_null)) id3S_StartLocalSound(s, CHAN_LOCAL_SOUND);
}

//:::::::::::::::::::
// UI_MouseEvent
//:::::::::::::::::::
void uiEvent_mouse(int dx, int dy) {
  if (!uis.activemenu) return;

  // convert X bias to 640 coords
  int bias = uis.bias / uis.xscale;
  // update mouse screen position
  uis.cursorx += dx;
  if (uis.cursorx < -bias) uis.cursorx = -bias;
  else if (uis.cursorx > SCREEN_WIDTH + bias) uis.cursorx = SCREEN_WIDTH + bias;
  uis.cursory += dy;
  if (uis.cursory < 0) uis.cursory = 0;
  else if (uis.cursory > SCREEN_HEIGHT) uis.cursory = SCREEN_HEIGHT;

  // region test the active menu items
  MenuCommon* m;
  for (int i = 0; i < uis.activemenu->nitems; i++) {
    m = (MenuCommon*)uis.activemenu->items[i];
    if (m->flags & (MFL_GRAYED | MFL_INACTIVE)) { continue; }
    if ((uis.cursorx < m->left) || (uis.cursorx > m->right) || (uis.cursory < m->top) || (uis.cursory > m->bottom)) {
      continue;  // cursor out of item bounds
    }

    // set focus to item at cursor
    if (uis.activemenu->cursor != i) {
      cursorSet(uis.activemenu, i);
      ((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~MFL_HASMOUSEFOCUS;
      if (!(((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor]))->flags & MFL_SILENT)) {
        id3S_StartLocalSound(q3sound.menu_move, CHAN_LOCAL_SOUND);
      }
    }
    ((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor]))->flags |= MFL_HASMOUSEFOCUS;
    return;
  }
  if (uis.activemenu->nitems > 0) {  // out of any region
    ((MenuCommon*)(uis.activemenu->items[uis.activemenu->cursor]))->flags &= ~MFL_HASMOUSEFOCUS;
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
  uis.frametime = realtime - uis.realtime;
  uis.realtime  = realtime;
  if (!(id3Key_GetCatcher() & KEYCATCH_UI)) { return; }
  uiCvarsUpdateAll();
  if (uis.activemenu) {
    if (uis.activemenu->fullscreen) {
      // draw the background
      if (uis.activemenu->showlogo) {
        uiDrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackShader);
      } else {
        uiDrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackNoLogoShader);
      }
    }
    if (uis.activemenu->draw) uis.activemenu->draw();
    else uiDrawMenu(uis.activemenu);
    if (uis.firstdraw) {
      uiEvent_mouse(0, 0);
      uis.firstdraw = false;
    }
  }
  // draw cursor
  uiSetColor(NULL);
  uiDrawHandlePic(uis.cursorx - 16, uis.cursory - 16, 32, 32, uis.cursor);
#ifndef NDEBUG
  if (uis.debug) { uiDrawString(0, 0, va("(%d,%d)", uis.cursorx, uis.cursory), UI_LEFT | UI_SMALLFONT, colorRed); }  // cursor coordinates
#endif
  // delay playing the enter sound until the menu has been drawn, to avoid delay while caching images
  if (m_entersound) {
    id3S_StartLocalSound(q3sound.menu_in, CHAN_LOCAL_SOUND);
    m_entersound = false;
  }
}
