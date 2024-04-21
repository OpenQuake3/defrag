#include "../../local.h"
#include "../tools.h"

//:::::::::::::::::::::::
// PAnything = Proportional
//   PString : ProportionalString
//   PSize   : ProportionalSize
//:::::::::::::::::::::::

//:::::::::::::::::::::::
// uiDrawBannerString2
//:::::::::::::::::::::::
static void uiDrawBannerString2(int x, int y, const char* str, vec4_t color) {
  // draw the colored text
  id3R_SetColor(color);
  // Init variables
  float ax = x * uis.xscale + uis.bias;
  float ay = y * uis.yscale;
  float frow;
  float fcol;
  float fwidth;
  float fheight;
  float aw;
  float ah;
  byte  ch;

  const char* s = str;
  while (*s) {
    ch = *s & 127;
    if (ch == ' ') {
      ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH) * uis.xscale;
    } else if (ch >= 'A' && ch <= 'Z') {
      ch -= 'A';
      fcol    = (float)propMapB[ch][0] / 256.0f;
      frow    = (float)propMapB[ch][1] / 256.0f;
      fwidth  = (float)propMapB[ch][2] / 256.0f;
      fheight = (float)PROPB_HEIGHT / 256.0f;
      aw      = (float)propMapB[ch][2] * uis.xscale;
      ah      = (float)PROPB_HEIGHT * uis.yscale;
      id3R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, uis.charsetPropB);
      ax += (aw + (float)PROPB_GAP_WIDTH * uis.xscale);
    }
    s++;
  }
  id3R_SetColor(NULL);
}

//:::::::::::::::::::::::
// uiDrawBannerString
//:::::::::::::::::::::::
void uiDrawBannerString(int x, int y, const char* str, int style, vec4_t color) {
  int         ch;
  int         width = 0;
  const char* s     = str;
  // find the width of the drawn text
  while (*s) {
    ch = *s;
    if (ch == ' ') {
      width += PROPB_SPACE_WIDTH;
    } else if (ch >= 'A' && ch <= 'Z') {
      width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
    }
    s++;
  }
  width -= PROPB_GAP_WIDTH;
  switch (style & UI_FORMATMASK) {
    case UI_CENTER: x -= width / 2; break;
    case UI_RIGHT: x -= width; break;
    case UI_LEFT:
    default: break;
  }
  if (style & UI_DROPSHADOW) {
    vec4_t drawcolor;
    Vector4Set(drawcolor, 0, 0, 0, color[3]);
    uiDrawBannerString2(x + 2, y + 2, str, drawcolor);
  }
  uiDrawBannerString2(x, y, str, color);
}

