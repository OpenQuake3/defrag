#include "../../local.h"
#include "../tools.h"

//::::::::::::::::
// TODO: Move out to a header accesible by the rest of the code
#define TEXT_STYLE_SHADOW 3   // Drop shadow
#define TEXT_STYLE_SHADOW2 6  // Drop shadow, twice the size
//.........................................
// uiTextGetWidthPix
//   Returns (in pixels) the total width of the given text, when using the selected font
//.........................................
int uiTextGetWidthPix(const char* text, fontInfo_t* font, float scale, int maxLength) {
  float useScale = scale * font->glyphScale;
  float out      = 0;
  if (text) {
    int len = strlen(text);
    if (maxLength > 0 && len > maxLength) { len = maxLength; }
    int         count = 0;
    const char* s     = text;
    while (s && *s && count < len) {
      if (Q_IsColorString(s)) {
        s += 2;
        continue;
      } else {
        glyphInfo_t* glyph = &font->glyphs[*s & 255];
        out += glyph->xSkip;
        s++;
        count++;
      }
    }
  }
  return out * useScale;
}
//.........................................
// uiTextGetWidth
//   Returns (in screen percentage) the max height of the given text, when using the selected font
//.........................................
float uiTextGetWidth(const char* text, fontInfo_t* font, float scale, int maxLength) { return (float)uiTextGetWidthPix(text, font, scale, maxLength) / GL_W; }

//.........................................
// uiTextGetHeightPix
//   Returns (in pixels) the max height of the given text, when using the selected font
//.........................................
int uiTextGetHeightPix(const char* text, fontInfo_t* font, float scale, int maxLength) {
  float useScale = scale * font->glyphScale;
  float max      = 0;
  if (text) {
    int len = strlen(text);
    if (maxLength > 0 && len > maxLength) { len = maxLength; }
    int         count = 0;
    const char* s     = text;
    while (s && *s && count < len) {
      if (Q_IsColorString(s)) {
        s += 2;
        continue;
      } else {
        glyphInfo_t* glyph = &font->glyphs[*s & 255];
        if (max < glyph->height) { max = glyph->height; }
        s++;
        count++;
      }
    }
  }
  return max * useScale;
}
//.........................................
// uiTextGetHeight
//   Returns (in screen percentage) the max height of the given text, when using the selected font
//.........................................
float uiTextGetHeight(const char* text, fontInfo_t* font, float scale, int maxLength) { return (float)uiTextGetHeightPix(text, font, scale, maxLength) / GL_H; }

