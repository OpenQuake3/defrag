#include "../elements.h"

//:::::::::::::::::
// Bitmap_Init
//:::::::::::::::::
void menuImage_init(MenuImage* i) {
  float x = i->generic.x;
  float y = i->generic.y;
  float w = (float)i->width / GL_W;
  float h = (float)i->height / GL_H;
  if (w < 0) { w = -w; }
  if (h < 0) { h = -h; }
  if (i->generic.flags & MFL_RIGHT_JUSTIFY) { x = x - w; }
  if (i->generic.flags & MFL_CENTER_JUSTIFY) { x = x - w / 2; }
  i->generic.left   = x;
  i->generic.right  = x + w;
  i->generic.top    = y;
  i->generic.bottom = y + h;
  // i->shader         = 0;
  // i->focusshader    = 0;
}

//:::::::::::::::::
// Bitmap_Draw
//:::::::::::::::::
void menuImage_draw(MenuImage* i) {
  int x = i->generic.x * GL_W;
  int y = i->generic.y * GL_H;
  int w = i->width;
  int h = i->height;
  if (i->generic.flags & MFL_RIGHT_JUSTIFY) { x = x - w; }
  if (i->generic.flags & MFL_CENTER_JUSTIFY) { x = x - w / 2; }
  if (i->focuspic && !i->shader) {  // used to refresh shader
    i->shader = (i->errorpic) ? id3R_RegisterShaderNoMip(i->errorpic) : id3R_RegisterShaderNoMip(i->focuspic);
  }
  if (i->focuspic && !i->focusshader) { i->focusshader = id3R_RegisterShaderNoMip(i->focuspic); }
  float* color;
  vec4_t tempcolor;
  if (i->generic.flags & MFL_GRAYED && i->shader) {
    id3R_SetColor(colorMdGrey);
    uiDrawHandlePicPix(x, y, w, h, i->shader);
    id3R_SetColor(NULL);
  } else {
    if (i->shader) { uiDrawHandlePicPix(x, y, w, h, i->shader); }
    if (((i->generic.flags & MFL_PULSE) || (i->generic.flags & MFL_PULSEIFFOCUS)) && (cursorGetItem(i->generic.parent) == i)) {
      if (i->focuscolor) {
        tempcolor[0] = i->focuscolor[0];
        tempcolor[1] = i->focuscolor[1];
        tempcolor[2] = i->focuscolor[2];
        color        = tempcolor;
      } else {
        color = (vec_t*)q3color.pulse;
      }
      color[3] = 0.5 + 0.5 * sin((double)uis.realtime / PULSE_DIVISOR);
      id3R_SetColor(color);
      uiDrawHandlePicPix(x, y, w, h, i->focusshader);
      id3R_SetColor(NULL);
    } else if ((i->generic.flags & MFL_HIGHLIGHT) || ((i->generic.flags & MFL_HIGHLIGHT_IF_FOCUS) && (cursorGetItem(i->generic.parent) == i))) {
      if (i->focuscolor) {
        id3R_SetColor(i->focuscolor);
        uiDrawHandlePicPix(x, y, w, h, i->focusshader);
        id3R_SetColor(NULL);
      } else {
        uiDrawHandlePicPix(x, y, w, h, i->focusshader);
      }
    }
  }
}
