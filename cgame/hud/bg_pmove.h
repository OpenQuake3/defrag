#ifndef BG_PMOVE_H
#define BG_PMOVE_H

#include "bg_local.h"
#include "bg_public.h"

float PM_CmdScale(playerState_t const* pm_ps, usercmd_t const* cmd);
float PM_AltCmdScale(playerState_t const* pm_ps, usercmd_t const* cmd);

qboolean PM_CheckJump(pmove_t* pm, playerState_t* pm_ps, pml_t* pml);

void PM_GroundTrace(pmove_t* pm, playerState_t* pm_ps, pml_t* pml);

void PM_SetWaterLevel(pmove_t* pm, playerState_t* pm_ps);

void PM_CheckDuck(pmove_t* pm, playerState_t* pm_ps);

#endif // BG_PMOVE_H