//.........................................
// uiDrawTextChar
//   Draws a character in the given shader
//   Coordinates are expected in screen size. No adjusting is done
//.........................................
static void uiTextDrawChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t glyphShader) {
  float w = width * scale;
  float h = height * scale;
  id3R_DrawStretchPic(x, y, w, h, s, t, s2, t2, glyphShader);
}
//.........................................
// uiDrawTextStr
//   Draws a text string with the given properties
//   Coordinates are expected in screen size. No adjusting is done
//.........................................
void uiTextDrawStr(const char* text, fontInfo_t* font, float x, float y, float scale, vec4_t color, float adjust, int style, int maxLength) {
  // if (scale <= hud_fontSmall.value)   { font = &uis.media.fontSmall; }
  // else if (scale > hud_fontBig.value) { font = &uis.media.fontBig; }
  float useScale = scale * font->glyphScale;
  if (text) {
    const char* s = text;
    id3R_SetColor(color);
    vec4_t newColor;
    memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    int len = strlen(text);
    if (maxLength > 0 && len > maxLength) { len = maxLength; }
    int count = 0;
    while (s && *s && count < len) {
      glyphInfo_t* glyph = &font->glyphs[*s & 255];
      // int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      // float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
      if (Q_IsColorString(s)) {
        memcpy(newColor, g_color_table[ColorIndex(*(s + 1))], sizeof(newColor));
        newColor[3] = color[3];
        id3R_SetColor(newColor);
        s += 2;
        continue;
      } else {
        float yadj = useScale * glyph->top;
        if (style == TEXT_STYLE_SHADOW || style == TEXT_STYLE_SHADOW2) {
          int ofs       = style == TEXT_STYLE_SHADOW2 ? 2 : 1;
          colorBlack[3] = newColor[3];
          id3R_SetColor(colorBlack);
          uiTextDrawChar(x + ofs, y - yadj + ofs, glyph->imageWidth, glyph->imageHeight, useScale, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
          colorBlack[3] = 1.0;
          id3R_SetColor(newColor);
        }
        uiTextDrawChar(x, y - yadj, glyph->imageWidth, glyph->imageHeight, useScale, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
        x += (glyph->xSkip * useScale) + adjust;
        s++;
        count++;
      }
    }
    id3R_SetColor(NULL);
  }
}
//.........................................
// uiTextDrawNoStyle
//   Pre-styling version
//.........................................
static void uiTextDrawNoStyle(const char* text, fontInfo_t* font, float x, float y, float scale, vec4_t color, float adjust, int style, int maxLength, int align) {
  // Convert X-Y from [0-1] to screen resolution
  x *= GL_W;
  y *= GL_H;

  int w = uiTextGetWidth(text, font, scale, maxLength);
  if (!align) { align = TEXT_ALIGN_DEFAULT; }
  switch (align) {
    case TEXT_ALIGN_LEFT: x -= 0; break;
    case TEXT_ALIGN_CENTER: x -= (w * 0.5); break;
    case TEXT_ALIGN_RIGHT: x -= w; break;
    default: break;
  }
  uiTextDrawStr(text, font, x, y, scale, color, adjust, style, maxLength);
}

//.........................................
// uiTextDraw
//   Draws text to screen using the given font properties
//   Expects X, Y in [0-1] percentage range
//   TODO: Add selection highlight box
//.........................................
void uiTextDraw(const char* text, fontInfo_t* font, float x, float y, float scale, vec4_t color, float adjust, int style, int maxLength, int align) {
  if (!text) { return; }

  // Convert X-Y from [0-1] to screen resolution
  x *= GL_W;
  y *= GL_H;
  int   shadowOffs = 2;    // Number of pixels to offset the drop shadow
  float inactivePc = 0.8;  // Color will be this percentage of the original input, when the item is inactive

  int w = uiTextGetWidthPix(text, font, scale, maxLength);
  if (!align) { align = TEXT_ALIGN_DEFAULT; }
  switch (align) {
    case TEXT_ALIGN_LEFT: x -= 0; break;
    case TEXT_ALIGN_CENTER: x -= (w * 0.5); break;
    case TEXT_ALIGN_RIGHT: x -= w; break;
    default: break;
  }

  vec4_t drawcolor;
  if (style & UI_DROPSHADOW) {
    Vector4Set(drawcolor, 0, 0, 0, color[3]);
    uiTextDrawStr(text, font, x+shadowOffs, y+shadowOffs, scale, drawcolor, adjust, style, maxLength);
  }

  if (style & UI_INACTIVE) {  // Inactive item
    // Draw at inactivePc percentage of the input color
    Vector4Set(drawcolor, color[0]*inactivePc, color[1]*inactivePc, color[2]*inactivePc, color[3]);
    uiTextDrawStr(text, font, x+shadowOffs, y+shadowOffs, scale, drawcolor, adjust, style, maxLength);
    return;
  }

  if (style & UI_PULSE) {  // Active item (pulsating)
    // Draw lower layer at inactivePc of the input color
    Vector4Set(drawcolor, color[0]*inactivePc, color[1]*inactivePc, color[2]*inactivePc, color[3]);
    uiTextDrawStr(text, font, x, y, scale, drawcolor, adjust, style, maxLength);

    // Draw top layer as Pulse between inactive color alpha, and full color alpha, with sin of realtime/divisor
    Vector4Set(drawcolor, color[0], color[1], color[2], inactivePc + (1-inactivePc) * sin((double)uis.realtime / PULSE_DIVISOR));
    uiTextDrawStr(text, font, x, y, scale, drawcolor, adjust, style, maxLength);
    return;
  }
  // Else draw text normally
  uiTextDrawStr(text, font, x, y, scale, color, adjust, style, maxLength);
}


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

  switch (style & UI_FORMATMASK) {
    case UI_CENTER: x -= (uiPStringWidth(str) * sizeScale) / 2; break;
    case UI_RIGHT: x -= (uiPStringWidth(str) * sizeScale); break;
    case UI_LEFT:
    default: break;
  }

  vec4_t drawcolor;
  if (style & UI_DROPSHADOW) {
    Vector4Set(drawcolor, 0, 0, 0, color[3]);
    uiDrawPString2(x + 2, y + 2, str, drawcolor, sizeScale, uis.charsetProp);
  }

  if (style & UI_INACTIVE) {
    Vector4Set(drawcolor, color[0] * 0.7, color[1] * 0.7, color[2] * 0.7, color[3]);
    uiDrawPString2(x, y, str, drawcolor, sizeScale, uis.charsetProp);
    return;
  }

  if (style & UI_PULSE) {
    Vector4Set(drawcolor, color[0] * 0.7, color[1] * 0.7, color[2] * 0.7, color[3]);
    uiDrawPString2(x, y, str, color, sizeScale, uis.charsetProp);

    Vector4Set(drawcolor, color[0], color[1], color[2], 0.5 + 0.5 * sin((double)uis.realtime / (double)PULSE_DIVISOR));
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
// uiDrawHandlePicPix
//:::::::::::::::::::
void uiDrawHandlePicPix(float x, float y, float w, float h, qhandle_t hShader) {
  bool flipV = (w < 0);  // flip about vertical
  if (w < 0) { w = -w; }
  float s0   = (flipV) ? 1 : 0;
  float s1   = (flipV) ? 0 : 1;

  bool flipH = (h < 0);  // flip about horizontal
  if (flipH) { h = -h; }
  float t0 = (flipH) ? 1 : 0;
  float t1 = (flipH) ? 0 : 1;

  id3R_DrawStretchPic(x, y, w, h, s0, t0, s1, t1, hShader);
}
void uiDrawHandlePic(float x, float y, float w, float h, qhandle_t hShader) {
  x *= GL_W;
  y *= GL_H;
  // uiAdjustFrom640(&x, &y, &w, &h);
  uiDrawHandlePicPix(x, y, w, h, hShader);
}

//:::::::::::::::::::
// uiFillRecPixt
//   Draw a rectangle. Measures are in pixels
//:::::::::::::::::::
void uiFillRectPix(float x, float y, float width, float height, const float* color) {
  id3R_SetColor(color);
  // uiAdjustFrom640(&x, &y, &width, &height);
  id3R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, uis.whiteShader);
  id3R_SetColor(NULL);
}
//:::::::::::::::::::
// uiFillRecPixt
//   Draw a rectangle. Measures are in screen percentages
//:::::::::::::::::::
void uiFillRect(float x, float y, float width, float height, const float* color) {
  x *= GL_W;
  y *= GL_H;
  uiFillRectPix(x, y, width, height, color);
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
        case MITEM_SWITCH: menuSwitch_draw((MenuSwitch*)itemptr); break;
        case MITEM_FIELD: menuField_draw((MenuField*)itemptr); break;
        case MITEM_SLIDER: menuSlider_draw((MenuSlider*)itemptr); break;
        case MITEM_MULTIOPT: menuMOpt_draw((MenuList*)itemptr); break;
        case MITEM_ACTION: menuAction_draw((MenuAction*)itemptr); break;
        case MITEM_IMAGE: menuImage_draw((MenuImage*)itemptr); break;
        case MITEM_LTEXT: OText_draw((MenuText*)itemptr); break;
        case MITEM_LIST: menuList_draw((MenuList*)itemptr); break;
        case MITEM_TEXT: menuText_draw((MenuText*)itemptr); break;
        // case MITEM_BTEXT: BText_draw((MenuText*)itemptr); break;
        default: id3Error(va("%s: unknown type %d", __func__, itemptr->type));
      }
    }
#ifndef NDEBUG  // TODO:
    if (uis.debug) {
    /*
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
  */
    }
#endif
  }
  itemptr = cursorGetItem(menu);
  if (itemptr && itemptr->statusbar) itemptr->statusbar((void*)itemptr);
}
