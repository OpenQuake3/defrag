#ifndef BG_PMOVE_H
#define BG_PMOVE_H

#include "bg_local.h"
#include "bg_public.h"

float PM_CmdScale(playerState_t const* ps, usercmd_t const* cmd);
float PM_AltCmdScale(playerState_t const* ps, usercmd_t const* cmd);

qboolean hud_CheckJump(pmove_t* pm, playerState_t* ps, pml_t* pml);
void hud_GroundTrace(pmove_t* pm, playerState_t* ps, pml_t* pml);
void hud_SetWaterLevel(pmove_t* pm, playerState_t* ps);
void hud_CheckDuck(pmove_t* pm, playerState_t* ps);

#endif // BG_PMOVE_H
