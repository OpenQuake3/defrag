
#include "../../local.h"

//:::::::::::::::::::::::
// SpinControl_Init
//:::::::::::::::::::::::
void menuMOpt_init(MenuList* s) {
  int len         = (s->generic.name) ? strlen(s->generic.name) * SMALLCHAR_WIDTH : 0;
  s->generic.left = s->generic.x - SMALLCHAR_WIDTH - len;
  int         l;
  const char* str;
  len = s->itemCount = 0;
  while ((str = s->itemNames[s->itemCount]) != 0) {
    l = strlen(str);
    if (l > len) { len = l; }
    s->itemCount++;
  }
  s->generic.top    = s->generic.y;
  s->generic.right  = s->generic.x + (len + 1) * SMALLCHAR_WIDTH;
  s->generic.bottom = s->generic.y + SMALLCHAR_HEIGHT;
}

//:::::::::::::::::::::::
// SpinControl_Key
//:::::::::::::::::::::::
sfxHandle_t menuMOpt_key(MenuList* s, int key) {
  sfxHandle_t sound = 0;
  switch (key) {
    case K_KP_RIGHTARROW:
    case K_RIGHTARROW:
    case K_MOUSE1:
      s->curvalue++;
      if (s->curvalue >= s->itemCount) s->curvalue = 0;
      sound = uiSound.move;
      break;
    case K_KP_LEFTARROW:
    case K_LEFTARROW:
      s->curvalue--;
      if (s->curvalue < 0) s->curvalue = s->itemCount - 1;
      sound = uiSound.move;
      break;
  }
  if (sound && s->generic.callback) s->generic.callback(s, MST_ACTIVE);
  return (sound);
}

//:::::::::::::::::::::::
// SpinControl_Draw
//:::::::::::::::::::::::
void menuMOpt_draw(MenuList* s) {
  float* color;
  int    style = UI_SMALLFONT;
  bool   focus = (s->generic.parent->cursor == s->generic.activeId);
  if (s->generic.flags & MFL_GRAYED) {
    color = (vec_t*)q3color.text_disabled;
  } else if (focus) {
    color = (vec_t*)q3color.text_highlight;
    style |= UI_PULSE;
  } else if (s->generic.flags & MFL_BLINK) {
    color = (vec_t*)q3color.text_highlight;
    style |= UI_BLINK;
  } else color = (vec_t*)q3color.text_normal;

  int x = s->generic.x;
  int y = s->generic.y;
  if (focus) {
    // draw cursor
    uiFillRect(s->generic.left,
               s->generic.top,
               s->generic.right - s->generic.left + 1,
               s->generic.bottom - s->generic.top + 1,
               (vec_t*)q3color.listbar);
    uiDrawChar(x, y, 13, UI_CENTER | UI_BLINK | UI_SMALLFONT, color);
  }

  uiDrawString(x - SMALLCHAR_WIDTH, y, s->generic.name, style | UI_RIGHT, color);
  uiDrawString(x + SMALLCHAR_WIDTH, y, s->itemNames[s->curvalue], style | UI_LEFT, color);
}
