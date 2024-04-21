#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H
//:::::::::::::::::

// Engine dependencies
#include "../../client/keycodes.h"
#include "../shared.h"
// Ui dependencies
#include "../color/schemes.h"
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
extern uiStatic_t uis;
//:::::::::::::::::

//:::::::::::::::::
// Global Menu IDs
#define MID_SINGLEPLAYER 10
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
#define MITEM_SLIDER 1
#define MITEM_ACTION 2
#define MITEM_SPINCONTROL 3
#define MITEM_FIELD 4
#define MITEM_RADIOBUTTON 5
#define MITEM_BITMAP 6
#define MITEM_TEXT 7
#define MITEM_SCROLLLIST 8
#define MITEM_PTEXT 9
#define MITEM_BTEXT 10
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
#define MS_GOTFOCUS 1
#define MS_LOSTFOCUS 2
#define MS_ACTIVATED 3
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
// elements/spincontrol.c
void        spinControl_init(MenuList*);
void        spinControl_draw(MenuList*);
sfxHandle_t spinControl_key(MenuList*, int);
// elements/radiobtn.c
void        radioBtn_init(MenuRadioBtn*);
void        radioBtn_draw(MenuRadioBtn*);
sfxHandle_t radioBtn_key(MenuRadioBtn*, int);
// elements/slider.c
void        slider_init(MenuSlider*);
void        slider_draw(MenuSlider*);
sfxHandle_t slider_key(MenuSlider*, int);
// elements/scrollList.c
void        scrollList_init(MenuList*);
void        scrollList_draw(MenuList*);
sfxHandle_t scrollList_key(MenuList*, int);
// framework/menu.c
void        menuPop(void);
void        menuPush(MenuFw*);
void        menuAddItem(MenuFw*, void*);
sfxHandle_t menuDefaultKey(MenuFw*, int);
// elements/action.c
void action_init(MenuAction*);
void action_draw(MenuAction*);
// elements/bitmap.c
void bitmap_init(MenuBitmap*);
void bitmap_draw(MenuBitmap*);
// elements/text.c
void text_init(MenuText*);
void text_draw(MenuText*);
void BText_init(MenuText*);
void BText_draw(MenuText*);
void PText_init(MenuText*);
void PText_draw(MenuText*);
// elements/cursor.c
bool  cursorInRect(int, int, int, int);
void* cursorGetItem(MenuFw*);
void  cursorMoved(MenuFw*);
void  cursorSet(MenuFw*, int);
void  cursorSetToItem(MenuFw*, void*);
void  cursorAdjust(MenuFw*, int);
//:::::::::::::::::

//:::::::::::::::::
#endif  // UI_ELEMENTS_H
