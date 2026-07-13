
#include "../rendc/tr_types.h"
// NOTE: include the ui_public.h from the new UI
#include "../ui_ta/ui_public.h"
// redefine to old API version
#undef UI_API_VERSION
#define UI_API_VERSION 4
#include "../client/keycodes.h"
#include "../sgame/bg_public.h"

typedef void (*voidfunc_f)(void);

extern vmCvar_t ui_run_fraglimit;
extern vmCvar_t ui_run_timelimit;

extern vmCvar_t ui_tourney_fraglimit;
extern vmCvar_t ui_tourney_timelimit;

extern vmCvar_t ui_team_fraglimit;
extern vmCvar_t ui_team_timelimit;
extern vmCvar_t ui_team_friendly;

extern vmCvar_t ui_ctf_capturelimit;
extern vmCvar_t ui_ctf_timelimit;
extern vmCvar_t ui_ctf_friendly;

extern vmCvar_t ui_arenasFile;
extern vmCvar_t ui_botsFile;
extern vmCvar_t ui_spScores1;
extern vmCvar_t ui_spScores2;
extern vmCvar_t ui_spScores3;
extern vmCvar_t ui_spScores4;
extern vmCvar_t ui_spScores5;
extern vmCvar_t ui_spAwards;
extern vmCvar_t ui_spVideos;
extern vmCvar_t ui_spSkill;

extern vmCvar_t ui_spSelection;

extern vmCvar_t ui_browserMaster;
extern vmCvar_t ui_browserGameType;
extern vmCvar_t ui_browserSortKey;
extern vmCvar_t ui_browserShowFull;
extern vmCvar_t ui_browserShowEmpty;

extern vmCvar_t ui_brassTime;
extern vmCvar_t ui_drawCrosshair;
extern vmCvar_t ui_drawCrosshairNames;
extern vmCvar_t ui_marks;

extern vmCvar_t ui_server1;
extern vmCvar_t ui_server2;
extern vmCvar_t ui_server3;
extern vmCvar_t ui_server4;
extern vmCvar_t ui_server5;
extern vmCvar_t ui_server6;
extern vmCvar_t ui_server7;
extern vmCvar_t ui_server8;
extern vmCvar_t ui_server9;
extern vmCvar_t ui_server10;
extern vmCvar_t ui_server11;
extern vmCvar_t ui_server12;
extern vmCvar_t ui_server13;
extern vmCvar_t ui_server14;
extern vmCvar_t ui_server15;
extern vmCvar_t ui_server16;

extern vmCvar_t ui_cdkey;
extern vmCvar_t ui_cdkeychecked;
extern vmCvar_t ui_ioq3;

//
// ui_qmenu.c
//

#define RCOLUMN_OFFSET (BIGCHAR_WIDTH)
#define LCOLUMN_OFFSET (-BIGCHAR_WIDTH)

// SLIDER_RANGE
// MAX_EDIT_LINE
// MAX_MENUDEPTH
// MAX_MENUITEMS

// MTYPE indexes
// QMF flags

// callback notifications
// QM_ flags

// menuframework_s
// menucommon_s
// Field
// menufield_s
// menuslider_s
// menulist_s
// menuaction_s
// menuradiobutton_s
// menubitmap_s
// menutext_s

extern void        Menu_Cache(void);
extern void        Menu_Focus(MenuCommon* m);
extern void        Menu_AddItem(MenuFw* menu, void* item);
extern void        Menu_AdjustCursor(MenuFw* menu, int dir);
extern void        Menu_Draw(MenuFw* menu);
extern void*       Menu_ItemAtCursor(MenuFw* m);
extern sfxHandle_t Menu_ActivateItem(MenuFw* s, MenuCommon* item);
extern void        Menu_SetCursor(MenuFw* s, int cursor);
extern void        Menu_SetCursorToItem(MenuFw* m, void* ptr);
extern sfxHandle_t Menu_DefaultKey(MenuFw* s, int key);
extern void        Bitmap_Init(MenuImage* b);
extern void        Bitmap_Draw(MenuImage* b);
extern void        ScrollList_Draw(MenuList* l);
extern sfxHandle_t ScrollList_Key(MenuList* l, int key);
extern sfxHandle_t menu_in_sound;
extern sfxHandle_t menu_move_sound;
extern sfxHandle_t menu_out_sound;
extern sfxHandle_t menu_buzz_sound;
extern sfxHandle_t menu_null_sound;
extern sfxHandle_t weaponChangeSound;
extern vec4_t      menu_text_color;
extern vec4_t      menu_grayed_color;
extern vec4_t      menu_dark_color;
extern vec4_t      menu_highlight_color;
extern vec4_t      menu_red_color;
extern vec4_t      menu_black_color;
extern vec4_t      menu_dim_color;
extern vec4_t      color_black;
extern vec4_t      color_gray50;
extern vec4_t      color_white;
extern vec4_t      color_yellow;
extern vec4_t      color_blue;
extern vec4_t      color_orange;
extern vec4_t      color_red;
extern vec4_t      color_dim;
extern vec4_t      name_color;
extern vec4_t      list_color;
extern vec4_t      q3color.listbar;
extern vec4_t      q3color.text_disabled;
extern vec4_t      q3color.text_normal;
extern vec4_t      q3color.text_highlight;

