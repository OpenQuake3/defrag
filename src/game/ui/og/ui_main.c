// USER INTERFACE MAIN
#include "ui_local.h"


// vmMain
// cvarTable_t
// cvars

// ui_run_fraglimit
// ui_run_timelimit

vmCvar_t ui_tourney_fraglimit;
vmCvar_t ui_tourney_timelimit;

vmCvar_t ui_team_fraglimit;
vmCvar_t ui_team_timelimit;
vmCvar_t ui_team_friendly;

vmCvar_t ui_ctf_capturelimit;
vmCvar_t ui_ctf_timelimit;
vmCvar_t ui_ctf_friendly;

vmCvar_t ui_arenasFile;
vmCvar_t ui_botsFile;
vmCvar_t ui_spScores1;
vmCvar_t ui_spScores2;
vmCvar_t ui_spScores3;
vmCvar_t ui_spScores4;
vmCvar_t ui_spScores5;
vmCvar_t ui_spAwards;
vmCvar_t ui_spVideos;
vmCvar_t ui_spSkill;

vmCvar_t ui_spSelection;

vmCvar_t ui_browserMaster;
vmCvar_t ui_browserGameType;
vmCvar_t ui_browserSortKey;
vmCvar_t ui_browserShowFull;
vmCvar_t ui_browserShowEmpty;

vmCvar_t ui_brassTime;
vmCvar_t ui_drawCrosshair;
vmCvar_t ui_drawCrosshairNames;
vmCvar_t ui_marks;

vmCvar_t ui_server1;
vmCvar_t ui_server2;
vmCvar_t ui_server3;
vmCvar_t ui_server4;
vmCvar_t ui_server5;
vmCvar_t ui_server6;
vmCvar_t ui_server7;
vmCvar_t ui_server8;
vmCvar_t ui_server9;
vmCvar_t ui_server10;
vmCvar_t ui_server11;
vmCvar_t ui_server12;
vmCvar_t ui_server13;
vmCvar_t ui_server14;
vmCvar_t ui_server15;
vmCvar_t ui_server16;

vmCvar_t ui_cdkeychecked;
vmCvar_t ui_ioq3;

static cvarTable_t cvarTable[] = {
  {     &ui_run_fraglimit,      "ui_run_fraglimit",    "0",              CVAR_ARCHIVE}, //::OSDF changed to "run" and 0, from "ffa" and 20
  {     &ui_run_timelimit,      "ui_run_timelimit",    "0",              CVAR_ARCHIVE}, //::OSDF changed to "run"

  { &ui_tourney_fraglimit,  "ui_tourney_fraglimit",    "0",              CVAR_ARCHIVE},
  { &ui_tourney_timelimit,  "ui_tourney_timelimit",   "15",              CVAR_ARCHIVE},

  {    &ui_team_fraglimit,     "ui_team_fraglimit",    "0",              CVAR_ARCHIVE},
  {    &ui_team_timelimit,     "ui_team_timelimit",   "20",              CVAR_ARCHIVE},
  {     &ui_team_friendly,      "ui_team_friendly",    "1",              CVAR_ARCHIVE},

  {  &ui_ctf_capturelimit,   "ui_ctf_capturelimit",    "8",              CVAR_ARCHIVE},
  {     &ui_ctf_timelimit,      "ui_ctf_timelimit",   "30",              CVAR_ARCHIVE},
  {      &ui_ctf_friendly,       "ui_ctf_friendly",    "0",              CVAR_ARCHIVE},

  {        &ui_arenasFile,          "g_arenasFile",     "",      CVAR_INIT | CVAR_ROM},
  {          &ui_botsFile,            "g_botsFile",     "",      CVAR_INIT | CVAR_ROM},
  {         &ui_spScores1,           "g_spScores1",     "",              CVAR_ARCHIVE},
  {         &ui_spScores2,           "g_spScores2",     "",              CVAR_ARCHIVE},
  {         &ui_spScores3,           "g_spScores3",     "",              CVAR_ARCHIVE},
  {         &ui_spScores4,           "g_spScores4",     "",              CVAR_ARCHIVE},
  {         &ui_spScores5,           "g_spScores5",     "",              CVAR_ARCHIVE},
  {          &ui_spAwards,            "g_spAwards",     "",              CVAR_ARCHIVE},
  {          &ui_spVideos,            "g_spVideos",     "",              CVAR_ARCHIVE},
  {           &ui_spSkill,             "g_spSkill",    "2", CVAR_ARCHIVE | CVAR_LATCH},

  {       &ui_spSelection,        "ui_spSelection",     "",                  CVAR_ROM},

  {     &ui_browserMaster,      "ui_browserMaster",    "1",              CVAR_ARCHIVE},
  {   &ui_browserGameType,    "ui_browserGameType",    "0",              CVAR_ARCHIVE},
  {    &ui_browserSortKey,     "ui_browserSortKey",    "4",              CVAR_ARCHIVE},
  {   &ui_browserShowFull,    "ui_browserShowFull",    "1",              CVAR_ARCHIVE},
  {  &ui_browserShowEmpty,   "ui_browserShowEmpty",    "1",              CVAR_ARCHIVE},

  {         &ui_brassTime,          "cg_brassTime", "2500",              CVAR_ARCHIVE},
  {     &ui_drawCrosshair,      "cg_drawCrosshair",    "4",              CVAR_ARCHIVE},
  {&ui_drawCrosshairNames, "cg_drawCrosshairNames",    "1",              CVAR_ARCHIVE},
  {             &ui_marks,              "cg_marks",    "1",              CVAR_ARCHIVE},

  {           &ui_server1,               "server1",     "",              CVAR_ARCHIVE},
  {           &ui_server2,               "server2",     "",              CVAR_ARCHIVE},
  {           &ui_server3,               "server3",     "",              CVAR_ARCHIVE},
  {           &ui_server4,               "server4",     "",              CVAR_ARCHIVE},
  {           &ui_server5,               "server5",     "",              CVAR_ARCHIVE},
  {           &ui_server6,               "server6",     "",              CVAR_ARCHIVE},
  {           &ui_server7,               "server7",     "",              CVAR_ARCHIVE},
  {           &ui_server8,               "server8",     "",              CVAR_ARCHIVE},
  {           &ui_server9,               "server9",     "",              CVAR_ARCHIVE},
  {          &ui_server10,              "server10",     "",              CVAR_ARCHIVE},
  {          &ui_server11,              "server11",     "",              CVAR_ARCHIVE},
  {          &ui_server12,              "server12",     "",              CVAR_ARCHIVE},
  {          &ui_server13,              "server13",     "",              CVAR_ARCHIVE},
  {          &ui_server14,              "server14",     "",              CVAR_ARCHIVE},
  {          &ui_server15,              "server15",     "",              CVAR_ARCHIVE},
  {          &ui_server16,              "server16",     "",              CVAR_ARCHIVE},

  {      &ui_cdkeychecked,       "ui_cdkeychecked",    "0",                  CVAR_ROM},
  {              &ui_ioq3,               "ui_ioq3",    "1",                  CVAR_ROM},
  {                  NULL,       "g_localTeamPref",     "",                         0}
};

// cvarTableSize
// UI_RegisterCvars
// UI_UpdateCvars


