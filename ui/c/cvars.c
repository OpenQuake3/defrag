//:::::::::::::::::::

#include "../local.h"

//:::::::::::::::::::
typedef struct {
  vmCvar_t* vmCvar;
  char*     cvarName;
  char*     defaultString;
  int       cvarFlags;
} cvarTable_t;
//:::::::::::::::::::
vmCvar_t run_fraglimit;
vmCvar_t run_timelimit;
//:::::::::::::::::::
static cvarTable_t cvarTable[] = {
  {&run_fraglimit, "ui_run_fraglimit", "0", CVAR_ARCHIVE}, //::OSDF changed to "run" and 0, from "ffa" and 20
  {&run_timelimit, "ui_run_timelimit", "0", CVAR_ARCHIVE}, //::OSDF changed to "run"
};
static int cvarTableSize = ARRAY_LEN(cvarTable);
//:::::::::::::::::::

//:::::::::::::::::::
// UI_RegisterCvars
//:::::::::::::::::::
void uiCvarsRegisterAll(void) {
  int          i;
  cvarTable_t* cv;
  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) { id3CvarRegister(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags); }
}

//:::::::::::::::::::
// UI_UpdateCvars
//:::::::::::::::::::
void uiCvarsUpdateAll(void) {
  int          i;
  cvarTable_t* cv;
  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
    if (!cv->vmCvar) { continue; }
    id3Cvar_Update(cv->vmCvar);
  }
}
