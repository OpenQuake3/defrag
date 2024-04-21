#include "../elements.h"


//:::::::::::::::::::::::
// Text_Init
//:::::::::::::::::::::::
void text_init(MenuText* t) { t->generic.flags |= MFL_INACTIVE; }

//:::::::::::::::::::::::
// Text_Draw
//:::::::::::::::::::::::
void text_draw(MenuText* t) {
  char buff[512];
  buff[0] = '\0';
  int x   = t->generic.x;
  int y   = t->generic.y;
  // possible label
  if (t->generic.name) { strcpy(buff, t->generic.name); }
  // possible value
  if (t->string) { strcat(buff, t->string); }
  bool   grayed = (t->generic.flags & MFL_GRAYED);
  float* color  = (grayed) ? (vec_t*)q3color.text_disabled : t->color;
  uiDrawString(x, y, buff, t->style, color);
}

//:::::::::::::::::::::::
// BText_Init
//:::::::::::::::::::::::
void BText_init(MenuText* t) { t->generic.flags |= MFL_INACTIVE; }

//:::::::::::::::::::::::
// BText_Draw
//:::::::::::::::::::::::
void BText_draw(MenuText* t) {
  int    x      = t->generic.x;
  int    y      = t->generic.y;
  bool   grayed = (t->generic.flags & MFL_GRAYED);
  float* color  = (grayed) ? (vec_t*)q3color.text_disabled : t->color;
  uiDrawBannerString(x, y, t->string, t->style, color);
}

//:::::::::::::::::::::::
// PText_Init
//:::::::::::::::::::::::
void PText_init(MenuText* t) {
  float sizeScale = uiPSizeScale(t->style);
  int   x         = t->generic.x;
  int   y         = t->generic.y;
  int   w         = uiPStringWidth(t->string) * sizeScale;
  int   h         = PROP_HEIGHT * sizeScale;
  if (t->generic.flags & MFL_RIGHT_JUSTIFY) { x -= w; }
  if (t->generic.flags & MFL_CENTER_JUSTIFY) { x -= w / 2; }
  t->generic.left   = x - PROP_GAP_WIDTH * sizeScale;
  t->generic.right  = x + w + PROP_GAP_WIDTH * sizeScale;
  t->generic.top    = y;
  t->generic.bottom = y + h;
}

//:::::::::::::::::
// PText_Draw
//:::::::::::::::::
void PText_draw(MenuText* t) {
  int    x      = t->generic.x;
  int    y      = t->generic.y;
  bool   grayed = (t->generic.flags & MFL_GRAYED);
  float* color  = (grayed) ? (vec_t*)q3color.text_disabled : t->color;
  int    style  = t->style;
  if (t->generic.flags & MFL_PULSEIFFOCUS) { style |= (cursorGetItem(t->generic.parent) == t) ? UI_PULSE : UI_INVERSE; }
  uiDrawPString(x, y, t->string, style, color);
}
