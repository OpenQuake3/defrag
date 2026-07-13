#ifndef UI_FW_TOOLS_H
#define UI_FW_TOOLS_H
//:::::::::::::::::

#include "../shared.h"
#include "../callbacks.h"

//:::::::::::::::::
extern uiStatic_t uis;
//:::::::::::::::::

//:::::::::::::::::
// General Framework Tools
//:::::::::::::::::
// framework/draw.c
int   uiPStringWidth(const char*);
void  uiDrawPString(int, int, const char*, int, vec4_t);
void  uiDrawPString_AutoWrap(int, int, int, int, const char*, int, vec4_t);
void  uiFillRect(float, float, float, float, const float*);
void  uiDrawChar(int, int, int, int, vec4_t);
void  uiDrawString(int, int, const char*, int, vec4_t);
void  uiDrawNamedPic(float, float, float, float, const char*);
void  uiDrawHandlePicPix(float, float, float, float, qhandle_t);
void  uiDrawHandlePic(float, float, float, float, qhandle_t);
void  uiDrawBannerString(int, int, const char*, int, vec4_t);
//...................
float uiTextGetWidth(const char*, fontInfo_t*, float, int);
int   uiTextGetWidthPix(const char*, fontInfo_t*, float, int);
float uiTextGetHeight(const char*, fontInfo_t*, float, int);
int   uiTextGetHeightPix(const char*, fontInfo_t*, float, int);
void  uiTextDrawStr(const char*, fontInfo_t*, float, float, float, vec4_t, float, int, int);
void  uiTextDraw(const char*, fontInfo_t*, float, float, float, vec4_t, float, int, int, int);
//:::::::::::::::::
// framework/utils.c  Config
#define PROPB_GAP_WIDTH 4
#define PROPB_SPACE_WIDTH 12
#define PROPB_HEIGHT 36
//:::::::::::::::::
// framework/utils.c  Data
extern int propMap[128][3];
extern int propMapB[26][3];
//:::::::::::::::::
// framework/utils.c  Methods
int   Passf(float);
void  uiAdjustFrom640(float*, float*, float*, float*);
void  uiLerpColor(vec4_t, vec4_t, vec4_t, float);
void  uiSetColor(const float*);
float uiPSizeScale(int);
//:::::::::::::::::

//:::::::::::::::::
#endif//UI_FW_TOOLS_H