extern char* ui_medalNames[];
extern char* ui_medalPicNames[];
extern char* ui_medalSounds[];

//
// ui_mfield.c
//
extern void        MField_Clear(Field* edit);
extern void        MField_KeyDownEvent(Field* edit, int key);
extern void        MField_CharEvent(Field* edit, int ch);
extern void        MField_Draw(Field* edit, int x, int y, int style, vec4_t color);
extern void        MenuField_Init(MenuField* m);
extern void        MenuField_Draw(MenuField* f);
extern sfxHandle_t MenuField_Key(MenuField* m, int* key);

//
// ui_menu.c
//
extern void MainMenu_Cache(void);
extern void UI_MainMenu(void);
extern void UI_RegisterCvars(void);
extern void UI_UpdateCvars(void);

//
// ui_credits.c
//
extern void UI_CreditMenu(void);

//
// ui_ingame.c
//
extern void InGame_Cache(void);
extern void UI_InGameMenu(void);

//
// ui_confirm.c
//
extern void ConfirmMenu_Cache(void);
extern void UI_ConfirmMenu(const char* question, void (*draw)(void), void (*action)(qboolean result));
extern void UI_ConfirmMenu_Style(const char* question, int style, void (*draw)(void), void (*action)(qboolean result));
extern void UI_Message(const char** lines);

//
// ui_setup.c
//
extern void UI_SetupMenu_Cache(void);
extern void UI_SetupMenu(void);

//
// ui_team.c
//
extern void UI_TeamMainMenu(void);
extern void TeamMain_Cache(void);

//
// ui_connect.c
//
extern void UI_DrawConnectScreen(qboolean overlay);

//
// ui_controls2.c
//
extern void UI_ControlsMenu(void);
extern void Controls_Cache(void);

//
// ui_demo2.c
//
extern void UI_DemosMenu(void);
extern void Demos_Cache(void);

//
// ui_cinematics.c
//
extern void UI_CinematicsMenu(void);
extern void UI_CinematicsMenu_f(void);
extern void UI_CinematicsMenu_Cache(void);

//
// ui_mods.c
//
extern void UI_ModsMenu(void);
extern void UI_ModsMenu_Cache(void);

//
// ui_cdkey.c
//
extern void UI_CDKeyMenu(void);
extern void UI_CDKeyMenu_Cache(void);
extern void UI_CDKeyMenu_f(void);

//
// ui_playermodel.c
//
extern void UI_PlayerModelMenu(void);
extern void PlayerModel_Cache(void);

//
// ui_playersettings.c
//
extern void UI_PlayerSettingsMenu(void);
extern void PlayerSettings_Cache(void);

//
// ui_preferences.c
//
extern void UI_PreferencesMenu(void);
extern void Preferences_Cache(void);

//
// ui_specifyleague.c
//
extern void UI_SpecifyLeagueMenu(void);
extern void SpecifyLeague_Cache(void);

//
// ui_specifyserver.c
//
extern void UI_SpecifyServerMenu(void);
extern void SpecifyServer_Cache(void);

//
// ui_servers2.c
//
#define MAX_FAVORITESERVERS 16

extern void UI_ArenaServersMenu(void);
extern void ArenaServers_Cache(void);

