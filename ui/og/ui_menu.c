/*
=======================================================================

MAIN MENU

=======================================================================
*/

#include "ui_local.h"

// Menu IDs

// MAIN_BANNER_MODEL
// MAIN_MENU_VERTICAL_SPACING

// mainmenu_t
// s_main
// errorMessage_t
// s_errorMessage
// MainMenu_ExitAction

// Main_MenuEvent
// MainMenu_Cache
// ErrorMessage_Key

// Main_MenuDraw

/*
===============
UI_TeamArenaExists
===============
*/
static bool UI_TeamArenaExists(void) {
  int   numdirs;
  char  dirlist[2048];
  char* dirptr;
  char* descptr;
  int   i;
  int   dirlen;

  numdirs = trap_FS_GetFileList("$modlist", "", dirlist, sizeof(dirlist));
  dirptr  = dirlist;
  for (i = 0; i < numdirs; i++) {
    dirlen  = strlen(dirptr) + 1;
    descptr = dirptr + dirlen;
    if (Q_stricmp(dirptr, BASETA) == 0) { return true; }
    dirptr += dirlen + strlen(descptr) + 1;
  }
  return false;
}

// UI_MainMenu
