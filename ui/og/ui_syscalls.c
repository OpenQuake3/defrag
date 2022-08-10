

// Passf
// id3Print
// id3Error
// id3Milliseconds
// id3Cvar_Register
// id3Cvar_Set
// id3Cvar_Update
float id3Cvar_VariableValue(const char* var_name) {
  floatint_t fi;
  fi.i = callback(UI_CVAR_VARIABLEVALUE, var_name);
  return fi.f;
}
// id3Cvar_VariableStringBuffer
void id3Cvar_SetValue(const char* var_name, float value) { callback(UI_CVAR_SETVALUE, var_name, PASSFLOAT(value)); }
void id3Cvar_Reset(const char* name) { callback(UI_CVAR_RESET, name); }
void id3Cvar_Create(const char* var_name, const char* var_value, int flags) { callback(UI_CVAR_CREATE, var_name, var_value, flags); }
void id3Cvar_InfoStringBuffer(int bit, char* buffer, int bufsize) { callback(UI_CVAR_INFOSTRINGBUFFER, bit, buffer, bufsize); }

int  id3Argc(void) { return callback(UI_ARGC); }
void id3Argv(int n, char* buffer, int bufferLength) { callback(UI_ARGV, n, buffer, bufferLength); }
// id3Cmd_ExecuteText

int  id3FS_FOpenFile(const char* qpath, fileHandle_t* f, fsMode_t mode) { return callback(UI_FS_FOPENFILE, qpath, f, mode); }
void id3FS_Read(void* buffer, int len, fileHandle_t f) { callback(UI_FS_READ, buffer, len, f); }
void id3FS_Write(const void* buffer, int len, fileHandle_t f) { callback(UI_FS_WRITE, buffer, len, f); }
void id3FS_FCloseFile(fileHandle_t f) { callback(UI_FS_FCLOSEFILE, f); }
int  id3FS_GetFileList(const char* path, const char* extension, char* listbuf, int bufsize) {
   return callback(UI_FS_GETFILELIST, path, extension, listbuf, bufsize);
}
int id3FS_Seek(fileHandle_t f, long offset, int origin) { return callback(UI_FS_SEEK, f, offset, origin); }

// id3R_RegisterModel
qhandle_t id3R_RegisterSkin(const char* name) { return callback(UI_R_REGISTERSKIN, name); }
void      id3R_RegisterFont(const char* fontName, int pointSize, fontInfo_t* font) { callback(UI_R_REGISTERFONT, fontName, pointSize, font); }
// id3R_RegisterShaderNoMip
// id3R_ClearScene
void id3R_AddRefEntityToScene(const refEntity_t* re) { callback(UI_R_ADDREFENTITYTOSCENE, re); }
void id3R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t* verts) { callback(UI_R_ADDPOLYTOSCENE, hShader, numVerts, verts); }
void id3R_AddLightToScene(const vec3_t org, float intensity, float r, float g, float b) {
  callback(UI_R_ADDLIGHTTOSCENE, org, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b));
}
// id3R_RenderScene
// id3R_SetColor
// id3R_DrawStretchPic
void id3R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs) { callback(UI_R_MODELBOUNDS, model, mins, maxs); }
// id3UpdateScreen

int id3CM_LerpTag(orientation_t* tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char* tagName) {
  return callback(UI_CM_LERPTAG, tag, mod, startFrame, endFrame, PASSFLOAT(frac), tagName);
}

// id3S_StartLocalSound
// id3S_RegisterSound
void id3S_StopBackgroundTrack(void) { callback(UI_S_STOPBACKGROUNDTRACK); }
void id3S_StartBackgroundTrack(const char* intro, const char* loop) { callback(UI_S_STARTBACKGROUNDTRACK, intro, loop); }

void id3Key_KeynumToStringBuf(int keynum, char* buf, int buflen) { callback(UI_KEY_KEYNUMTOSTRINGBUF, keynum, buf, buflen); }
void id3Key_GetBindingBuf(int keynum, char* buf, int buflen) { callback(UI_KEY_GETBINDINGBUF, keynum, buf, buflen); }
void id3Key_SetBinding(int keynum, const char* binding) { callback(UI_KEY_SETBINDING, keynum, binding); }
// id3Key_IsDown
// id3Key_GetOverstrikeMode
// id3Key_SetOverstrikeMode
// id3Key_ClearStates
// id3Key_GetCatcher
// id3Key_SetCatcher

// id3GetClipboardData
// id3GetClientState
// id3GetGlconfig
int id3GetConfigString(int index, char* buff, int buffsize) { return callback(UI_GETCONFIGSTRING, index, buff, buffsize); }

