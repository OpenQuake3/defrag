
#include "../elements.h"

//:::::::::::::::::
// Action_Init
//:::::::::::::::::
void menuAction_init(MenuAction* a) {
  // calculate bounds
  int len;
  if (a->generic.name) len = strlen(a->generic.name);
  else len = 0;
  // left justify text
  a->generic.left   = a->generic.x;
  a->generic.right  = a->generic.x + len * BIGCHAR_WIDTH;
  a->generic.top    = a->generic.y;
  a->generic.bottom = a->generic.y + BIGCHAR_HEIGHT;
}

//:::::::::::::::::
// Action_Draw
//:::::::::::::::::
void menuAction_draw(MenuAction* a) {
  int    style = 0;
  float* color = (vec_t*)q3color.text_menu;
  if (a->generic.flags & MFL_GRAYED) {
    color = (vec_t*)q3color.text_disabled;
  } else if ((a->generic.flags & MFL_PULSEIFFOCUS) && (a->generic.parent->cursor == a->generic.activeId)) {
    color = (vec_t*)q3color.text_highlight;
    style = UI_PULSE;
  } else if ((a->generic.flags & MFL_HIGHLIGHT_IF_FOCUS) && (a->generic.parent->cursor == a->generic.activeId)) {
    color = (vec_t*)q3color.text_highlight;
  } else if (a->generic.flags & MFL_BLINK) {
    style = UI_BLINK;
    color = (vec_t*)q3color.text_highlight;
  }
  int x = a->generic.x;
  int y = a->generic.y;
  uiDrawString(x, y, a->generic.name, UI_LEFT | style, color);
  // draw cursor
  if (a->generic.parent->cursor == a->generic.activeId) { uiDrawChar(x - BIGCHAR_WIDTH, y, 13, UI_LEFT | UI_BLINK, color); }
}
