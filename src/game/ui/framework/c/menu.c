#include "../../local.h"

//:::::::::::::::::::
// menuCache
//   Register general menu cache
//:::::::::::::::::::
void menuCache(void) {
  // New UI assets
  uis.bgMain      = id3R_RegisterShaderNoMip("ui/bg");
  uis.bgAlt       = id3R_RegisterShaderNoMip("ui/bgAlt");
  uis.cursor      = id3R_RegisterShaderNoMip("ui/cursor");
  uis.logoQ3      = id3R_RegisterShaderNoMip("ui/logoQ3");
  // Icons
  uis.icon.cancel = id3R_RegisterShaderNoMip("ui/cancel");
  uis.icon.accept = id3R_RegisterShaderNoMip("ui/accept");
  // Sounds
  uiSound.move    = id3S_RegisterSound("ui/snd/move.wav", false);
  uiSound.select  = id3S_RegisterSound("ui/snd/select.wav", false);
  uiSound.error   = id3S_RegisterSound("ui/snd/error.wav", false);
  uiSound.cancel  = id3S_RegisterSound("ui/snd/cancel.wav", false);
  uiSound.silence = id3S_RegisterSound("ui/snd/silence.wav", false);
  // Songs
  song.chronos    = id3S_RegisterSound("ui/snd/song-chronos.wav", false);
  // song.succubus   = id3S_RegisterSound("ui/snd/songSuccubus.wav", false);
  // Fonts
  id3R_RegisterFont(FONT_FILE_DEFAULT, FONT_SIZE_DEFAULT - 4, &uis.font.small);
  id3R_RegisterFont(FONT_FILE_DEFAULT, FONT_SIZE_DEFAULT, &uis.font.normal);
  id3R_RegisterFont(FONT_FILE_ACTION, FONT_SIZE_ACTION, &uis.font.action);
  id3R_RegisterFont(FONT_FILE_ACTIONKEY, FONT_SIZE_ACTIONKEY, &uis.font.actionKey);

  // Old Q3 assets
  // uis.charset          = id3R_RegisterShaderNoMip("gfx/2d/bigchars");
  // uis.charsetProp      = id3R_RegisterShaderNoMip("menu/art/font1_prop.tga");
  // uis.charsetPropGlow  = id3R_RegisterShaderNoMip("menu/art/font1_prop_glo.tga");
  // uis.charsetPropB     = id3R_RegisterShaderNoMip("menu/art/font2_prop.tga");
  // uis.cursor               = id3R_RegisterShaderNoMip("menu/art/3_cursor2");
  uis.rb_on            = id3R_RegisterShaderNoMip("menu/art/switch_on");
  uis.rb_off           = id3R_RegisterShaderNoMip("menu/art/switch_off");
  uis.whiteShader      = id3R_RegisterShaderNoMip("white");

  // TODO: Move to menu/snd/*
  q3sound.menu_in      = id3S_RegisterSound("sound/misc/menu1.wav", false);
  q3sound.menu_out     = id3S_RegisterSound("sound/misc/menu3.wav", false);
  q3sound.menu_move    = id3S_RegisterSound("sound/misc/menu2.wav", false);
  q3sound.menu_buzz    = id3S_RegisterSound("sound/misc/menu4.wav", false);
  q3sound.weaponChange = id3S_RegisterSound("sound/weapons/change.wav", false);
  q3sound.menu_null    = -1;  // need a nonzero sound, make an empty sound for this

  // sliderBar                = id3R_RegisterShaderNoMip("menu/art/slider2");
  // sliderButton_0           = id3R_RegisterShaderNoMip("menu/art/sliderbutt_0");
  // sliderButton_1           = id3R_RegisterShaderNoMip("menu/art/sliderbutt_1");
  Com_Printf(":: Finished loading menu cache\n");
}

//:::::::::::::::::::
// menuForceOff
//   Disables activemenu
//   Clears all keystates
//   Sets singleplayer and paused to off
//:::::::::::::::::::
void menuForceOff(void) {
  uis.menusp     = 0;
  uis.activemenu = NULL;
  id3Key_SetCatcher(id3Key_GetCatcher() & ~KEYCATCH_UI);
  id3Key_ClearStates();
  id3Cvar_Set("cl_paused", "0");
}
//:::::::::::::::::::

//
//:::::::::::::::::::
// uiPushMenu
//:::::::::::::::::::
void menuPush(MenuFw* menu) {
  // avoid stacking menus invoked by hotkeys
  int m;                              // Menu id
  for (m = 0; m < uis.menusp; m++) {  // Search for the input menu in the menus stack
    if (uis.stack[m] == menu) {       // Found it
      uis.menusp = m;
      break;
    }
  }
  if (m == uis.menusp) {  // Check if we are at the last registered item
    if (uis.menusp >= MAX_MENUDEPTH) { id3Error(va("%s: menu stack overflow", __func__)); }
    uis.stack[uis.menusp++] = menu;  // Increase the stack id, and store the menu data
  }
  uis.activemenu    = menu;  // Make input menu the active one
  // default cursor position
  menu->cursor      = 0;
  menu->cursor_prev = 0;
  m_entersound      = true;
  if (menu->isMain) m_enterSong = true;
  id3Key_SetCatcher(KEYCATCH_UI);
  // force first available item to have focus
  for (int it = 0; it < menu->nitems; it++) {  // For every item in the input menu
    MenuCommon* item = (MenuCommon*)menu->items[it];
    if (!(item->flags & (MFL_GRAYED | MFL_MOUSEONLY | MFL_INACTIVE))) {  // Ignore grayed, mouseonly and inactive items
      menu->cursor_prev = -1;
      cursorSet(menu, it);
      break;
    }
  }
  uis.firstdraw = true;
}
//:::::::::::::::::::
// uiPopMenu
//:::::::::::::::::::
void menuPop(void) {
  id3S_StartLocalSound(uiSound.cancel, CHAN_LOCAL_SOUND);
  uis.menusp--;
  if (uis.menusp < 0) { id3Error(va("%s: menu stack underflow", __func__)); }
  if (uis.menusp) {
    uis.activemenu = uis.stack[uis.menusp - 1];
    uis.firstdraw  = true;
  } else {
    menuForceOff();
  }
}

