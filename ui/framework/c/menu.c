#include "../../local.h"

//:::::::::::::::::::
// menuCache
//   Register general menu cache
//:::::::::::::::::::
void menuCache(void) {
  // New UI assets
  uis.menuBackShader       = id3R_RegisterShaderNoMip("ui/bgtest");
  uis.cursor               = id3R_RegisterShaderNoMip("ui/cursor");
  // Old Q3 assets
  uis.charset              = id3R_RegisterShaderNoMip("gfx/2d/bigchars");
  uis.charsetProp          = id3R_RegisterShaderNoMip("menu/art/font1_prop.tga");
  uis.charsetPropGlow      = id3R_RegisterShaderNoMip("menu/art/font1_prop_glo.tga");
  uis.charsetPropB         = id3R_RegisterShaderNoMip("menu/art/font2_prop.tga");
  // uis.cursor               = id3R_RegisterShaderNoMip("menu/art/3_cursor2");
  uis.rb_on                = id3R_RegisterShaderNoMip("menu/art/switch_on");
  uis.rb_off               = id3R_RegisterShaderNoMip("menu/art/switch_off");
  uis.whiteShader          = id3R_RegisterShaderNoMip("white");

  // uis.menuBackNoLogoShader = id3R_RegisterShaderNoMip("menubacknologo");
  // uis.menuBackShader       = id3R_RegisterShaderNoMip("menuback");

  q3sound.menu_in          = id3S_RegisterSound("sound/misc/menu1.wav", false);
  q3sound.menu_out         = id3S_RegisterSound("sound/misc/menu3.wav", false);
  q3sound.menu_move        = id3S_RegisterSound("sound/misc/menu2.wav", false);
  q3sound.menu_buzz        = id3S_RegisterSound("sound/misc/menu4.wav", false);
  q3sound.weaponChange     = id3S_RegisterSound("sound/weapons/change.wav", false);
  q3sound.menu_null        = -1;  // need a nonzero sound, make an empty sound for this

  // sliderBar                = id3R_RegisterShaderNoMip("menu/art/slider2");
  // sliderButton_0           = id3R_RegisterShaderNoMip("menu/art/sliderbutt_0");
  // sliderButton_1           = id3R_RegisterShaderNoMip("menu/art/sliderbutt_1");
  Com_Printf(":: Finished loading the menu cache\n");
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
  int i;
  for (i = 0; i < uis.menusp; i++) {
    if (uis.stack[i] == menu) {
      uis.menusp = i;
      break;
    }
  }
  if (i == uis.menusp) {
    if (uis.menusp >= MAX_MENUDEPTH) { id3Error(va("%s: menu stack overflow", __func__)); }
    uis.stack[uis.menusp++] = menu;
  }
  uis.activemenu    = menu;
  // default cursor position
  menu->cursor      = 0;
  menu->cursor_prev = 0;
  m_entersound      = true;
  id3Key_SetCatcher(KEYCATCH_UI);
  // force first available item to have focus
  MenuCommon* item;
  for (int j = 0; j < menu->nitems; j++) {
    item = (MenuCommon*)menu->items[j];
    if (!(item->flags & (MFL_GRAYED | MFL_MOUSEONLY | MFL_INACTIVE))) {
      menu->cursor_prev = -1;
      cursorSet(menu, j);
      break;
    }
  }
  uis.firstdraw = true;
}
//:::::::::::::::::::
// uiPopMenu
//:::::::::::::::::::
void menuPop(void) {
  id3S_StartLocalSound(q3sound.menu_out, CHAN_LOCAL_SOUND);
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
  menu->items[menu->nitems]                              = item;
  ((MenuCommon*)menu->items[menu->nitems])->parent       = menu;
  ((MenuCommon*)menu->items[menu->nitems])->menuPosition = menu->nitems;
  ((MenuCommon*)menu->items[menu->nitems])->flags &= ~MFL_HASMOUSEFOCUS;
  // perform any item specific initializations
  MenuCommon* itemptr = (MenuCommon*)item;
  if (!(itemptr->flags & MFL_NODEFAULTINIT)) {
    switch (itemptr->type) {
      case MITEM_ACTION: action_init((MenuAction*)item); break;
      case MITEM_FIELD: menuField_init((MenuField*)item); break;
      case MITEM_SPINCONTROL: spinControl_init((MenuList*)item); break;
      case MITEM_RADIOBUTTON: radioBtn_init((MenuRadioBtn*)item); break;
      case MITEM_SLIDER: slider_init((MenuSlider*)item); break;
      case MITEM_BITMAP: bitmap_init((MenuBitmap*)item); break;
      case MITEM_TEXT: text_init((MenuText*)item); break;
      case MITEM_SCROLLLIST: scrollList_init((MenuList*)item); break;
      case MITEM_PTEXT: PText_init((MenuText*)item); break;
      case MITEM_BTEXT: BText_init((MenuText*)item); break;
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
    item->callback(item, MS_ACTIVATED);
    if (!(item->flags & MFL_SILENT)) { return q3sound.menu_move; }
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
    case K_ESCAPE: menuPop(); return q3sound.menu_out;
  }
  if (!m || !m->nitems) { return 0; }
  // route key stimulus to widget
  sfxHandle_t sound = 0;
  MenuCommon* item  = cursorGetItem(m);
  if (item && !(item->flags & (MFL_GRAYED | MFL_INACTIVE))) {
    switch (item->type) {
      case MITEM_SPINCONTROL: sound = spinControl_key((MenuList*)item, key); break;
      case MITEM_RADIOBUTTON: sound = radioBtn_key((MenuRadioBtn*)item, key); break;
      case MITEM_SLIDER: sound = slider_key((MenuSlider*)item, key); break;
      case MITEM_SCROLLLIST: sound = scrollList_key((MenuList*)item, key); break;
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
        sound = q3sound.menu_move;
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
        sound = q3sound.menu_move;
      }
      break;

    case K_MOUSE1:
    case K_MOUSE3:
      if (item && (item->flags & MFL_HASMOUSEFOCUS) && !(item->flags & (MFL_GRAYED | MFL_INACTIVE))) {  // sk.chg -> Removed nested if
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
