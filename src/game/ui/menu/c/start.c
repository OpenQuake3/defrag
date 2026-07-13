#include "../../local.h"
#include "../../shared.h"
#include "../../framework/elements.h"
#include "../../framework/tools.h"
#include "../all.h"

//:::::::::::::::::::
typedef struct {
  MenuFw   menu;
  MenuText play;
  // MenuText  multiplayer;
  MenuText setup;
  // MenuText  demos;
  // MenuText  cinematics;
  // MenuText  teamArena;
  // MenuText  mods;
  MenuText  exit;
  qhandle_t bannerModel;
} MenuStart;
//:::::::::::::::::::
typedef struct errorMessage_s {
  MenuFw menu;
  char   errorMessage[4096];
} ErrorMsg;
//:::::::::::::::::::
// Menu only data
static MenuStart s_mstart;
static ErrorMsg  s_errorMsg;
//:::::::::::::::::::
// Module wide data
Q3sound q3sound;
Sounds  uiSound;
Songs   song;
bool    m_entersound;  // after a frame, so caching won't disrupt the sound
bool    m_enterSong;
//:::::::::::::::::::

//:::::::::::::::::::
// errorMessage_Key
//:::::::::::::::::::
sfxHandle_t errorMessage_key(int key) {
  id3Cvar_Set("com_errorMessage", "");
  menuStart_init();
  return (q3sound.menu_null);
}

//:::::::::::::::::::
// menuStart_Cache
//   Register Start menu cache
//:::::::::::::::::::
#define MDL_START_BANNER "models/mapobjects/banner/banner5.md3"
//:::::::::::::::::::
void menuStart_cache(void) {
  s_mstart.bannerModel = id3R_RegisterModel(MDL_START_BANNER);  // Register the banner model file
}
//:::::::::::::::::::
// Main_MenuDraw
//   Common to the main menu and errorMessage menu
//:::::::::::::::::::
#define BANNER_X 320
#define BANNER_Y 450
#define BANNER_TEXT GAME_NAME_HUMAN " " GAME_VERSION " | GPL-3.0+"
//:::::::::::::::::::
static void menuStart_draw(void) {  // Main_MenuDraw(void) {
  // setup the refdef
  refdef_t refdef;
  memset(&refdef, 0, sizeof(refdef));
  refdef.rdflags = RDF_NOWORLDMODEL;
  AxisClear(refdef.viewaxis);

  float x       = 0;
  float y       = 0;
  float w       = 160;  // 640;
  float h       = 90;   // 120;
  // uiAdjustFrom640(&x, &y, &w, &h);
  refdef.x      = x;
  refdef.y      = y;
  refdef.width  = w;
  refdef.height = h;

  float adjust  = 0;  // JDC: Kenneth asked me to stop this 1.0 * sin( (float)uis.realtime / 1000 );
  refdef.fov_x  = 60 + adjust;
  refdef.fov_y  = 19.6875 + adjust;
  refdef.time   = uis.realtime;

  vec3_t origin;
  VectorSet(origin, 300, 0, -32);

  id3R_ClearScene();

  // add the model
  refEntity_t ent;
  memset(&ent, 0, sizeof(ent));

  adjust = 5.0 * sin((float)uis.realtime / 5000);
  vec3_t angles;
  VectorSet(angles, 0, 180 + adjust, 0);
  AnglesToAxis(angles, ent.axis);
  ent.hModel = s_mstart.bannerModel;
  VectorCopy(origin, ent.origin);
  VectorCopy(origin, ent.lightingOrigin);
  ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
  VectorCopy(ent.origin, ent.oldorigin);

  id3R_AddRefEntityToScene(&ent);
  // id3R_RenderScene(&refdef);
  uiDrawHandlePic(0.40, 0.0, 1000, 1000, uis.logoQ3);

  if (strlen(s_errorMsg.errorMessage)) {
    uiDrawPString_AutoWrap(320, 192, 600, 20, s_errorMsg.errorMessage, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, (vec_t*)q3color.text_menu);
  } else {
    uiDrawMenu(&s_mstart.menu);  // standard menu drawing
  }
  uiTextDraw(BANNER_TEXT, &uis.font.small, 0.5, 0.99, fontScale(&uis.font.small), (vec_t*)mColor.neutral, 0, 0, strlen(BANNER_TEXT), TEXT_ALIGN_CENTER);
}


//:::::::::::::::::::
// MainMenu_ExitAction
//:::::::::::::::::::
static void menuStart_exit(bool result) {
  if (!result) { return; }
  menuPop();
  id3Cmd_ExecuteText(EXEC_APPEND, "quit\n");
  // menuCredits();
}

//:::::::::::::::::::
// Main_MenuEvent
//:::::::::::::::::::
void menuStart_event(void* ptr, int event) {
  if (event != MST_ACTIVE) { return; }
  switch (((MenuCommon*)ptr)->id) {
    case MID_PLAY: menuPlay(); break;
    // case MID_MULTIPLAYER: menuArenaServers(); break;
    // case MID_SETUP: menuSetup(); break;
    // case MID_DEMOS: menuDemos(); break;
    // case MID_CINEMATICS: menuCinematics(); break;
    // case MID_MODS: menuMods(); break;
    // case MID_TEAMARENA: id3Cvar_Set("fs_game", BASETA); id3Cmd_ExecuteText(EXEC_APPEND, "vid_restart;"); break;
    case MID_EXIT: menuConfirm("Exit", 0, menuStart_exit); break;
    default: Com_Printf("%s: Requested unrecognized menu ID %i\n", __func__, ((MenuCommon*)ptr)->id); break;
  }
}