int  id3LAN_GetServerCount(int source) { return callback(UI_LAN_GETSERVERCOUNT, source); }
void id3LAN_GetServerAddressString(int source, int n, char* buf, int buflen) { callback(UI_LAN_GETSERVERADDRESSSTRING, source, n, buf, buflen); }
void id3LAN_GetServerInfo(int source, int n, char* buf, int buflen) { callback(UI_LAN_GETSERVERINFO, source, n, buf, buflen); }
int  id3LAN_GetServerPing(int source, int n) { return callback(UI_LAN_GETSERVERPING, source, n); }
int  id3LAN_GetPingQueueCount(void) { return callback(UI_LAN_GETPINGQUEUECOUNT); }
int  id3LAN_ServerStatus(const char* serverAddress, char* serverStatus, int maxLen) {
   return callback(UI_LAN_SERVERSTATUS, serverAddress, serverStatus, maxLen);
}
void id3LAN_SaveCachedServers(void) { callback(UI_LAN_SAVECACHEDSERVERS); }
void id3LAN_LoadCachedServers(void) { callback(UI_LAN_LOADCACHEDSERVERS); }
void id3LAN_ResetPings(int n) { callback(UI_LAN_RESETPINGS, n); }
void id3LAN_ClearPing(int n) { callback(UI_LAN_CLEARPING, n); }
void id3LAN_GetPing(int n, char* buf, int buflen, int* pingtime) { callback(UI_LAN_GETPING, n, buf, buflen, pingtime); }
void id3LAN_GetPingInfo(int n, char* buf, int buflen) { callback(UI_LAN_GETPINGINFO, n, buf, buflen); }
void id3LAN_MarkServerVisible(int source, int n, bool visible) { callback(UI_LAN_MARKSERVERVISIBLE, source, n, visible); }
int  id3LAN_ServerIsVisible(int source, int n) { return callback(UI_LAN_SERVERISVISIBLE, source, n); }
bool id3LAN_UpdateVisiblePings(int source) { return callback(UI_LAN_UPDATEVISIBLEPINGS, source); }
int  id3LAN_AddServer(int source, const char* name, const char* addr) { return callback(UI_LAN_ADDSERVER, source, name, addr); }
void id3LAN_RemoveServer(int source, const char* addr) { callback(UI_LAN_REMOVESERVER, source, addr); }
int  id3LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2) {
   return callback(UI_LAN_COMPARESERVERS, source, sortKey, sortDir, s1, s2);
}

int id3MemoryRemaining(void) { return callback(UI_MEMORY_REMAINING); }

void id3GetCDKey(char* buf, int buflen) { callback(UI_GET_CDKEY, buf, buflen); }
void id3SetCDKey(char* buf) { callback(UI_SET_CDKEY, buf); }
bool id3VerifyCDKey(const char* key, const char* chksum) { return callback(UI_VERIFY_CDKEY, key, chksum); }

int id3PC_AddGlobalDefine(char* define) { return callback(UI_PC_ADD_GLOBAL_DEFINE, define); }
int id3PC_LoadSource(const char* filename) { return callback(UI_PC_LOAD_SOURCE, filename); }
int id3PC_FreeSource(int handle) { return callback(UI_PC_FREE_SOURCE, handle); }
int id3PC_ReadToken(int handle, pc_token_t* pc_token) { return callback(UI_PC_READ_TOKEN, handle, pc_token); }
int id3PC_SourceFileAndLine(int handle, char* filename, int* line) { return callback(UI_PC_SOURCE_FILE_AND_LINE, handle, filename, line); }

int id3RealTime(qtime_t* qtime) { return callback(UI_REAL_TIME, qtime); }

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to false (do not alter gamestate)
int id3CIN_PlayCinematic(const char* arg0, int xpos, int ypos, int width, int height, int bits) {
  return callback(UI_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits);
}
// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status id3CIN_StopCinematic(int handle) { return callback(UI_CIN_STOPCINEMATIC, handle); }
// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status id3CIN_RunCinematic(int handle) { return callback(UI_CIN_RUNCINEMATIC, handle); }
// draws the current frame
void id3CIN_DrawCinematic(int handle) { callback(UI_CIN_DRAWCINEMATIC, handle); }
// allows you to resize the animation dynamically
void id3CIN_SetExtents(int handle, int x, int y, int w, int h) { callback(UI_CIN_SETEXTENTS, handle, x, y, w, h); }
void id3R_RemapShader(const char* oldShader, const char* newShader, const char* timeOffset) {
  callback(UI_R_REMAP_SHADER, oldShader, newShader, timeOffset);
}

void id3SetPbClStatus(int status) { callback(UI_SET_PBCLSTATUS, status); }