//:::::::::::::::::::::::
// uiDrawPString2
//:::::::::::::::::::::::
static void uiDrawPString2(int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset) {
  id3R_SetColor(color);  // draw the colored text

  float ax = x * uis.xscale + uis.bias;
  float ay = y * uis.yscale;
  float aw = 0;
  float ah;
  byte  ch;
  float fcol;
  float frow;
  float fwidth;
  float fheight;

  const char* s = str;
  while (*s) {
    ch = *s & 127;
    if (ch == ' ') {
      aw = (float)PROP_SPACE_WIDTH * uis.xscale * sizeScale;
    } else if (propMap[ch][2] != -1) {
      fcol    = (float)propMap[ch][0] / 256.0f;
      frow    = (float)propMap[ch][1] / 256.0f;
      fwidth  = (float)propMap[ch][2] / 256.0f;
      fheight = (float)PROP_HEIGHT / 256.0f;
      aw      = (float)propMap[ch][2] * uis.xscale * sizeScale;
      ah      = (float)PROP_HEIGHT * uis.yscale * sizeScale;
      id3R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + fwidth, frow + fheight, charset);
    }
    ax += (aw + (float)PROP_GAP_WIDTH * uis.xscale * sizeScale);
    s++;
  }
  id3R_SetColor(NULL);
}
//:::::::::::::::::::::::
void uiDrawPString(int x, int y, const char* str, int style, vec4_t color) {
  if (!str) { return; }
  float sizeScale = uiPSizeScale(style);

  int width;
  switch (style & UI_FORMATMASK) {
    case UI_CENTER:
      width = uiPStringWidth(str) * sizeScale;
      x -= width / 2;
      break;
    case UI_RIGHT:
      width = uiPStringWidth(str) * sizeScale;
      x -= width;
      break;
    case UI_LEFT:
    default: break;
  }

  vec4_t drawcolor;
  if (style & UI_DROPSHADOW) {
    drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
    drawcolor[3]                               = color[3];
    uiDrawPString2(x + 2, y + 2, str, drawcolor, sizeScale, uis.charsetProp);
  }

  if (style & UI_INVERSE) {
    drawcolor[0] = color[0] * 0.7;
    drawcolor[1] = color[1] * 0.7;
    drawcolor[2] = color[2] * 0.7;
    drawcolor[3] = color[3];
    uiDrawPString2(x, y, str, drawcolor, sizeScale, uis.charsetProp);
    return;
  }

  if (style & UI_PULSE) {
    drawcolor[0] = color[0] * 0.7;
    drawcolor[1] = color[1] * 0.7;
    drawcolor[2] = color[2] * 0.7;
    drawcolor[3] = color[3];
    uiDrawPString2(x, y, str, color, sizeScale, uis.charsetProp);

    drawcolor[0] = color[0];
    drawcolor[1] = color[1];
    drawcolor[2] = color[2];
    drawcolor[3] = 0.5 + 0.5 * sin((double)uis.realtime / (double)PULSE_DIVISOR);
    uiDrawPString2(x, y, str, drawcolor, sizeScale, uis.charsetPropGlow);
    return;
  }
  uiDrawPString2(x, y, str, color, sizeScale, uis.charsetProp);
}
//:::::::::::::::::::::::
void uiDrawPString_AutoWrap(int x, int y, int xmax, int ystep, const char* str, int style, vec4_t color) {
  if (!str || str[0] == '\0') { return; }  // Return if there is nothing to draw
  // Get proportional scale
  float sizeScale = uiPSizeScale(style);
  // Init the buffer
  char  buf[1024];
  char *s1, *s2, *s3;
  Q_strncpyz(buf, str, sizeof(buf));
  s1 = s2 = s3 = buf;
  // Draw it
  int  width;
  char c_bcp;
  while (1) {
    do { s3++; } while (*s3 != ' ' && *s3 != '\0');
    c_bcp = *s3;
    *s3   = '\0';
    width = uiPStringWidth(s1) * sizeScale;
    *s3   = c_bcp;
    if (width > xmax) {
      if (s1 == s2) { s2 = s3; }  // don't have a clean cut, will overflow
      *s2 = '\0';
      uiDrawPString(x, y, s1, style, color);
      y += ystep;
      if (c_bcp == '\0') {
        // that was the last word. we could start a new loop, but that wouldn't be much use
        // even if the word is too long, we would overflow it (see above), so just print it now if needed
        s2++;
        if (*s2 != '\0') { uiDrawPString(x, y, s2, style, color); }  // if we are printing an overflowing line we have s2 == s3
        break;
      }
      s2++;
      s1 = s2;
      s3 = s2;
    } else {
      s2 = s3;
      if (c_bcp == '\0') {  // we reached the end
        uiDrawPString(x, y, s1, style, color);
        break;
      }
    }
  }
}

//:::::::::::::::::::
// uiDrawNamedPic
//:::::::::::::::::::
void uiDrawNamedPic(float x, float y, float width, float height, const char* picname) {
  qhandle_t hShader = id3R_RegisterShaderNoMip(picname);
  uiAdjustFrom640(&x, &y, &width, &height);
  id3R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

//:::::::::::::::::::
// uiDrawHandlePic
//:::::::::::::::::::
void uiDrawHandlePic(float x, float y, float w, float h, qhandle_t hShader) {
  bool flipV = (w < 0);  // flip about vertical
  if (w < 0) { w = -w; }
  float s0   = (flipV) ? 1 : 0;
  float s1   = (flipV) ? 0 : 1;

  bool flipH = (h < 0);  // flip about horizontal
  if (flipH) { h = -h; }
  float t0 = (flipH) ? 1 : 0;
  float t1 = (flipH) ? 0 : 1;

  uiAdjustFrom640(&x, &y, &w, &h);
  id3R_DrawStretchPic(x, y, w, h, s0, t0, s1, t1, hShader);
}

//:::::::::::::::::::
// uiFillRect
//   Coordinates are 640*480 virtual values
//:::::::::::::::::::
void uiFillRect(float x, float y, float width, float height, const float* color) {
  id3R_SetColor(color);
  uiAdjustFrom640(&x, &y, &width, &height);
  id3R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, uis.whiteShader);
  id3R_SetColor(NULL);
}

static void drawString2(int x, int y, const char* str, vec4_t color, int charw, int charh) {
  if (y < -charh) { return; }  // offscreen
  // Draw the colored text
  id3R_SetColor(color);
  // Set the scale
  float ax               = x * uis.xscale + uis.bias;
  float ay               = y * uis.yscale;
  float aw               = charw * uis.xscale;
  float ah               = charh * uis.yscale;
  // Set the color
  int         forceColor = false;  // APSFIXME;
  vec4_t      tempcolor;
  const char* s = str;
  while (*s) {
    if (Q_IsColorString(s)) {
      if (!forceColor) {
        memcpy(tempcolor, g_color_table[ColorIndex(s[1])], sizeof(tempcolor));
        tempcolor[3] = color[3];
        id3R_SetColor(tempcolor);
      }
      s += 2;
      continue;
    }
    float frow;
    float fcol;
    char  ch = *s & 255;
    if (ch != ' ') {
      frow = (ch >> 4) * 0.0625;
      fcol = (ch & 15) * 0.0625;
      id3R_DrawStretchPic(ax, ay, aw, ah, fcol, frow, fcol + 0.0625, frow + 0.0625, uis.charset);
    }
    ax += aw;
    s++;
  }
  id3R_SetColor(NULL);
}

