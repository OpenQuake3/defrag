#include "../elements.h"


//:::::::::::::::::::::::
// PText_Init
//   Initalizes the bounds of the item
//:::::::::::::::::::::::
void menuText_init(MenuText* t) {
  float scale = fontScale(&t->font);  // uiPSizeScale(t->style);  // TODO: Make this possible with fonts
  float x     = t->generic.x;
  float y     = t->generic.y;
  float w     = uiTextGetWidth(t->string, &t->font, scale, strlen(t->string));   // uiPStringWidth(t->string) * sizeScale;
  float h     = uiTextGetHeight(t->string, &t->font, scale, strlen(t->string));  // PROP_HEIGHT * sizeScale;
  // Correct alignment. X starts at the leftmost point
  switch (t->align) {
    // TODO: Add vertical alignment
    case TEXT_ALIGN_LEFT   : break;  // x = x
    case TEXT_ALIGN_RIGHT  : x -= w; break;
    case TEXT_ALIGN_CENTER : x -= (w / 2); break;
  }
  // Find each of the item bounds
  // Text renders upward from y (glyph drawn at y - glyph->top), so bounds extend above y
  t->generic.left  = x;      // - PROP_GAP_WIDTH * sizeScale;
  t->generic.right = x + w;  // + PROP_GAP_WIDTH * sizeScale;
  t->generic.top   = y - h;
  if (Q_stristr(t->font.name, "heyNovember")) {  // hack: heyNovember font incorrect alignment (what is this font doing?)
    h -= 0.1 * uiTextGetHeight(t->string, &uis.font.actionKey, fontScale(&uis.font.actionKey), strlen(t->string));
  }
  t->generic.bottom = y;
}

//:::::::::::::::::
// PText_Draw
//:::::::::::::::::
void menuText_draw(MenuText* t) {
  float  x      = t->generic.x;
  float  y      = t->generic.y;
  bool   grayed = (t->generic.flags & MFL_GRAYED);
  float* color  = (grayed) ? (vec_t*)q3color.text_disabled : t->color;
  int    style  = t->style;
  if (t->generic.flags & MFL_PULSEIFFOCUS) { style |= (cursorGetItem(t->generic.parent) == t) ? UI_PULSE : UI_INACTIVE; }
  if (Q_stristr(t->font.name, "heyNovember")) {  // hack: heyNovember font incorrect alignment (what is this font doing?)
    y += 0.7 * uiTextGetHeight(t->string, &uis.font.actionKey, fontScale(&uis.font.actionKey), strlen(t->string));
  }
  uiTextDraw(t->string, &t->font, x, y, fontScale(&t->font), color, 0, style, strlen(t->string), t->align);
}

//:::::::::::::::::::::::
// Text_
// Is this even needed at all?
// The only difference seems to be that it concatenates the lable with the value
//:::::::::::::::::::::::
// Text_Init
//:::::::::::::::::::::::
void OText_init(MenuText* t) { t->generic.flags |= MFL_INACTIVE; }

//:::::::::::::::::::::::
// Text_Draw
//:::::::::::::::::::::::
void OText_draw(MenuText* t) {
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



// Banner text is a duplicate of PText, not clickable and with a different font atlas
//:::::::::::::::::::::::
// BText_Init
//:::::::::::::::::::::::
// void BText_init(MenuText* t) { t->generic.flags |= MFL_INACTIVE; }

//:::::::::::::::::::::::
// BText_Draw
//:::::::::::::::::::::::
// void BText_draw(MenuText* t) {
//   int    x      = t->generic.x;
//   int    y      = t->generic.y;
//   bool   grayed = (t->generic.flags & MFL_GRAYED);
//   float* color  = (grayed) ? (vec_t*)q3color.text_disabled : t->color;
//   uiDrawBannerString(x, y, t->string, t->style, color);
// }