//
// ui_startserver.c
//
extern void UI_StartServerMenu(qboolean multiplayer);
extern void StartServer_Cache(void);
extern void ServerOptions_Cache(void);
extern void UI_BotSelectMenu(char* bot);
extern void UI_BotSelectMenu_Cache(void);

//
// ui_serverinfo.c
//
extern void UI_ServerInfoMenu(void);
extern void ServerInfo_Cache(void);

//
// ui_video.c
//
extern void UI_GraphicsOptionsMenu(void);
extern void GraphicsOptions_Cache(void);
extern void DriverInfo_Cache(void);

//
// ui_players.c
//

// FIXME ripped from cg_local.h
typedef struct {
  int oldFrame;
  int oldFrameTime;  // time when ->oldFrame was exactly on

  int frame;
  int frameTime;  // time when ->frame will be exactly on

  float backlerp;

  float    yawAngle;
  qboolean yawing;
  float    pitchAngle;
  qboolean pitching;

  int          animationNumber;  // may include ANIM_TOGGLEBIT
  animation_t* animation;
  int          animationTime;  // time when the first frame of the animation will be exact
} lerpFrame_t;

typedef struct {
  // model info
  qhandle_t   legsModel;
  qhandle_t   legsSkin;
  lerpFrame_t legs;

  qhandle_t   torsoModel;
  qhandle_t   torsoSkin;
  lerpFrame_t torso;

  qhandle_t headModel;
  qhandle_t headSkin;

  animation_t animations[MAX_ANIMATIONS];

  qboolean fixedlegs;   // true if legs yaw is always the same as torso yaw
  qboolean fixedtorso;  // true if torso never changes yaw

  qhandle_t weaponModel;
  qhandle_t barrelModel;
  qhandle_t flashModel;
  vec3_t    flashDlightColor;
  int       muzzleFlashTime;

  vec3_t color1;
  byte   c1RGBA[4];

  // currently in use drawing parms
  vec3_t   viewAngles;
  vec3_t   moveAngles;
  weapon_t currentWeapon;
  int      legsAnim;
  int      torsoAnim;

  // animation vars
  weapon_t weapon;
  weapon_t lastWeapon;
  weapon_t pendingWeapon;
  int      weaponTimer;
  int      pendingLegsAnim;
  int      torsoAnimationTimer;

  int pendingTorsoAnim;
  int legsAnimationTimer;

  qboolean chat;
  qboolean newModel;

  qboolean barrelSpinning;
  float    barrelAngle;
  int      barrelTime;

  int realWeapon;
} playerInfo_t;

void UI_DrawPlayer(float x, float y, float w, float h, playerInfo_t* pi, int time);
void UI_PlayerInfo_SetModel(playerInfo_t* pi, const char* model);
void UI_PlayerInfo_SetInfo(playerInfo_t* pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNum, qboolean chat);
qboolean UI_RegisterClientModelname(playerInfo_t* pi, const char* modelSkinName);

//
// ui_atoms.c
//

// uiStatic_t

extern void       UI_Init(void);
extern void       UI_Shutdown(void);
extern void       UI_KeyEvent(int key, int down);
extern void       UI_MouseEvent(int dx, int dy);
extern void       UI_Refresh(int realtime);
extern qboolean   UI_ConsoleCommand(int realTime);
extern float      UI_ClampCvar(float min, float max, float value);
extern void       UI_DrawNamedPic(float x, float y, float width, float height, const char* picname);
extern void       UI_DrawHandlePic(float x, float y, float w, float h, qhandle_t hShader);
extern void       UI_FillRect(float x, float y, float width, float height, const float* color);
extern void       UI_DrawRect(float x, float y, float width, float height, const float* color);
extern void       UI_UpdateScreen(void);
extern void       UI_SetColor(const float* rgba);
extern void       UI_LerpColor(vec4_t a, vec4_t b, vec4_t c, float t);
extern void       UI_DrawBannerString(int x, int y, const char* str, int style, vec4_t color);
extern float      UI_ProportionalSizeScale(int style);
extern void       UI_DrawProportionalString(int x, int y, const char* str, int style, vec4_t color);
extern void       UI_DrawProportionalString_AutoWrapped(int x, int ystart, int xmax, int ystep, const char* str, int style, vec4_t color);
extern int        UI_ProportionalStringWidth(const char* str);
extern void       UI_DrawString(int x, int y, const char* str, int style, vec4_t color);
extern void       UI_DrawChar(int x, int y, int ch, int style, vec4_t color);
extern qboolean   UI_CursorInRect(int x, int y, int width, int height);
extern void       UI_AdjustFrom640(float* x, float* y, float* w, float* h);
extern void       UI_DrawTextBox(int x, int y, int width, int lines);
extern qboolean   UI_IsFullscreen(void);
extern void       UI_SetActiveMenu(uiMenuCommand_t menu);
extern void       UI_PushMenu(MenuFw* menu);
extern void       UI_PopMenu(void);
extern void       UI_ForceMenuOff(void);
extern char*      UI_Argv(int arg);
extern char*      UI_Cvar_VariableString(const char* var_name);
extern void       UI_Refresh(int time);
extern void       UI_StartDemoLoop(void);
// m_entersound
// extern uiStatic_t uis;

