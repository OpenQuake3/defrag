#include "../elements.h"

//:::::::::::::::::
// Bitmap_Init
//:::::::::::::::::
void bitmap_init(MenuBitmap* b) {
  int x = b->generic.x;
  int y = b->generic.y;
  int w = b->width;
  int h = b->height;
  if (w < 0) { w = -w; }
  if (h < 0) { h = -h; }
  if (b->generic.flags & MFL_RIGHT_JUSTIFY) { x = x - w; }
  if (b->generic.flags & MFL_CENTER_JUSTIFY) { x = x - w / 2; }
  b->generic.left   = x;
  b->generic.right  = x + w;
  b->generic.top    = y;
  b->generic.bottom = y + h;
  b->shader         = 0;
  b->focusshader    = 0;
}

//:::::::::::::::::
// Bitmap_Draw
//:::::::::::::::::
void bitmap_draw(MenuBitmap* b) {
  int x = b->generic.x;
  int y = b->generic.y;
  int w = b->width;
  int h = b->height;
  if (b->generic.flags & MFL_RIGHT_JUSTIFY) { x = x - w; }
  if (b->generic.flags & MFL_CENTER_JUSTIFY) { x = x - w / 2; }
  if (b->generic.name && !b->shader) {  // used to refresh shader
    b->shader = (b->errorpic) ? id3R_RegisterShaderNoMip(b->errorpic) : id3R_RegisterShaderNoMip(b->generic.name);
  }
  if (b->focuspic && !b->focusshader) { b->focusshader = id3R_RegisterShaderNoMip(b->focuspic); }
  float* color;
  vec4_t tempcolor;
  if (b->generic.flags & MFL_GRAYED && b->shader) {
    id3R_SetColor(colorMdGrey);
    uiDrawHandlePic(x, y, w, h, b->shader);
    id3R_SetColor(NULL);
  } else {
    if (b->shader) { uiDrawHandlePic(x, y, w, h, b->shader); }
    if (((b->generic.flags & MFL_PULSE) || (b->generic.flags & MFL_PULSEIFFOCUS)) && (cursorGetItem(b->generic.parent) == b)) {
      if (b->focuscolor) {
        tempcolor[0] = b->focuscolor[0];
        tempcolor[1] = b->focuscolor[1];
        tempcolor[2] = b->focuscolor[2];
        color        = tempcolor;
      } else {
        color = (vec_t*)q3color.pulse;
      }
      color[3] = 0.5 + 0.5 * sin((double)uis.realtime / PULSE_DIVISOR);
      id3R_SetColor(color);
      uiDrawHandlePic(x, y, w, h, b->focusshader);
      id3R_SetColor(NULL);
    } else if ((b->generic.flags & MFL_HIGHLIGHT) || ((b->generic.flags & MFL_HIGHLIGHT_IF_FOCUS) && (cursorGetItem(b->generic.parent) == b))) {
      if (b->focuscolor) {
        id3R_SetColor(b->focuscolor);
        uiDrawHandlePic(x, y, w, h, b->focusshader);
        id3R_SetColor(NULL);
      } else {
        uiDrawHandlePic(x, y, w, h, b->focusshader);
      }
    }
  }
}
