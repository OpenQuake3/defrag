#include "../all.h"  // menu/all.h

//:::::::::::::::::::
// Confirm Menu config
#define MID_CONFIRM_NO 10
#define MID_CONFIRM_YES 11
#define SHADER_CONFIRM_FRAME "menu/art/cut_frame"
//:::::::::::::::::::
// Confirm Menu data types
typedef struct {
  MenuFw       menu;
  MenuText     no;
  MenuText     yes;
  int          slashX;
  int          style;
  const char*  question;
  const char** lines;
  void (*draw)(void);
  void (*action)(bool result);
} confirmMenu_t;
//:::::::::::::::::::
static confirmMenu_t s_confirm;
//:::::::::::::::::::


//:::::::::::::::::::
// menuConfirm_Event
//:::::::::::::::::::
static void menuConfirm_event(void* ptr, int event) {
  if (event != MS_ACTIVATED) { return; }
  menuPop();
  bool result;
  if (((MenuCommon*)ptr)->id == MID_CONFIRM_NO) {
    result = false;
  } else {
    result = true;
  }
  if (s_confirm.action) { s_confirm.action(result); }
}
//:::::::::::::::::::
// menuConfirm_Key
//:::::::::::::::::::
static sfxHandle_t menuConfirm_key(int key) {
  switch (key) {
    case K_KP_LEFTARROW:
    case K_LEFTARROW:
    case K_KP_RIGHTARROW:
    case K_RIGHTARROW: key = K_TAB; break;
    case 'n':
    case 'N': menuConfirm_event(&s_confirm.no, MS_ACTIVATED); break;
    case 'y':
    case 'Y': menuConfirm_event(&s_confirm.yes, MS_ACTIVATED); break;
  }
  return menuDefaultKey(&s_confirm.menu, key);
}


// menuConfirm_Draw
static void menuConfirm_draw(void) {
  uiDrawNamedPic(142, 118, 359, 256, SHADER_CONFIRM_FRAME);
  uiDrawPString(320, 204, s_confirm.question, s_confirm.style, (vec_t*)mStartQ3.key);
  uiDrawPString(s_confirm.slashX, 265, "/", UI_LEFT | UI_INVERSE, (vec_t*)mStartQ3.key);
  uiDrawMenu(&s_confirm.menu);
  if (s_confirm.draw) { s_confirm.draw(); }
}

//:::::::::::::::::::
// menuConfirm_Cache
//:::::::::::::::::::
void menuConfirm_cache(void) { id3R_RegisterShaderNoMip(SHADER_CONFIRM_FRAME); }

//:::::::::::::::::::
// UI_MenuConfirm_Style
//:::::::::::::::::::
void menuConfirm_style(const char* question, int style, void (*draw)(void), void (*action)(bool result)) {
  // zero set all our globals
  memset(&s_confirm, 0, sizeof(s_confirm));

  menuConfirm_cache();
  int n1                    = uiPStringWidth("YES/NO");
  int n2                    = uiPStringWidth("YES") + PROP_GAP_WIDTH;
  int n3                    = uiPStringWidth("/") + PROP_GAP_WIDTH;
  int l1                    = 320 - (n1 / 2);
  int l2                    = l1 + n2;
  int l3                    = l2 + n3;
  s_confirm.slashX          = l2;
  s_confirm.question        = question;
  s_confirm.draw            = draw;
  s_confirm.action          = action;
  s_confirm.style           = style;
  s_confirm.menu.draw       = menuConfirm_draw;
  s_confirm.menu.key        = menuConfirm_key;
  s_confirm.menu.wrapAround = true;

  uiClientState_t cstate;
  id3GetClientState(&cstate);
  if (cstate.connState >= CA_CONNECTED) {
    s_confirm.menu.fullscreen = false;
  } else {
    s_confirm.menu.fullscreen = true;
  }

  s_confirm.yes.generic.type     = MITEM_PTEXT;
  s_confirm.yes.generic.flags    = MFL_LEFT_JUSTIFY | MFL_PULSEIFFOCUS;
  s_confirm.yes.generic.callback = menuConfirm_event;
  s_confirm.yes.generic.id       = MID_CONFIRM_YES;
  s_confirm.yes.generic.x        = l1;
  s_confirm.yes.generic.y        = 264;
  s_confirm.yes.string           = "YES";
  s_confirm.yes.color            = (vec_t*)mStartQ3.key;
  s_confirm.yes.style            = UI_LEFT;

  s_confirm.no.generic.type      = MITEM_PTEXT;
  s_confirm.no.generic.flags     = MFL_LEFT_JUSTIFY | MFL_PULSEIFFOCUS;
  s_confirm.no.generic.callback  = menuConfirm_event;
  s_confirm.no.generic.id        = MID_CONFIRM_NO;
  s_confirm.no.generic.x         = l3;
  s_confirm.no.generic.y         = 264;
  s_confirm.no.string            = "NO";
  s_confirm.no.color             = (vec_t*)mStartQ3.key;
  s_confirm.no.style             = UI_LEFT;

  menuAddItem(&s_confirm.menu, &s_confirm.yes);
  menuAddItem(&s_confirm.menu, &s_confirm.no);

  menuPush(&s_confirm.menu);

  cursorSetToItem(&s_confirm.menu, &s_confirm.no);
}

//:::::::::::::::::::
// UI_menuConfirm
//:::::::::::::::::::
void menuConfirm(const char* question, void (*draw)(void), void (*action)(bool result)) {
  menuConfirm_style(question, UI_CENTER | UI_INVERSE, draw, action);
}
//:::::::::::::::::::
