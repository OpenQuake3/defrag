#ifndef UI_TYPES_H
#define UI_TYPES_H
//:::::::::::::::::

#include "shared.h"
#include "../rendc/tr_types.h"
#include "config.h"

//:::::::::::::::::
// Menu Data types
typedef struct menuframework_s {
  int   cursor;
  int   cursor_prev;
  int   nitems;
  void* items[MAX_MENUITEMS];
  bool  wrapAround;
  bool  fullscreen;
  bool  showlogo;
  void (*draw)(void);
  sfxHandle_t (*key)(int key);
} MenuFw;
//:::::::::::::::::
typedef struct menucommon_s {
  int          type;
  const char*  name;
  int          id;
  int          x, y;
  int          left;
  int          top;
  int          right;
  int          bottom;
  MenuFw*      parent;
  int          menuPosition;
  unsigned int flags;
  void (*callback)(void* self, int event);
  void (*statusbar)(void* self);
  void (*ownerdraw)(void* self);
} MenuCommon;
//:::::::::::::::::
typedef struct mfield_s {
  int  cursor;
  int  scroll;
  int  widthInChars;
  char buffer[MAX_EDIT_LINE];
  int  maxchars;
} Field;
//:::::::::::::::::
typedef struct menufield_s {
  MenuCommon generic;
  Field      field;
} MenuField;
//:::::::::::::::::
typedef struct menuslider_s {
  MenuCommon generic;
  float      minvalue;
  float      maxvalue;
  float      curvalue;
  float      range;
} MenuSlider;
//:::::::::::::::::
typedef struct menulist_s {
  MenuCommon   generic;
  int          oldvalue;
  int          curvalue;
  int          numitems;
  int          top;
  const char** itemnames;
  int          width;
  int          height;
  int          columns;
  int          separation;
} MenuList;
//:::::::::::::::::
typedef struct menuaction_s {
  MenuCommon generic;
} MenuAction;
//:::::::::::::::::
typedef struct menuradiobutton_s {
  MenuCommon generic;
  int        curvalue;
} MenuRadioBtn;
//:::::::::::::::::
typedef struct menubitmap_s {
  MenuCommon generic;
  char*      focuspic;
  char*      errorpic;
  qhandle_t  shader;
  qhandle_t  focusshader;
  int        width;
  int        height;
  float*     focuscolor;
} MenuBitmap;
//:::::::::::::::::
typedef struct {
  MenuCommon generic;
  char*      string;
  int        style;
  float*     color;
} MenuText;
//:::::::::::::::::

//:::::::::::::::::
typedef struct {
  int        frametime;
  int        realtime;
  int        cursorx;
  int        cursory;
  int        menusp;
  MenuFw*    activemenu;
  MenuFw*    stack[MAX_MENUDEPTH];
  glconfig_t glconfig;
  bool       debug;
  qhandle_t  whiteShader;
  qhandle_t  menuBackShader;
  qhandle_t  menuBackNoLogoShader;
  qhandle_t  charset;
  qhandle_t  charsetProp;
  qhandle_t  charsetPropGlow;
  qhandle_t  charsetPropB;
  qhandle_t  cursor;
  qhandle_t  rb_on;
  qhandle_t  rb_off;
  float      xscale;
  float      yscale;
  float      bias;
  bool       demoversion;
  bool       firstdraw;
} uiStatic_t;
//:::::::::::::::::
typedef struct q3sound_s {
  sfxHandle_t menu_in;
  sfxHandle_t menu_out;
  sfxHandle_t menu_move;
  sfxHandle_t menu_buzz;
  sfxHandle_t menu_null;
  sfxHandle_t weaponChange;
} Q3sound;
//:::::::::::::::::

//:::::::::::::::::
#endif//UI_TYPES_H
