#include "../../local.h"

//:::::::::::::::::::
// Menu Item IDs
#define MID_PLAY_CANCEL 10
#define MID_PLAY_LOAD 11
#define MID_PLAY_LIST 12
//:::::::::::::::::::
// Limits and Config
#define MAX_MAPS 1024
#define MAX_MAPNAME_SIZE 32
#define NAME_BUFSIZE (MAX_MAPS * MAX_MAPNAME_SIZE)


//:::::::::::::::::::
typedef struct MenuPlay_s {
  MenuFw    menu;
  MenuList  list;
  MenuText  title;
  MenuImage cancel;
  MenuImage load;
  char*     maplist[MAX_MAPS];
} MenuPlay;
//:::::::::::::::::::
static MenuPlay s_mplay;
//:::::::::::::::::::


static void menuPlay_event(void* ptr, int event);
//:::::::::::::::::::
static sfxHandle_t menuPlay_key(int key) {
  switch (key) {
    case K_MOUSE2: /* fall-through */
    case K_ESCAPE:   menuPop(); return uiSound.cancel;
    case K_KP_ENTER: /* fall-through */
    case K_ENTER:    menuPlay_event(&s_mplay.list, MST_ACTIVE); return uiSound.move;
  }
  return menuList_key(&s_mplay.list, key);
}

//:::::::::::::::::::
static void menuPlay_event(void* ptr, int event) {
  if (event != MST_ACTIVE) { return; }

  switch (((MenuCommon*)ptr)->id) {
    case MID_PLAY_LIST: /* fall-through on double-click */
    case MID_PLAY_LOAD: {
      menuForceOff();
      id3Cmd_ExecuteText(EXEC_APPEND, va("map %s\n", s_mplay.list.itemNames[s_mplay.list.curvalue]));
      break;
    }
    case MID_PLAY_CANCEL: {
      menuPop();
      break;
    }
  }
}

//:::::::::::::::::::
// menuPlay_cache
//:::::::::::::::::::
static void menuPlay_cache(void) {}


//:::::::::::::::::::
// menuPlay_init
//:::::::::::::::::::
static void menuPlay_init(void) {
  memset(&s_mplay, 0, sizeof(MenuPlay));
  menuPlay_cache();

  float  xMargin                  = (GL_W * 0.04) / GL_W;
  float  yMargin                  = (GL_H * 0.04) / GL_H;
  vec_t* textColor                = (vec_t*)mColor.fg;
  vec_t* focusColor               = (vec_t*)mColor.key;
  int    iconFlags                = MFL_CENTER_JUSTIFY | MFL_HIGHLIGHT_IF_FOCUS;
  char*  mapExtension             = ".bsp";

  s_mplay.menu.fullscreen         = true;
  s_mplay.menu.wrapAround         = true;
  s_mplay.menu.key                = menuPlay_key;

  s_mplay.title.generic.name      = "Title Label";
  s_mplay.title.string            = "choose a map";
  s_mplay.title.font              = uis.font.actionKey;
  s_mplay.title.generic.type      = MITEM_TEXT;
  s_mplay.title.generic.x         = 0 + xMargin;
  s_mplay.title.generic.y         = 0.10 + uiTextGetHeight(s_mplay.title.string, &s_mplay.title.font, fontScale(&s_mplay.title.font), strlen(s_mplay.title.string));
  s_mplay.title.align             = TEXT_ALIGN_LEFT;
  s_mplay.title.color             = textColor;
  s_mplay.title.generic.flags     = 0;
  s_mplay.title.style             = MFL_HIDDEN;

  s_mplay.cancel.generic.name     = "Cancel";
  s_mplay.cancel.generic.type     = MITEM_IMAGE;
  s_mplay.cancel.generic.flags    = iconFlags;
  s_mplay.cancel.generic.callback = menuPlay_event;
  s_mplay.cancel.width            = 0.05 * GL_W;
  s_mplay.cancel.height           = 0.05 * GL_W;
  s_mplay.cancel.generic.x        = 0.15;
  s_mplay.cancel.generic.y        = 1 - yMargin - ((float)s_mplay.cancel.height / GL_H);  // 480 - 64;
  s_mplay.cancel.generic.id       = MID_PLAY_CANCEL;
  // s_mplay.cancel.focuspic         = ART_BACK1;
  // s_mplay.cancel.errorpic         = ??
  s_mplay.cancel.shader           = uis.icon.cancel;
  s_mplay.cancel.focusshader      = uis.icon.cancel;
  s_mplay.cancel.focuscolor       = focusColor;

  s_mplay.load.generic.name       = "Load";
  s_mplay.load.generic.type       = MITEM_IMAGE;
  s_mplay.load.generic.flags      = iconFlags;
  s_mplay.load.generic.callback   = menuPlay_event;
  s_mplay.load.width              = 0.05 * GL_W;
  s_mplay.load.height             = 0.05 * GL_W;
  s_mplay.load.generic.x          = 1 - 0.15;
  s_mplay.load.generic.y          = 1 - yMargin - ((float)s_mplay.load.height / GL_H);  // 480 - 64;
  s_mplay.load.generic.id         = MID_PLAY_LOAD;
  // s_mplay.load.focuspic           = ART_GO1;
  // s_mplay.load.errorpic         = ??
  s_mplay.load.shader             = uis.icon.accept;
  s_mplay.load.focusshader        = uis.icon.accept;
  s_mplay.load.focuscolor         = focusColor;

  s_mplay.list.generic.name       = "MapList";
  s_mplay.list.align              = TEXT_ALIGN_LEFT;
  s_mplay.list.font               = uis.font.normal;
  s_mplay.list.generic.type       = MITEM_LIST;
  s_mplay.list.generic.flags      = 0;  // MFL_PULSEIFFOCUS;
  s_mplay.list.style              = 0;
  s_mplay.list.generic.callback   = menuPlay_event;
  s_mplay.list.generic.id         = MID_PLAY_LIST;
  s_mplay.list.generic.x          = xMargin;
  float titleHeight      = uiTextGetHeight(s_mplay.title.string, &s_mplay.title.font, fontScale(&s_mplay.title.font), strlen(s_mplay.title.string));
  s_mplay.list.generic.y = yMargin + titleHeight + yMargin + titleHeight;
  s_mplay.list.width     = 0.5 - xMargin;
  s_mplay.list.height    = 0.5;
  s_mplay.list.itemNames = (const char**)s_mplay.maplist;
  s_mplay.list.columns   = 1;
  s_mplay.list.rows      = 100;  // Max rows. Will be clamped to the height if they overflow
  // Vector2Set(s_mplay.list.separation, 0.01, 0.01);

  // id3FS_GetFileList(const char* path, const char* extension, char* listbuf, int bufsize)
  char mapNames[NAME_BUFSIZE];  // Map names buffer. It's not an array of arrays, its an array of characters
  int  mapCount = id3FS_GetFileList("maps", mapExtension, mapNames, ARRAY_LEN(mapNames));

  // Com_Printf("%d", s_mplay.list.curvalue);

  // NOTE: This is assigning a slice of a bigger all-names array as a single mapname.
  char* mapname = mapNames;

  // .........................................
  int map       = 0;
  for (int j = 0; j < 2; j++) {  // TODO: Why is this repeating twice ??
    if (mapCount > MAX_MAPS) { mapCount = MAX_MAPS; }

    for (; map < mapCount; map++) {  // TODO: Doesn't this loop just hit the max maps the first time, instead?
      // remove .bsp extension from mapname
      int len = strlen(mapname);
      if (len > strlen(mapExtension)) { mapname[len - strlen(mapExtension)] = '\0'; }
      s_mplay.list.itemNames[map] = mapname;
      mapname += len + 1;  // Move to the next name in the same array of characters (not array of arrays)
    }
  }
  // .........................................

  s_mplay.list.itemCount = mapCount;

  if (!mapCount) {
    s_mplay.list.itemNames[0] = "No Maps Found.";
    s_mplay.list.itemCount    = 1;
    // degenerate case, not selectable
    s_mplay.load.generic.flags |= (MFL_HIDDEN | MFL_INACTIVE);
  }

  menuAddItem(&s_mplay.menu, &s_mplay.title);
  menuAddItem(&s_mplay.menu, &s_mplay.list);
  menuAddItem(&s_mplay.menu, &s_mplay.cancel);
  menuAddItem(&s_mplay.menu, &s_mplay.load);

  menuPush(&s_mplay.menu);
}


