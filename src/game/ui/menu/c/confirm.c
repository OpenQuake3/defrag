#include "../all.h"  // menu/all.h

//:::::::::::::::::::
// Confirm Menu config
#define MID_CONFIRM_NO 10
#define MID_CONFIRM_YES 11
#define SHADER_CONFIRM_FRAME "menu/art/cut_frame"
//:::::::::::::::::::
// Confirm Menu data types
typedef struct sep_s {
  const char* text;
  fontInfo_t  font;
  int         align;  // Text alignment type
  vec2_t      pos;    // x,y position (percentage)
  float       yMod;   // Amount (percentage) to modify Y positioning, to compensate for the font's face vertical size
} Separator;
//:::::::::::::::::::
typedef struct {
  MenuFw       menu;
  MenuText     no;
  MenuText     yes;
  int          style;
  fontInfo_t   font;
  int          align;
  vec2_t       pos;
  float        yMargin;  // Margin (percentage) to add to raw spacing, to calculate absolute line spacing
  Separator    sep;
  const char*  question;
  const char** lines;
  void (*draw)(void);
  void (*action)(bool result);
} MenuConfirm;
//:::::::::::::::::::
static MenuConfirm s_confirm;
//:::::::::::::::::::


//:::::::::::::::::::
// menuConfirm_Event
//:::::::::::::::::::
static void menuConfirm_event(void* ptr, int event) {
  if (event != MST_ACTIVE) { return; }
  menuPop();
  bool result = (((MenuCommon*)ptr)->id == MID_CONFIRM_YES);
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
    case 'N': menuConfirm_event(&s_confirm.no, MST_ACTIVE); break;
    case 'y':
    case 'Y': menuConfirm_event(&s_confirm.yes, MST_ACTIVE); break;
  }
  return menuDefaultKey(&s_confirm.menu, key);
}


// menuConfirm_Draw
static void menuConfirm_draw(void) {
  float x        = s_confirm.pos[0];
  float y        = s_confirm.pos[1];
  float ySpacing = 0;                  // Raw Spacing between one line and the next
  float yMargin  = s_confirm.yMargin;  // Margin to add to raw spacing, to calculate absolute line spacing
  // Draw the Question
  uiTextDraw(s_confirm.question, &s_confirm.font, x, y, fontScale(&s_confirm.font), (vec_t*)mColorQ3.fg, 0, s_confirm.style, strlen(s_confirm.question), s_confirm.align);
  // Add line spacing
  ySpacing = uiTextGetHeight(s_confirm.question, &s_confirm.font, fontScale(&s_confirm.font), strlen(s_confirm.question));
  y += ySpacing - s_confirm.sep.yMod + yMargin;

  // Draw the separator
  uiTextDraw(s_confirm.sep.text, &s_confirm.sep.font, x, y, fontScale(&s_confirm.sep.font), (vec_t*)mColorQ3.fg, 0, UI_INACTIVE, strlen(s_confirm.sep.text), s_confirm.sep.align);

  // Draw the options
  uiDrawMenu(&s_confirm.menu);
  if (s_confirm.draw) { s_confirm.draw(); }
}

//:::::::::::::::::::
// menuConfirm_Cache
//:::::::::::::::::::
void menuConfirm_cache(void) {
  id3R_RegisterShaderNoMip(SHADER_CONFIRM_FRAME);  // Ugly round frame
}

