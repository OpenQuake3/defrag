#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H
//:::::::::::::::::

// Engine dependencies
#include "../../client/keycodes.h"
#include "../shared.h"
// Ui dependencies
#include "../color/schemes.h"
#include "../color/tools.h"
#include "tools.h"
#include "../types.h"
#include "../config.h"
#include "../callbacks.h"

//:::::::::::::::::
// Globals
extern qhandle_t sliderBar;
extern qhandle_t sliderButton_0;
extern qhandle_t sliderButton_1;
//:::::::::::::::::
extern Q3sound    q3sound;
extern Sounds     uiSound;
extern Songs      song;
extern uiStatic_t uis;
//:::::::::::::::::

//:::::::::::::::::
// Global Menu IDs
#define MID_PLAY 10
#define MID_MULTIPLAYER 11
#define MID_SETUP 12
#define MID_DEMOS 13
#define MID_CINEMATICS 14
#define MID_TEAMARENA 15
#define MID_MODS 16
#define MID_EXIT 17

//:::::::::::::::::
// Menu Item Properties
//:::::::::::::::::
// Menu Item Indexes
#define MITEM_NULL 0
#define MITEM_SLIDER 1    // Slideable item that controls a range of values
#define MITEM_ACTION 2    // Executes an action (?when?). eg. Animating the player model in the settings menu
#define MITEM_MULTIOPT 3  // Multi Option list: Will cycle through the given list
#define MITEM_FIELD 4     // Input Field: Text item that can be changed by the user
#define MITEM_SWITCH 5    // Boolean option (checkbox style). User can mark it on/off
#define MITEM_IMAGE 6     // Image that will act as a selectable menu item
#define MITEM_LTEXT 7     // Non-clickable item ??
#define MITEM_LIST 8      // Item list (like mods menu): Drawn all at once on screen
#define MITEM_TEXT 9      // Proportional text, with the default font
// #define MITEM_BTEXT 10    // Banner title text
//:::::::::::::::::
// Menu Item Flags
#define MFL_BLINK ((unsigned int)0x00000001)
#define MFL_SMALLFONT ((unsigned int)0x00000002)
#define MFL_LEFT_JUSTIFY ((unsigned int)0x00000004)
#define MFL_CENTER_JUSTIFY ((unsigned int)0x00000008)
#define MFL_RIGHT_JUSTIFY ((unsigned int)0x00000010)
#define MFL_NUMBERSONLY ((unsigned int)0x00000020)  // edit field is only numbers
#define MFL_HIGHLIGHT ((unsigned int)0x00000040)
#define MFL_HIGHLIGHT_IF_FOCUS ((unsigned int)0x00000080)  // steady focus
#define MFL_PULSEIFFOCUS ((unsigned int)0x00000100)        // pulse if focus
#define MFL_HASMOUSEFOCUS ((unsigned int)0x00000200)
#define MFL_NOONOFFTEXT ((unsigned int)0x00000400)
#define MFL_MOUSEONLY ((unsigned int)0x00000800)      // only mouse input allowed
#define MFL_HIDDEN ((unsigned int)0x00001000)         // skips drawing
#define MFL_GRAYED ((unsigned int)0x00002000)         // grays and disables
#define MFL_INACTIVE ((unsigned int)0x00004000)       // disables any input
#define MFL_NODEFAULTINIT ((unsigned int)0x00008000)  // skip default initialization
#define MFL_OWNERDRAW ((unsigned int)0x00010000)
#define MFL_PULSE ((unsigned int)0x00020000)
#define MFL_LOWERCASE ((unsigned int)0x00040000)  // edit field is all lower case
#define MFL_UPPERCASE ((unsigned int)0x00080000)  // edit field is all upper case
#define MFL_SILENT ((unsigned int)0x00100000)
//:::::::::::::::::
// Menu State flags
//   Engine callback notifications of menu states
typedef enum MenuState_e { MST_FOCUS = 1, MST_FOCUSLOST, MST_ACTIVE } MenuState;
// #define MST_FOCUS 1
// #define MST_FOCUSLOST 2
// #define MST_ACTIVE 3
//:::::::::::::::::

//:::::::::::::::::
// Menu Drawing tools
void uiDrawMenu(MenuFw*);

//:::::::::::::::::
// Framework elements  (widgets)
//:::::::::::::::::
// elements/field.c
void        menuField_init(MenuField*);
void        menuField_draw(MenuField*);
sfxHandle_t menuField_key(MenuField*, int*);
// elements/multiopt.c
void        menuMOpt_init(MenuList*);
void        menuMOpt_draw(MenuList*);
sfxHandle_t menuMOpt_key(MenuList*, int);
// elements/switch.c
void        menuSwitch_init(MenuSwitch*);
void        menuSwitch_draw(MenuSwitch*);
sfxHandle_t menuSwitch_key(MenuSwitch*, int);
// elements/slider.c
void        menuSlider_init(MenuSlider*);
void        menuSlider_draw(MenuSlider*);
sfxHandle_t menuSlider_key(MenuSlider*, int);
// elements/list.c
void        menuList_init(MenuList*);
void        menuList_draw(MenuList*);
sfxHandle_t menuList_key(MenuList*, int);
// framework/menu.c
void        menuPop(void);
void        menuPush(MenuFw*);
void        menuAddItem(MenuFw*, void*);
sfxHandle_t menuDefaultKey(MenuFw*, int);
// elements/action.c
void menuAction_init(MenuAction*);
void menuAction_draw(MenuAction*);
// elements/image.c
void menuImage_init(MenuImage*);
void menuImage_draw(MenuImage*);
// elements/text.c
void menuText_init(MenuText*);
void menuText_draw(MenuText*);
void OText_init(MenuText*);
void OText_draw(MenuText*);
// void BText_init(MenuText*);
// void BText_draw(MenuText*);
// elements/cursor.c
bool  cursorInRect(float, float, float, float);
bool  cursorInRectPix(int, int, int, int);
void* cursorGetItem(MenuFw*);
void  cursorMoved(MenuFw*);
void  cursorSet(MenuFw*, int);
void  cursorSetToItem(MenuFw*, void*);
void  cursorAdjust(MenuFw*, int);
//:::::::::::::::::

//:::::::::::::::::
#endif  // UI_ELEMENTS_H