//
// ui_spLevel.c
//
void UI_SPLevelMenu_Cache(void);
void UI_SPLevelMenu(void);
void UI_SPLevelMenu_f(void);
void UI_SPLevelMenu_ReInit(void);

//
// ui_spArena.c
//
void UI_SPArena_Start(const char* arenaInfo);

//
// ui_spPostgame.c
//
void UI_SPPostgameMenu_Cache(void);
void UI_SPPostgameMenu_f(void);

//
// ui_spSkill.c
//
void UI_SPSkillMenu(const char* arenaInfo);
void UI_SPSkillMenu_Cache(void);

//
// ui_syscalls.c
//
// void        trap_Print(const char* string);
// void        trap_Error(const char* string) __attribute__((noreturn));
// int         trap_Milliseconds(void);
// void        trap_Cvar_Register(vmCvar_t* vmCvar, const char* varName, const char* defaultValue, int flags);
void        trap_Cvar_Update(vmCvar_t* vmCvar);
void        trap_Cvar_Set(const char* var_name, const char* value);
float       trap_Cvar_VariableValue(const char* var_name);
void        trap_Cvar_VariableStringBuffer(const char* var_name, char* buffer, int bufsize);
void        trap_Cvar_SetValue(const char* var_name, float value);
void        trap_Cvar_Reset(const char* name);
void        trap_Cvar_Create(const char* var_name, const char* var_value, int flags);
void        trap_Cvar_InfoStringBuffer(int bit, char* buffer, int bufsize);
int         trap_Argc(void);
void        trap_Argv(int n, char* buffer, int bufferLength);
void        trap_Cmd_ExecuteText(int exec_when, const char* text);  // don't use EXEC_NOW!
int         trap_FS_FOpenFile(const char* qpath, fileHandle_t* f, fsMode_t mode);
void        trap_FS_Read(void* buffer, int len, fileHandle_t f);
void        trap_FS_Write(const void* buffer, int len, fileHandle_t f);
void        trap_FS_FCloseFile(fileHandle_t f);
int         trap_FS_GetFileList(const char* path, const char* extension, char* listbuf, int bufsize);
int         trap_FS_Seek(fileHandle_t f, long offset, int origin);  // fsOrigin_t
qhandle_t   trap_R_RegisterModel(const char* name);
qhandle_t   trap_R_RegisterSkin(const char* name);
qhandle_t   trap_R_RegisterShaderNoMip(const char* name);
void        trap_R_ClearScene(void);
void        trap_R_AddRefEntityToScene(const refEntity_t* re);
void        trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t* verts);
void        trap_R_AddLightToScene(const vec3_t org, float intensity, float r, float g, float b);
void        trap_R_RenderScene(const refdef_t* fd);
void        trap_R_SetColor(const float* rgba);
void        trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader);
void        trap_UpdateScreen(void);
int         trap_CM_LerpTag(orientation_t* tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char* tagName);
void        trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum);
sfxHandle_t trap_S_RegisterSound(const char* sample, qboolean compressed);
void        trap_Key_KeynumToStringBuf(int keynum, char* buf, int buflen);
void        trap_Key_GetBindingBuf(int keynum, char* buf, int buflen);
void        trap_Key_SetBinding(int keynum, const char* binding);
qboolean    trap_Key_IsDown(int keynum);
qboolean    trap_Key_GetOverstrikeMode(void);
void        trap_Key_SetOverstrikeMode(qboolean state);
void        trap_Key_ClearStates(void);
int         trap_Key_GetCatcher(void);
void        trap_Key_SetCatcher(int catcher);
void        trap_GetClipboardData(char* buf, int bufsize);
void        trap_GetClientState(uiClientState_t* state);
// void        trap_GetGlconfig(glconfig_t* glconfig);
int         trap_GetConfigString(int index, char* buff, int buffsize);
int         trap_LAN_GetServerCount(int source);
void        trap_LAN_GetServerAddressString(int source, int n, char* buf, int buflen);
void        trap_LAN_GetServerInfo(int source, int n, char* buf, int buflen);
int         trap_LAN_GetPingQueueCount(void);
int         trap_LAN_ServerStatus(const char* serverAddress, char* serverStatus, int maxLen);
void        trap_LAN_ClearPing(int n);
void        trap_LAN_GetPing(int n, char* buf, int buflen, int* pingtime);
void        trap_LAN_GetPingInfo(int n, char* buf, int buflen);
int         trap_MemoryRemaining(void);
void        trap_GetCDKey(char* buf, int buflen);
void        trap_SetCDKey(char* buf);

