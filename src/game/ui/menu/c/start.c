#include "../../local.h"
#include "../../shared.h"
#include "../../framework/elements.h"
#include "../../framework/tools.h"
#include "../all.h"

//:::::::::::::::::::
typedef struct {
  MenuFw    menu;
  MenuText  singleplayer;
  MenuText  multiplayer;
  MenuText  setup;
  MenuText  demos;
  MenuText  cinematics;
  MenuText  teamArena;
  MenuText  mods;
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
bool    m_entersound;  // after a frame, so caching won't disrupt the sound
//:::::::::::::::::::

//:::::::::::::::::::
// errorMessage_Key
//:::::::::::::::::::
sfxHandle_t errorMessage_key(int key) {
  id3Cvar_Set("com_errorMessage", "");
  menuStart();
  return (q3sound.menu_null);
}

//:::::::::::::::::::
// menuStart_Cache
//   Register Start menu cache
//:::::::::::::::::::
#define MDL_START_BANNER "models/mapobjects/banner/banner5.md3"
//:::::::::::::::::::
void menuStart_cache(void) {
  // Register the banner model file
  s_mstart.bannerModel = id3R_RegisterModel(MDL_START_BANNER);
}
//:::::::::::::::::::
// Main_MenuDraw
//   Common to the main menu and errorMessage menu
//:::::::::::::::::::
#define BANNER_X 320
#define BANNER_Y 450
#define BANNER_TEXT "Opensource Defrag | GPL2.0+"
//:::::::::::::::::::
static void menuStart_draw(void) {  // Main_MenuDraw(void) {
  // setup the refdef
  refdef_t refdef;
  memset(&refdef, 0, sizeof(refdef));
  refdef.rdflags = RDF_NOWORLDMODEL;
  AxisClear(refdef.viewaxis);

  float x = 0;
  float y = 0;
  float w = 160;  // 640;
  float h = 90;   // 120;
  uiAdjustFrom640(&x, &y, &w, &h);
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

  // id3R_AddRefEntityToScene(&ent);

  id3R_RenderScene(&refdef);

  if (strlen(s_errorMsg.errorMessage)) {
    uiDrawPString_AutoWrap(320, 192, 600, 20, s_errorMsg.errorMessage, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW, (vec_t*)q3color.text_menu);
  } else {
    uiDrawMenu(&s_mstart.menu);  // standard menu drawing
  }
  uiDrawString(BANNER_X, BANNER_Y, BANNER_TEXT, UI_CENTER | UI_SMALLFONT, (vec_t*)mStartColor.neutral);
}


//:::::::::::::::::::
// MainMenu_ExitAction
//:::::::::::::::::::
static void menuStart_exit(bool result) {
  if (!result) { return; }
  menuPop();
  menuCredits();
}

//:::::::::::::::::::
// Main_MenuEvent
//:::::::::::::::::::
void menuStart_event(void* ptr, int event) {
  if (event != MS_ACTIVATED) { return; }
  switch (((MenuCommon*)ptr)->id) {
    // case MID_SINGLEPLAYER: menuSPLevel(); break;
    // case MID_MULTIPLAYER: menuArenaServers(); break;
    // case MID_SETUP: menuSetup(); break;
    // case MID_DEMOS: menuDemos(); break;
    // case MID_CINEMATICS: menuCinematics(); break;
    // case MID_MODS: menuMods(); break;
    // case MID_TEAMARENA: id3Cvar_Set("fs_game", BASETA); id3Cmd_ExecuteText(EXEC_APPEND, "vid_restart;"); break;
    case MID_EXIT: menuConfirm("EXIT GAME?", 0, menuStart_exit); break;
    default: Com_Printf("%s: Requested unrecognized menu ID %i", __func__, ((MenuCommon*)ptr)->id); break;
  }
}

//:::::::::::::::::::
// menuStart
//   Defines the properties of the Start menu
//   Initializes its cache
//:::::::::::::::::::
#define MAIN_MENU_VERTICAL_SPACING 34
//:::::::::::::::::::
void menuStart(void) {                // UI_MainMenu(void) {
  id3Cvar_Set("sv_killserver", "1");  // Make sure the attract loop server is down and local cinematics are killed
  // Initialize all data
  MenuStart* sm  = &s_mstart;
  ErrorMsg*  err = &s_errorMsg;
  memset(sm, 0, sizeof(MenuStart));
  memset(err, 0, sizeof(ErrorMsg));
  menuStart_cache();  // com_errorMessage needs cache data too // MainMenu_Cache();

  id3Cvar_VariableStringBuffer("com_errorMessage", err->errorMessage, sizeof(err->errorMessage));
  if (strlen(err->errorMessage)) {
    err->menu.draw       = menuStart_draw;  // Main_MenuDraw;
    err->menu.key        = errorMessage_key;
    err->menu.fullscreen = true;
    err->menu.wrapAround = true;
    err->menu.showlogo   = true;
    id3Key_SetCatcher(KEYCATCH_UI);
    uis.menusp = 0;
    menuPush(&err->menu);
    return;
  }

  int    xpos                       = 32;  // 320;
  int    justify                    = MFL_LEFT_JUSTIFY;
  int    y                          = 200;  // 134;
  int    style                      = UI_LEFT | UI_DROPSHADOW;
  vec_t* textColor                  = (vec_t*)q3color.text_menu;

  sm->menu.draw                     = menuStart_draw;
  sm->menu.fullscreen               = true;
  sm->menu.wrapAround               = true;
  sm->menu.showlogo                 = true;

  sm->singleplayer.generic.type     = MITEM_PTEXT;
  sm->singleplayer.generic.flags    = MFL_LEFT_JUSTIFY | MFL_PULSEIFFOCUS;
  sm->singleplayer.generic.x        = xpos;
  sm->singleplayer.generic.y        = y;
  sm->singleplayer.generic.id       = MID_SINGLEPLAYER;
  sm->singleplayer.generic.callback = menuStart_event;
  sm->singleplayer.string           = "SINGLE PLAYER";
  sm->singleplayer.color            = textColor;
  sm->singleplayer.style            = style;

  y += MAIN_MENU_VERTICAL_SPACING;
  sm->multiplayer.generic.type     = MITEM_PTEXT;
  sm->multiplayer.generic.flags    = justify | MFL_PULSEIFFOCUS;
  sm->multiplayer.generic.x        = xpos;
  sm->multiplayer.generic.y        = y;
  sm->multiplayer.generic.id       = MID_MULTIPLAYER;
  sm->multiplayer.generic.callback = menuStart_event;
  sm->multiplayer.string           = "MULTIPLAYER";
  sm->multiplayer.color            = textColor;
  sm->multiplayer.style            = style;

  y += MAIN_MENU_VERTICAL_SPACING;
  sm->demos.generic.type     = MITEM_PTEXT;
  sm->demos.generic.flags    = justify | MFL_PULSEIFFOCUS;
  sm->demos.generic.x        = xpos;
  sm->demos.generic.y        = y;
  sm->demos.generic.id       = MID_DEMOS;
  sm->demos.generic.callback = menuStart_event;
  sm->demos.string           = "DEMOS";
  sm->demos.color            = textColor;
  sm->demos.style            = style;

  y += MAIN_MENU_VERTICAL_SPACING;

  y += MAIN_MENU_VERTICAL_SPACING;
  sm->setup.generic.type     = MITEM_PTEXT;
  sm->setup.generic.flags    = justify | MFL_PULSEIFFOCUS;
  sm->setup.generic.x        = xpos;
  sm->setup.generic.y        = y;
  sm->setup.generic.id       = MID_SETUP;
  sm->setup.generic.callback = menuStart_event;
  sm->setup.string           = "SETUP";
  sm->setup.color            = textColor;
  sm->setup.style            = style;

  y += MAIN_MENU_VERTICAL_SPACING;
  sm->mods.generic.type     = MITEM_PTEXT;
  sm->mods.generic.flags    = justify | MFL_PULSEIFFOCUS;
  sm->mods.generic.x        = xpos;
  sm->mods.generic.y        = y;
  sm->mods.generic.id       = MID_MODS;
  sm->mods.generic.callback = menuStart_event;
  sm->mods.string           = "MODS";
  sm->mods.color            = textColor;
  sm->mods.style            = style;

  y += MAIN_MENU_VERTICAL_SPACING;
  sm->exit.generic.type     = MITEM_PTEXT;
  sm->exit.generic.flags    = justify | MFL_PULSEIFFOCUS;
  sm->exit.generic.x        = xpos;
  sm->exit.generic.y        = y;
  sm->exit.generic.id       = MID_EXIT;
  sm->exit.generic.callback = menuStart_event;
  sm->exit.string           = "EXIT";
  sm->exit.color            = textColor;
  sm->exit.style            = style;

  // menuAddItem(&sm->menu, &sm->singleplayer);
  // menuAddItem(&sm->menu, &sm->multiplayer);
  // menuAddItem(&sm->menu, &sm->demos);
  // menuAddItem(&sm->menu, &sm->setup);
  // menuAddItem(&sm->menu, &sm->mods);
  menuAddItem(&sm->menu, &sm->exit);

  id3Key_SetCatcher(KEYCATCH_UI);
  uis.menusp = 0;
  menuPush(&sm->menu);
}
