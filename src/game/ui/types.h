#ifndef UI_TYPES_H
#define UI_TYPES_H
//:::::::::::::::::

#include "shared.h"
#include "../rendc/tr_types.h"
#include "config.h"

//::::::::::::::::::
// Menu Data types
typedef struct MenuFw_s {
  int   cursor;
  int   cursor_prev;
  int   nitems;
  void* items[MAX_MENUITEMS];
  bool  wrapAround;
  bool  fullscreen;
  bool  isMain;  // Menu is considered a main menu. (was showlogo)
  void (*draw)(void);
  sfxHandle_t (*key)(int key);  // Run when there is a keyboard event from the engine. Defaults to menuDefaultKey when not set
} MenuFw;
//::::::::::::::::::
typedef struct MenuCommon_s {
  int          type;      // MITEM type id
  const char*  name;      // Name of the item (also drawn for labeled text)
  int          id;        // Menu id. Used for navigation and actions
  float        x, y;      // Position of the menu (percentage)
  float        left;      // Left bound of the item (percentage)
  float        top;       // Top bound of the item (percentage)
  float        right;     // Right bound of the item (percentage)
  float        bottom;    // Bottom bound of the item (percentage)
  MenuFw*      parent;    // Owner of this menu item
  int          activeId;  // Currently active item (? hovering or opposite ?) (was menuPosition)
  unsigned int flags;     // Item properties
  void (*callback)(void* self, int event);
  void (*statusbar)(void* self);
  void (*ownerdraw)(void* self);
} MenuCommon;  // Properties common between items
//::::::::::::::::::
typedef struct Field_s {
  int  cursor;
  int  scroll;
  int  widthInChars;
  char buffer[MAX_EDIT_LINE];
  int  maxchars;
} Field;
//::::::::::::::::::
typedef struct MenuField_s {
  MenuCommon generic;  // Properties common between items
  Field      field;
} MenuField;
//::::::::::::::::::
typedef struct MenuSlider_s {
  MenuCommon generic;  // Properties common between items
  float      minvalue;
  float      maxvalue;
  float      curvalue;
  float      range;
} MenuSlider;
//::::::::::::::::::
typedef struct MenuList_s {
  MenuCommon   generic;  // Properties common between items
  int          oldvalue;
  int          curvalue;
  int          topId;       // Item id that we are currently drawing at the top
  const char** itemNames;   // Array of null terminated strings, containing all the list items
  int          itemCount;   // Total number of items (can be used as size of the names array)
  int          columns;     // Total number of columns that we want to draw
  int          rows;        // Current number of items that we are vertically drawing. Also initializes the max desired count.
  float        width;       // Max horizontal span of the list (percentage). Was per column, now total.
  float        height;      // Max vertical   span of the list (percentage). Was item count (rows) now measures dimension.
  vec2_t       separation;  // Desired horizontal+vertical separation between items (percentage)
  vec2_t       itemSize;    // Max horizontal+vertical size of the items in the list (pixels)
  int          style;          // Formatting flags
  fontInfo_t   font;           // Font data
  int          align;          // Text alignment enum id
  int          lastClickTime;  // Timestamp of the last click (for double-click detection)
  int          lastClickIndex; // Item index of the last click (for double-click detection)
} MenuList;
//::::::::::::::::::
typedef struct MenuAction_s {
  MenuCommon generic;  // Properties common between items
} MenuAction;
//::::::::::::::::::
typedef struct MenuSwitch_s {
  MenuCommon generic;  // Properties common between items
  int        curvalue;
} MenuSwitch;
//::::::::::::::::::
typedef struct MenuImage_s {
  MenuCommon generic;  // Properties common between items
  char*      focuspic;
  char*      errorpic;
  qhandle_t  shader;
  qhandle_t  focusshader;
  int        width;
  int        height;
  float*     focuscolor;
} MenuImage;
//::::::::::::::::::
typedef struct MenuText_s {
  MenuCommon generic;  // Properties common between items
  char*      string;   // Text to draw
  int        style;    // UI styling flags
  vec_t*     color;    // Text color
  fontInfo_t font;     // Font data
  int        align;    // Text alignment enum id
} MenuText;
//::::::::::::::::::

//::::::::::::::::::
typedef struct Fonts_s {
  fontInfo_t small;
  fontInfo_t normal;
  fontInfo_t label;
  fontInfo_t action;
  fontInfo_t actionKey;
  fontInfo_t mono;
  fontInfo_t number;
} Fonts;
//:::::::::::::::::

//::::::::::::::::::
typedef struct Icons_s {
  qhandle_t cancel;
  qhandle_t accept;
} Icons;
//::::::::::::::::::
typedef struct {
  int        frametime;
  int        realtime;
  int        cursorx;               // Mouse Cursor horizontal position
  int        cursory;               // Mouse cursor vertical position
  int        menusp;                // Menus counter and id ("pointer")
  MenuFw*    activemenu;            // Active menu data pointer
  MenuFw*    stack[MAX_MENUDEPTH];  // Menu data array
  glconfig_t glconfig;
  bool       debug;
  qhandle_t  whiteShader;
  qhandle_t  bgMain;  // Background for main menus (was menuBackShader)
  qhandle_t  bgAlt;   // Background for alternative menus (was menuBackNoLogoShader)
  qhandle_t  charset;
  qhandle_t  charsetProp;
  qhandle_t  charsetPropGlow;
  qhandle_t  charsetPropB;
  qhandle_t  logoQ3;  // TODO: Remove, and change to OSDF logo
  qhandle_t  cursor;
  Icons      icon;  // Icon handles
  qhandle_t  rb_on;
  qhandle_t  rb_off;
  float      xscale;
  float      yscale;
  float      bias;
  bool       demoversion;
  bool       firstdraw;
  Fonts      font;
} uiStatic_t;
//::::::::::::::::::
typedef struct q3sound_s {
  sfxHandle_t menu_in;
  sfxHandle_t menu_out;
  sfxHandle_t menu_move;
  sfxHandle_t menu_buzz;
  sfxHandle_t menu_null;
  sfxHandle_t weaponChange;
} Q3sound;
//..................
typedef struct Sounds_s {
  sfxHandle_t move;
  sfxHandle_t select;
  sfxHandle_t error;
  sfxHandle_t cancel;
  sfxHandle_t silence;
  sfxHandle_t notification;
} Sounds;
//::::::::::::::::::
typedef struct Songs_s {
  sfxHandle_t chronos;   // Alexander Nakarada - Chronos
  sfxHandle_t succubus;  // Alexander Nakarada - Succubus
} Songs;
//::::::::::::::::::

//::::::::::::::::::
#endif  // UI_TYPES_H