qboolean trap_VerifyCDKey(const char* key, const char* chksum);

void trap_SetPbClStatus(int status);

//
// ui_addbots.c
//
void UI_AddBots_Cache(void);
void UI_AddBotsMenu(void);

//
// ui_removebots.c
//
void UI_RemoveBots_Cache(void);
void UI_RemoveBotsMenu(void);

//
// ui_teamorders.c
//
extern void UI_TeamOrdersMenu(void);
extern void UI_TeamOrdersMenu_f(void);
extern void UI_TeamOrdersMenu_Cache(void);

//
// ui_loadconfig.c
//
void UI_LoadConfig_Cache(void);
void UI_LoadConfigMenu(void);

//
// ui_saveconfig.c
//
void UI_SaveConfigMenu_Cache(void);
void UI_SaveConfigMenu(void);

//
// ui_display.c
//
void UI_DisplayOptionsMenu_Cache(void);
void UI_DisplayOptionsMenu(void);

//
// ui_sound.c
//
void UI_SoundOptionsMenu_Cache(void);
void UI_SoundOptionsMenu(void);

//
// ui_network.c
//
void UI_NetworkOptionsMenu_Cache(void);
void UI_NetworkOptionsMenu(void);

//
// ui_gameinfo.c
//
typedef enum { AWARD_ACCURACY, AWARD_IMPRESSIVE, AWARD_EXCELLENT, AWARD_GAUNTLET, AWARD_FRAGS, AWARD_PERFECT } awardType_t;

const char* UI_GetArenaInfoByNumber(int num);
const char* UI_GetArenaInfoByMap(const char* map);
const char* UI_GetSpecialArenaInfo(const char* tag);
int         UI_GetNumArenas(void);
int         UI_GetNumSPArenas(void);
int         UI_GetNumSPTiers(void);

char* UI_GetBotInfoByNumber(int num);
char* UI_GetBotInfoByName(const char* name);
int   UI_GetNumBots(void);

void     UI_GetBestScore(int level, int* score, int* skill);
void     UI_SetBestScore(int level, int score);
int      UI_TierCompleted(int levelWon);
qboolean UI_ShowTierVideo(int tier);
qboolean UI_CanShowTierVideo(int tier);
int      UI_GetCurrentGame(void);
void     UI_NewGame(void);
void     UI_LogAwardData(int award, int data);
int      UI_GetAwardLevel(int award);

void UI_SPUnlock_f(void);
void UI_SPUnlockMedals_f(void);

// void UI_InitGameinfo(void);

// GRank
//
// ui_rankings.c
//
void Rankings_DrawText(void* self);
void Rankings_DrawName(void* self);
void Rankings_DrawPassword(void* self);
void Rankings_Cache(void);
void UI_RankingsMenu(void);

//
// ui_login.c
//
void Login_Cache(void);
void UI_LoginMenu(void);

//
// ui_signup.c
//
void Signup_Cache(void);
void UI_SignupMenu(void);

//
// ui_rankstatus.c
//
void RankStatus_Cache(void);
void UI_RankStatusMenu(void);