//:::::::::::::::::::
// uiSpacingUpdate
//:::::::::::::::::::
inline static void uiSpacingUpdate(float* ySpacing, MenuText* m) { 
  *ySpacing = uiTextGetHeight(m->string, &m->font, fontScale(&m->font), strlen(m->string));
}

//:::::::::::::::::::
// menuStart_init
//   Defines the properties of the Start menu
//   Initializes its cache
//   UI_MainMenu
//:::::::::::::::::::
// #define MAIN_MENU_VERTICAL_SPACING 34
#define MAIN_MENU_VERTICAL_SPACING 0.10
//:::::::::::::::::::
void menuStart_init(void) {
  id3Cvar_Set("sv_killserver", "1");  // Make sure the attract loop server is down and local cinematics are killed
  // Initialize all data
  MenuStart* sm  = &s_mstart;
  ErrorMsg*  err = &s_errorMsg;
  memset(sm, 0, sizeof(MenuStart));
  memset(err, 0, sizeof(ErrorMsg));
  menuStart_cache();  // com_errorMessage needs cache data too // MainMenu_Cache();

  // Draw error message when needed
  id3Cvar_VariableStringBuffer("com_errorMessage", err->errorMessage, sizeof(err->errorMessage));
  if (strlen(err->errorMessage)) {
    err->menu.draw       = menuStart_draw;  // Main_MenuDraw;
    err->menu.key        = errorMessage_key;
    err->menu.fullscreen = true;
    err->menu.wrapAround = true;
    err->menu.isMain   = true;
    id3Key_SetCatcher(KEYCATCH_UI);
    uis.menusp = 0;
    menuPush(&err->menu);
    return;
  }

  // Else draw normal menu
  float  x                  = 0.15;  // GL_W * 0.5; // 32;  // 320;
  float  y                  = 0.55;
  float  ySpacing           = 0;     // Raw Spacing between one line and the next
  float  yMargin            = 0.03;  // Margin to add to raw spacing, to calculate absolute line spacing
  int    justify            = MFL_LEFT_JUSTIFY;
  int    style              = UI_DROPSHADOW;
  int    align              = TEXT_ALIGN_LEFT;
  vec_t* textColor          = (vec_t*)q3color.text_menu;

  sm->menu.draw             = menuStart_draw;
  sm->menu.fullscreen       = true;
  sm->menu.wrapAround       = true;
  sm->menu.isMain           = true;

  sm->play.string           = "Play";
  sm->play.generic.name     = sm->play.string;
  sm->play.color            = textColor;
  sm->play.style            = style;
  sm->play.font             = uis.font.actionKey;
  sm->play.align            = align;
  sm->play.generic.type     = MITEM_TEXT;
  sm->play.generic.flags    = justify | MFL_PULSEIFFOCUS;
  sm->play.generic.id       = MID_PLAY;
  sm->play.generic.callback = menuStart_event;
  sm->play.generic.x        = x;
  sm->play.generic.y        = y;
  // Move to the next line
  uiSpacingUpdate(&ySpacing, &sm->play);
  y += ySpacing + yMargin - 0.05;

  sm->setup.string           = "Settings";
  sm->setup.generic.name     = sm->setup.string;
  sm->setup.color            = textColor;
  sm->setup.style            = style;
  sm->setup.font             = uis.font.action;
  sm->setup.align            = align;
  sm->setup.generic.type     = MITEM_TEXT;
  sm->setup.generic.flags    = justify | MFL_PULSEIFFOCUS | MFL_GRAYED;
  sm->setup.generic.x        = x;
  sm->setup.generic.y        = y;
  sm->setup.generic.id       = MID_SETUP;
  sm->setup.generic.callback = menuStart_event;
  // Move to the next line
  uiSpacingUpdate(&ySpacing, &sm->setup);
  y += ySpacing + yMargin;

  sm->exit.string           = "Exit";
  sm->exit.generic.name     = sm->exit.string;
  sm->exit.color            = textColor;
  sm->exit.style            = style;
  sm->exit.font             = uis.font.action;
  sm->exit.align            = align;
  sm->exit.generic.type     = MITEM_TEXT;
  sm->exit.generic.flags    = justify | MFL_PULSEIFFOCUS;
  sm->exit.generic.x        = x;
  sm->exit.generic.y        = y;
  sm->exit.generic.id       = MID_EXIT;
  sm->exit.generic.callback = menuStart_event;

  menuAddItem(&sm->menu, &sm->play);
  menuAddItem(&sm->menu, &sm->setup);
  menuAddItem(&sm->menu, &sm->exit);

  id3Key_SetCatcher(KEYCATCH_UI);
  uis.menusp = 0;
  menuPush(&sm->menu);  // Add menu to the registered menus list ("stack")
}