//:::::::::::::::::::
// Menu_AddItem
//:::::::::::::::::::
void menuAddItem(MenuFw* menu, void* item) {
  if (menu->nitems >= MAX_MENUITEMS) { id3Error("Menu_AddItem: excessive items"); }
  // Set data
  menu->items[menu->nitems]                          = item;
  ((MenuCommon*)menu->items[menu->nitems])->parent   = menu;
  ((MenuCommon*)menu->items[menu->nitems])->activeId = menu->nitems;
  ((MenuCommon*)menu->items[menu->nitems])->flags &= ~MFL_HASMOUSEFOCUS;
  // perform any item specific initializations
  MenuCommon* itemptr = (MenuCommon*)item;
  if (!(itemptr->flags & MFL_NODEFAULTINIT)) {
    switch (itemptr->type) {
      case MITEM_ACTION: menuAction_init((MenuAction*)item); break;
      case MITEM_FIELD: menuField_init((MenuField*)item); break;
      case MITEM_MULTIOPT: menuMOpt_init((MenuList*)item); break;
      case MITEM_SWITCH: menuSwitch_init((MenuSwitch*)item); break;
      case MITEM_SLIDER: menuSlider_init((MenuSlider*)item); break;
      case MITEM_IMAGE: menuImage_init((MenuImage*)item); break;
      case MITEM_LTEXT: OText_init((MenuText*)item); break;
      case MITEM_LIST: menuList_init((MenuList*)item); break;
      case MITEM_TEXT: menuText_init((MenuText*)item); break;
      // case MITEM_BTEXT: BText_init((MenuText*)item); break;
      default: id3Error(va("Menu_Init: unknown type %d", itemptr->type));
    }
  }
  menu->nitems++;
}

//:::::::::::::::::::
// Menu_ActivateItem
//:::::::::::::::::::
sfxHandle_t menuActivateItem(MenuFw* s, MenuCommon* item) {
  if (item->callback) {
    item->callback(item, MST_ACTIVE);
    if (!(item->flags & MFL_SILENT)) { return uiSound.move; }
  }
  return 0;
}

//:::::::::::::::::::
// Menu_DefaultKey
//:::::::::::::::::::
sfxHandle_t menuDefaultKey(MenuFw* m, int key) {
  // menu system keys
  switch (key) {
    case K_MOUSE2:
    case K_ESCAPE: menuPop(); return uiSound.cancel;
  }
  if (!m || !m->nitems) { return 0; }
  // route key stimulus to widget
  sfxHandle_t sound = 0;
  MenuCommon* item  = cursorGetItem(m);
  if (item && !(item->flags & (MFL_GRAYED | MFL_INACTIVE))) {
    switch (item->type) {
      case MITEM_MULTIOPT: sound = menuMOpt_key((MenuList*)item, key); break;
      case MITEM_SWITCH: sound = menuSwitch_key((MenuSwitch*)item, key); break;
      case MITEM_SLIDER: sound = menuSlider_key((MenuSlider*)item, key); break;
      case MITEM_LIST: sound = menuList_key((MenuList*)item, key); break;
      case MITEM_FIELD: sound = menuField_key((MenuField*)item, &key); break;
    }
    if (sound) { return sound; }  // key was handled
  }
  // default handling
  int cursor_prev;
  switch (key) {
#ifndef NDEBUG
    case K_F11: uis.debug ^= 1; break;
    case K_F12: id3Cmd_ExecuteText(EXEC_APPEND, "screenshot\n"); break;
#endif
    case K_KP_UPARROW:
    case K_UPARROW:
      cursor_prev    = m->cursor;
      m->cursor_prev = m->cursor;
      m->cursor--;
      cursorAdjust(m, -1);
      if (cursor_prev != m->cursor) {
        cursorMoved(m);
        sound = uiSound.move;
      }
      break;

    case K_TAB:
    case K_KP_DOWNARROW:
    case K_DOWNARROW:
      cursor_prev    = m->cursor;
      m->cursor_prev = m->cursor;
      m->cursor++;
      cursorAdjust(m, 1);
      if (cursor_prev != m->cursor) {
        cursorMoved(m);
        sound = uiSound.move;
      }
      break;

    case K_MOUSE1:
    case K_MOUSE3:
      if (item && item->type != MITEM_LIST && (item->flags & MFL_HASMOUSEFOCUS) && !(item->flags & (MFL_GRAYED | MFL_INACTIVE))) {
        return (menuActivateItem(m, item));
      }
      break;

    case K_JOY1:
    case K_JOY2:
    case K_JOY3:
    case K_JOY4:
    case K_AUX1:
    case K_AUX2:
    case K_AUX3:
    case K_AUX4:
    case K_AUX5:
    case K_AUX6:
    case K_AUX7:
    case K_AUX8:
    case K_AUX9:
    case K_AUX10:
    case K_AUX11:
    case K_AUX12:
    case K_AUX13:
    case K_AUX14:
    case K_AUX15:
    case K_AUX16:
    case K_KP_ENTER:
    case K_ENTER:
      if (item && !(item->flags & (MFL_MOUSEONLY | MFL_GRAYED | MFL_INACTIVE))) {  // sk.chg -> Removed nested if
        return (menuActivateItem(m, item));
      }
      break;
  }
  return sound;
}
