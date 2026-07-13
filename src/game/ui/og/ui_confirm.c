/*
=======================================================================
CONFIRMATION MENU
=======================================================================
*/

#include "ui_local.h"

// ART_CONFIRM_FRAME
// MID_CONFIRM_NO
// MID_CONFIRM_YES
// MenuConfirm
// s_confirm

// ConfirmMenu_Event
// ConfirmMenu_Key
/*
=================
MessageMenu_Draw
=================
*/
static void MessageMenu_Draw(void) {
  int i, y;

  UI_DrawNamedPic(142, 118, 359, 256, ART_CONFIRM_FRAME);

  y = 188;
  for (i = 0; s_confirm.lines[i]; i++) {
    UI_DrawProportionalString(320, y, s_confirm.lines[i], s_confirm.style, color_red);
    y += 18;
  }

  Menu_Draw(&s_confirm.menu);

  if (s_confirm.draw) { s_confirm.draw(); }
}

// ConfirmMenu_Draw
// ConfirmMenu_Cache
// UI_ConfirmMenu_Stlye
// UI_ConfirmMenu

/*
=================
UI_Message
hacked over from Confirm stuff
=================
*/
void UI_Message(const char** lines) {
  // zero set all our globals
  memset(&s_confirm, 0, sizeof(s_confirm));

  menuConfirm_cache();

  int n1                        = UI_ProportionalStringWidth("OK");
  int l1                        = 320 - (n1 / 2);

  s_confirm.lines           = lines;
  s_confirm.style           = UI_CENTER | UI_INACTIVE | UI_SMALLFONT;

  s_confirm.menu.draw       = MessageMenu_Draw;
  s_confirm.menu.key        = menuConfirm_Key;
  s_confirm.menu.wrapAround = true;

  uiClientState_t cstate;
  id3GetClientState(&cstate);
  if (cstate.connState >= CA_CONNECTED) {
    s_confirm.menu.fullscreen = false;
  } else {
    s_confirm.menu.fullscreen = true;
  }

  s_confirm.yes.generic.type     = MITEM_TEXT;
  s_confirm.yes.generic.flags    = MFL_LEFT_JUSTIFY | MFL_PULSEIFFOCUS;
  s_confirm.yes.generic.callback = menuConfirm_Event;
  s_confirm.yes.generic.id       = MID_CONFIRM_YES;
  s_confirm.yes.generic.x        = l1;
  s_confirm.yes.generic.y        = 280;
  s_confirm.yes.string           = "OK";
  s_confirm.yes.color            = color_red;
  s_confirm.yes.style            = UI_LEFT;

  Menu_AddItem(&s_confirm.menu, &s_confirm.yes);

  UI_PushMenu(&s_confirm.menu);

  Menu_SetCursorToItem(&s_confirm.menu, &s_confirm.yes);
}