//:::::::::::::::::::
// uiDrawString
//:::::::::::::::::::
void uiDrawString(int x, int y, const char* str, int style, vec4_t color) {
  if (!str) { return; }
  if ((style & UI_BLINK) && ((uis.realtime / BLINK_DIVISOR) & 1)) { return; }

  int charw;
  int charh;
  if (style & UI_SMALLFONT) {
    charw = SMALLCHAR_WIDTH;
    charh = SMALLCHAR_HEIGHT;
  } else if (style & UI_GIANTFONT) {
    charw = GIANTCHAR_WIDTH;
    charh = GIANTCHAR_HEIGHT;
  } else {
    charw = BIGCHAR_WIDTH;
    charh = BIGCHAR_HEIGHT;
  }

  vec4_t newcolor;
  float* drawcolor;
  vec4_t lowlight;
  if (style & UI_PULSE) {
    lowlight[0] = 0.8 * color[0];
    lowlight[1] = 0.8 * color[1];
    lowlight[2] = 0.8 * color[2];
    lowlight[3] = 0.8 * color[3];
    uiLerpColor(color, lowlight, newcolor, 0.5 + 0.5 * sin((double)uis.realtime / PULSE_DIVISOR));
    drawcolor = newcolor;
  } else drawcolor = color;

  int len;
  switch (style & UI_FORMATMASK) {
    case UI_CENTER:  // center justify at x
      len = strlen(str);
      x   = x - len * charw / 2;
      break;
    case UI_RIGHT:  // right justify at x
      len = strlen(str);
      x   = x - len * charw;
      break;
    default: break;  // left justify at x
  }

  vec4_t dropcolor;
  if (style & UI_DROPSHADOW) {
    dropcolor[0] = dropcolor[1] = dropcolor[2] = 0;
    dropcolor[3]                               = drawcolor[3];
    drawString2(x + 2, y + 2, str, dropcolor, charw, charh);
  }
  drawString2(x, y, str, drawcolor, charw, charh);
}

//:::::::::::::::::::
// uiDrawChar
//:::::::::::::::::::
void uiDrawChar(int x, int y, int ch, int style, vec4_t color) {
  char buff[2];
  buff[0] = ch;
  buff[1] = '\0';
  uiDrawString(x, y, buff, style, color);
}

//:::::::::::::::::::::::
// uiDrawMenu
//   Draws all items of a MenuFramework object
//:::::::::::::::::::::::
void uiDrawMenu(MenuFw* menu) {
  MenuCommon* itemptr;
  // draw menu
  for (int i = 0; i < menu->nitems; i++) {
    itemptr = (MenuCommon*)menu->items[i];
    if (itemptr->flags & MFL_HIDDEN) { continue; }
    if (itemptr->ownerdraw) {
      itemptr->ownerdraw(itemptr);  // total subclassing, owner draws everything
    } else {
      switch (itemptr->type) {
        case MITEM_RADIOBUTTON: radioBtn_draw((MenuRadioBtn*)itemptr); break;
        case MITEM_FIELD: menuField_draw((MenuField*)itemptr); break;
        case MITEM_SLIDER: slider_draw((MenuSlider*)itemptr); break;
        case MITEM_SPINCONTROL:
          spinControl_draw((MenuList*)itemptr);
          break;
          /*
          case MITEM_ACTION: drawAction((MenuAction*)itemptr); break;
          case MITEM_BITMAP: drawBitmap((MenuBitmap*)itemptr); break;
          case MITEM_TEXT: drawText((MenuText*)itemptr); break;
          case MITEM_SCROLLLIST: drawScrollList((MenuList*)itemptr); break;
          case MITEM_PTEXT: drawPText((MenuText*)itemptr); break;
          case MITEM_BTEXT: drawBText((MenuText*)itemptr); break;
          default: id3Error(va("Menu_Draw: unknown type %d", itemptr->type));
         */
      }
    }
    /*
#ifndef NDEBUG
    if (uis.debug) {
      if (!(itemptr->flags & MFL_INACTIVE)) {
        int x = itemptr->left;
        int y = itemptr->top;
        int w = itemptr->right - itemptr->left + 1;
        int h = itemptr->bottom - itemptr->top + 1;
        if (itemptr->flags & MFL_HASMOUSEFOCUS) {
          drawRect(x, y, w, h, colorYellow);
        } else {
          drawRect(x, y, w, h, colorWhite);
        }
      }
    }
#endif
  */
  }
  /*
  itemptr = cursorGetItem(menu);
  if (itemptr && itemptr->statusbar) itemptr->statusbar((void*)itemptr);
  */
}