//:::::::::::::::::::
// menuPlay
//   Called when starting the menu
//:::::::::::::::::::
void menuPlay(void) { menuPlay_init(); }
// void menuPlay(void) { }


//:::::::::::::::::::
// menuPlay_thumbnailDraw
//:::::::::::::::::::
static void menuPlay_thumbnailDraw( void *self ) {
//  menubitmap_s* b = (menubitmap_s *)self;
//  if( !b->generic.name ) { return; }

//  if( b->generic.name && !b->shader ) {
//    b->shader = trap_R_RegisterShaderNoMip( b->generic.name );
//    if( !b->shader && b->errorpic ) {
//      b->shader = trap_R_RegisterShaderNoMip( b->errorpic );
//    }
//  }

//  if( b->focuspic && !b->focusshader ) {
//    b->focusshader = trap_R_RegisterShaderNoMip( b->focuspic );
//  }

//  int x = b->generic.x;
//  int y = b->generic.y;
//  int w = b->width;
//  int h = b->height;
//  if( b->shader ) { UI_DrawHandlePic( x, y, w, h, b->shader ); }

//  x = b->generic.x;
//  y = b->generic.y + b->height;
//  UI_FillRect( x, y, b->width, 28, colorBlack );

//  x += b->width / 2;
//  y += 4;
//  int n = s_startserver.page * MAX_MAPSPERPAGE + b->generic.id - ID_PICTURES;

//  char mapname[ MAX_NAMELENGTH ];
//  const char* info = UI_GetArenaInfoByNumber( s_startserver.maplist[ n ]);
//  Q_strncpyz( mapname, Info_ValueForKey( info, "map"), MAX_NAMELENGTH );
//  Q_strupr( mapname );
//  UI_DrawString( x, y, mapname, UI_CENTER|UI_SMALLFONT, color_orange );

//  x = b->generic.x;
//  y = b->generic.y;
//  w = b->width;
//  h = b->height + 28;
//  if( b->generic.flags & QMF_HIGHLIGHT ) { UI_DrawHandlePic( x, y, w, h, b->focusshader ); }
}