//:::::::::::::::::::
// UI_MenuConfirm_Style
//:::::::::::::::::::
// When ySep is 0, y position is not modified
void menuConfirm_init(const char* question, fontInfo_t* font, float ySep, int style, void (*draw)(void), void (*action)(bool result)) {
  // zero set all our globals
  memset(&s_confirm, 0, sizeof(s_confirm));
  // Default positions
  float x       = 0.5;
  float y       = 0.55;
  float xMargin = 0.01;  // Horizontal separation between one item and the other (percentage)
  float yMargin = 0.03;  // Vertical   separation between one item and the other (percentage)
  // Load the menu cache
  menuConfirm_cache();
  // Separator properties
  Vector2Set(s_confirm.sep.pos, x, y);
  s_confirm.sep.font             = uis.font.action;
  s_confirm.sep.align            = TEXT_ALIGN_CENTER;
  s_confirm.sep.text             = "|";
  s_confirm.sep.yMod             = ySep;
  float sepW                     = uiTextGetWidth(s_confirm.sep.text, &s_confirm.sep.font, fontScale(&s_confirm.sep.font), strlen(s_confirm.sep.text));

  s_confirm.yes.string           = "Yes";
  s_confirm.yes.color            = (vec_t*)mColorQ3.fg;
  s_confirm.yes.font             = uis.font.action;
  s_confirm.yes.align            = TEXT_ALIGN_CENTER;
  s_confirm.yes.generic.type     = MITEM_TEXT;
  s_confirm.yes.generic.flags    = MFL_LEFT_JUSTIFY | MFL_PULSEIFFOCUS;
  s_confirm.yes.generic.callback = menuConfirm_event;
  s_confirm.yes.generic.id       = MID_CONFIRM_YES;
  s_confirm.yes.generic.y        = y - ySep + yMargin;
  float yesW                     = uiTextGetWidth(s_confirm.yes.string, &s_confirm.yes.font, fontScale(&s_confirm.yes.font), strlen(s_confirm.yes.string));
  s_confirm.yes.generic.x        = x - sepW * 0.5 - xMargin - yesW * 0.5;  // Different alignment options? Probably not needed


  s_confirm.no.string            = "No";
  s_confirm.no.color             = (vec_t*)mColorQ3.fg;
  s_confirm.no.font              = uis.font.action;
  s_confirm.no.align             = TEXT_ALIGN_CENTER;
  s_confirm.no.generic.type      = MITEM_TEXT;
  s_confirm.no.generic.flags     = MFL_LEFT_JUSTIFY | MFL_PULSEIFFOCUS;
  s_confirm.no.generic.callback  = menuConfirm_event;
  s_confirm.no.generic.id        = MID_CONFIRM_NO;
  s_confirm.no.generic.y         = y - ySep + yMargin;
  float noW                      = uiTextGetWidth(s_confirm.no.string, &s_confirm.no.font, fontScale(&s_confirm.no.font), strlen(s_confirm.no.string));
  s_confirm.no.generic.x         = x + sepW * 0.5 + xMargin + noW * 0.5;  // Different alignment options? Probably not needed

  // Main question properties
  s_confirm.font                 = *font;
  s_confirm.align                = TEXT_ALIGN_CENTER;
  s_confirm.question             = question;
  s_confirm.draw                 = draw;
  s_confirm.action               = action;
  s_confirm.style                = style;
  s_confirm.menu.draw            = menuConfirm_draw;
  s_confirm.menu.key             = menuConfirm_key;
  s_confirm.menu.wrapAround      = true;
  s_confirm.yMargin              = yMargin;
  Vector2Set(s_confirm.pos, 0.5, 0.45);


  uiClientState_t cstate;
  id3GetClientState(&cstate);
  if (cstate.connState >= CA_CONNECTED) s_confirm.menu.fullscreen = false;  // if connected, loading, primed, active or cinematic/image
  else s_confirm.menu.fullscreen = true;
  // s_confirm.menu.fullscreen = false;


  menuAddItem(&s_confirm.menu, &s_confirm.yes);
  menuAddItem(&s_confirm.menu, &s_confirm.no);

  menuPush(&s_confirm.menu);

  cursorSetToItem(&s_confirm.menu, &s_confirm.no);
}

//:::::::::::::::::::
// UI_menuConfirm
//:::::::::::::::::::
void menuConfirm(const char* question, void (*draw)(void), void (*action)(bool result)) {
  float yMod = uiTextGetHeight(question, &uis.font.actionKey, fontScale(&uis.font.actionKey), strlen(question)) * 0.5;
  menuConfirm_init(question, &uis.font.actionKey, yMod, UI_CENTER | UI_INACTIVE, draw, action);
}
//:::::::::::::::::::
