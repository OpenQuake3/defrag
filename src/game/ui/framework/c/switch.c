#include "../elements.h"

//:::::::::::::::::
// RadioButton_Init
//:::::::::::::::::
void menuSwitch_init(MenuSwitch* rb) {
  int len            = (rb->generic.name) ? strlen(rb->generic.name) : 0;  // calculate bounds
  rb->generic.left   = rb->generic.x - (len + 1) * SMALLCHAR_WIDTH;
  rb->generic.right  = rb->generic.x + 6 * SMALLCHAR_WIDTH;
  rb->generic.top    = rb->generic.y;
  rb->generic.bottom = rb->generic.y + SMALLCHAR_HEIGHT;
}

//:::::::::::::::::
// RadioButton_Key
//:::::::::::::::::
sfxHandle_t menuSwitch_key(MenuSwitch* rb, int key) {
  switch (key) {
    case K_MOUSE1:
      if (!(rb->generic.flags & MFL_HASMOUSEFOCUS)) { break; }
    case K_JOY1:
    case K_JOY2:
    case K_JOY3:
    case K_JOY4:
    case K_ENTER:
    case K_KP_ENTER:
    case K_KP_LEFTARROW:
    case K_LEFTARROW:
    case K_KP_RIGHTARROW:
    case K_RIGHTARROW:
      rb->curvalue = !rb->curvalue;
      if (rb->generic.callback) rb->generic.callback(rb, MST_ACTIVE);
      return (uiSound.move);
  }
  return 0; // key not handled
}

//:::::::::::::::::::::::
// drawRadioBtn
//:::::::::::::::::::::::
void menuSwitch_draw(MenuSwitch* rb) {
  int    x      = rb->generic.x;
  int    y      = rb->generic.y;
  bool   grayed = (rb->generic.flags & MFL_GRAYED);
  float* color  = (grayed) ? (vec_t*)q3color.text_disabled : (vec_t*)q3color.text_normal;
  int    style  = UI_LEFT | UI_SMALLFONT;
  bool   focus  = (rb->generic.parent->cursor == rb->generic.activeId);
  if (focus) {
    color = (vec_t*)q3color.text_highlight;
    style |= UI_PULSE;
    // draw cursor
    uiFillRect(rb->generic.left,
               rb->generic.top,
               rb->generic.right - rb->generic.left + 1,
               rb->generic.bottom - rb->generic.top + 1,
               (vec_t*)q3color.listbar);
    uiDrawChar(x, y, 13, UI_CENTER | UI_BLINK | UI_SMALLFONT, color);
  }

  if (rb->generic.name) { uiDrawString(x - SMALLCHAR_WIDTH, y, rb->generic.name, UI_RIGHT | UI_SMALLFONT, color); }
  if (!rb->curvalue) {
    uiDrawHandlePicPix(x + SMALLCHAR_WIDTH, y + 2, 16, 16, uis.rb_off);
    uiDrawString(x + SMALLCHAR_WIDTH + 16, y, "off", style, color);
  } else {
    uiDrawHandlePicPix(x + SMALLCHAR_WIDTH, y + 2, 16, 16, uis.rb_on);
    uiDrawString(x + SMALLCHAR_WIDTH + 16, y, "on", style, color);
  }
}
