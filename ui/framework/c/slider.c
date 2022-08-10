#include "../elements.h"

//:::::::::::::::::::
qhandle_t sliderBar;
qhandle_t sliderButton_0;
qhandle_t sliderButton_1;
//:::::::::::::::::::

//:::::::::::::::::
// Slider_Init
//:::::::::::::::::
void slider_init(MenuSlider* s) {
  int len           = (s->generic.name) ? strlen(s->generic.name) : 0;  // calculate bounds
  s->generic.left   = s->generic.x - (len + 1) * SMALLCHAR_WIDTH;
  s->generic.right  = s->generic.x + (SLIDER_RANGE + 2 + 1) * SMALLCHAR_WIDTH;
  s->generic.top    = s->generic.y;
  s->generic.bottom = s->generic.y + SMALLCHAR_HEIGHT;
}

//:::::::::::::::::
// Slider_Key
//:::::::::::::::::
sfxHandle_t slider_key(MenuSlider* s, int key) {
  sfxHandle_t sound;
  int         x;
  int         oldvalue;

  switch (key) {
    case K_MOUSE1:
      x           = uis.cursorx - s->generic.x - 2 * SMALLCHAR_WIDTH;
      oldvalue    = s->curvalue;
      s->curvalue = (x / (float)(SLIDER_RANGE * SMALLCHAR_WIDTH)) * (s->maxvalue - s->minvalue) + s->minvalue;
      if (s->curvalue < s->minvalue) s->curvalue = s->minvalue;
      else if (s->curvalue > s->maxvalue) s->curvalue = s->maxvalue;
      sound = (s->curvalue != oldvalue) ? q3sound.menu_move : 0;
      break;
    case K_KP_LEFTARROW:
    case K_LEFTARROW:
      if (s->curvalue > s->minvalue) { s->curvalue--; }
      sound = (s->curvalue > s->minvalue) ? q3sound.menu_move : q3sound.menu_buzz;
      break;
    case K_KP_RIGHTARROW:
    case K_RIGHTARROW:
      if (s->curvalue < s->maxvalue) { s->curvalue++; }
      sound = (s->curvalue < s->maxvalue) ? q3sound.menu_move : q3sound.menu_buzz;
      break;
    default: sound = 0; break;  // key not handled
  }
  if (sound && s->generic.callback) { s->generic.callback(s, MS_ACTIVATED); }
  return (sound);
}

//:::::::::::::::::
// Slider_Draw
//   sk->note There was another version of the slider, behind an #if 0
//:::::::::::::::::
void slider_draw(MenuSlider* s) {
  int    x      = s->generic.x;
  int    y      = s->generic.y;
  bool   focus  = (s->generic.parent->cursor == s->generic.menuPosition);
  bool   grayed = (s->generic.flags & MFL_GRAYED);
  float* color  = (grayed) ? (vec_t*)q3color.text_disabled : (vec_t*)q3color.text_normal;
  int    style  = UI_SMALLFONT;
  if (focus) {
    color = (vec_t*)q3color.text_highlight;
    style |= UI_PULSE;
  }
  // draw label
  uiDrawString(x - SMALLCHAR_WIDTH, y, s->generic.name, UI_RIGHT | style, color);
  // draw slider
  uiSetColor(color);
  uiDrawHandlePic(x + SMALLCHAR_WIDTH, y, 96, 16, sliderBar);
  uiSetColor(NULL);
  // clamp thumb
  if (s->maxvalue > s->minvalue) {
    s->range = Com_Clamp(0, 1, (s->curvalue - s->minvalue) / (float)(s->maxvalue - s->minvalue));
  } else {
    s->range = 0;
  }
  // draw thumb
  int button = (style & UI_PULSE) ? sliderButton_1 : sliderButton_0;
  uiDrawHandlePic((int)(x + 2 * SMALLCHAR_WIDTH + (SLIDER_RANGE - 1) * SMALLCHAR_WIDTH * s->range) - 2, y - 2, 12, 20, button);
}
