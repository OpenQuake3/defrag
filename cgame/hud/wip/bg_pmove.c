#include "bg_pmove.h"
#include "cg_local.h"
#include <stdlib.h>

/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============*/
float PM_CmdScale(playerState_t const* ps, usercmd_t const* cmd) {
  int32_t max = abs(cmd->forwardmove);
  if (abs(cmd->rightmove) > max) { max = abs(cmd->rightmove); }
  if (abs(cmd->upmove) > max)    { max = abs(cmd->upmove); }
  if (!max)                      { return 0; }
  float const total = sqrtf((float)(cmd->forwardmove * cmd->forwardmove + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove));
  return (float)ps->speed * max / (127.f * total);
}

/* PM_CmdScale without upmove */
float PM_AltCmdScale(playerState_t const* ps, usercmd_t const* cmd) {
  int32_t max = abs(cmd->forwardmove);
  if (abs(cmd->rightmove) > max) { max = abs(cmd->rightmove); }
  // Skips upmove
  if (!max)                      { return 0; }
  float const total = sqrtf((float)(cmd->forwardmove * cmd->forwardmove + cmd->rightmove * cmd->rightmove));
  return (float)ps->speed * max / (127.f * total);
}

//:::::::::::::::::
// hud_CheckJump
//   Copy/Paste of PM_
//   Skips animations and sound
//:::::::::::::::::
// qboolean hud_CheckJump(pmove_t* pm, playerState_t* ps, pml_t* pml) {
//   if (ps->pm_flags & PMF_RESPAWNED) {
//     return qfalse; // don't allow jump until all buttons are up
//   }

//   if (pm->cmd.upmove < 10) {
//     // not holding jump
//     return qfalse;
//   }

//   // must wait for jump to be released
//   if (ps->pm_flags & PMF_JUMP_HELD) {
//     // clear upmove so cmdscale doesn't lower running speed
//     pm->cmd.upmove = 0;
//     return qfalse;
//   }

//   pml->groundPlane = qfalse; // jumping away
//   pml->walking = qfalse;
//   ps->pm_flags |= PMF_JUMP_HELD;

//   ps->groundEntityNum = ENTITYNUM_NONE;
//   ps->velocity[2] = JUMP_VELOCITY;
//   // Skips event and animations
//   return qtrue;
// }

//:::::::::::::::::
// hud_CorrectAllSolid
//   Copy/Paste of PM_
//:::::::::::::::::
// static qboolean hud_CorrectAllSolid(pmove_t* pm, playerState_t* ps, pml_t* pml, trace_t* trace) {
//   vec3_t point;

//   // if (pm->debugLevel)
//   // {
//   //   Com_Printf("%i:allsolid\n", c_pmove);
//   // }

//   // jitter around
//   for (int8_t i = -1; i <= 1; ++i) {
//     for (int8_t j = -1; j <= 1; ++j) {
//       for (int8_t k = -1; k <= 1; ++k) {
//         VectorCopy(ps->origin, point);
//         point[0] += (float)i;
//         point[1] += (float)j;
//         point[2] += (float)k;
//         trap_CM_BoxTrace(trace, point, point, pm->mins, pm->maxs, 0, pm->tracemask);
//         if (!trace->allsolid) {
//           point[0] = ps->origin[0];
//           point[1] = ps->origin[1];
//           point[2] = ps->origin[2] - .25f;

//           trap_CM_BoxTrace(trace, ps->origin, point, pm->mins, pm->maxs, 0, pm->tracemask);
//           pml->groundTrace = *trace;
//           return qtrue;
//         }
//       }
//     }
//   }

//   ps->groundEntityNum = ENTITYNUM_NONE;
//   pml->groundPlane    = qfalse;
//   pml->walking        = qfalse;
//   return qfalse;
// }

//:::::::::::::::::
// hud_GroundTraceMissed
//   Copy/Paste of PM_
//   without the animations
//:::::::::::::::::
// static void hud_GroundTraceMissed(playerState_t* ps, pml_t* pml) {
//   ps->groundEntityNum = ENTITYNUM_NONE;
//   pml->groundPlane    = qfalse;
//   pml->walking        = qfalse;
// }

//:::::::::::::::::
// hud_GroundTrace
//   Copy/Paste of PM_ but skips:
//   Animations, Crashland, and debug prints
//:::::::::::::::::
// void hud_GroundTrace(pmove_t* pm, playerState_t* ps, pml_t* pml) {
//   vec3_t  point;
//   trace_t trace;

//   point[0] = ps->origin[0];
//   point[1] = ps->origin[1];
//   point[2] = ps->origin[2] - .25f;

//   trap_CM_BoxTrace(&trace, ps->origin, point, pm->mins, pm->maxs, 0, pm->tracemask);
//   pml->groundTrace = trace;

//   // do something corrective if the trace starts in a solid...
//   if (trace.allsolid) {
//     if (!hud_CorrectAllSolid(pm, ps, pml, &trace)) return;
//   }

//   // if the trace didn't hit anything, we are in free fall
//   if (trace.fraction == 1.f) {
//     hud_GroundTraceMissed(ps, pml);
//     // Doesnt set pml.groundplane
//     // Doesnt set pml.walking
//     return;
//   }

//   // check if getting thrown off the ground
//   if (ps->velocity[2] > 0 && DotProduct(ps->velocity, trace.plane.normal) > 10) {
//     // if ( pm->debugLevel ) {
//     //   Com_Printf("%i:kickoff\n", c_pmove);
//     // }
//     ps->groundEntityNum = ENTITYNUM_NONE;
//     pml->groundPlane    = qfalse;
//     pml->walking        = qfalse;
//     return;
//   }

//   // slopes that are too steep will not be considered onground
//   if (trace.plane.normal[2] < MIN_WALK_NORMAL)
//   {
//     // if ( pm->debugLevel ) {
//     //   Com_Printf("%i:steep\n", c_pmove);
//     // }
//     // FIXME: if they can't slide down the slope, let them
//     // walk (sharp crevices)
//     ps->groundEntityNum = ENTITYNUM_NONE;
//     pml->groundPlane       = qtrue;
//     pml->walking           = qfalse;
//     return;
//   }

//   pml->groundPlane = qtrue;
//   pml->walking     = qtrue;

//   // hitting solid ground will end a waterjump
//   if (ps->pm_flags & PMF_TIME_WATERJUMP)
//   {
//     ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
//     ps->pm_time = 0;
//   }

//   if (ps->groundEntityNum == ENTITYNUM_NONE)
//   {
//     // just hit the ground
//     // if ( pm->debugLevel ) {
//     //   Com_Printf("%i:Land\n", c_pmove);
//     // }

//     // PM_CrashLand();

//     // don't do landing time if we were just going down a slope
//     if (pml->previous_velocity[2] < -200)
//     {
//       // don't allow another jump for a little while
//       ps->pm_flags |= PMF_TIME_LAND;
//       ps->pm_time = 250;
//     }
//   }

//   ps->groundEntityNum = trace.entityNum;

//   // don't reset the z velocity for slopes
//   // pm.ps->velocity[2] = 0;

//   // PM_AddTouchEnt(trace.entityNum);
// }

//:::::::::::::::::
// hud_SetWaterLevel
//   Copy/Paste of PM_
//:::::::::::::::::
// void hud_SetWaterLevel(pmove_t* pm, playerState_t* ps) {
//   vec3_t point;
//   int    cont;
//   int    sample1;
//   int    sample2;

//   // get waterlevel, accounting for ducking
//   pm->waterlevel = 0;
//   pm->watertype  = 0;

//   point[0] = ps->origin[0];
//   point[1] = ps->origin[1];
//   point[2] = ps->origin[2] + MINS_Z + 1;

//   cont = trap_CM_PointContents(point, 0);

//   if (cont & MASK_WATER)
//   {
//     sample2 = ps->viewheight - MINS_Z;
//     sample1 = sample2 / 2;

//     pm->watertype  = cont;
//     pm->waterlevel = 1;
//     point[2]       = ps->origin[2] + MINS_Z + sample1;
//     cont           = trap_CM_PointContents(point, 0);
//     if (cont & MASK_WATER)
//     {
//       pm->waterlevel = 2;
//       point[2]       = ps->origin[2] + MINS_Z + sample2;
//       cont           = trap_CM_PointContents(point, 0);
//       if (cont & MASK_WATER)
//       {
//         pm->waterlevel = 3;
//       }
//     }
//   }
// }

//:::::::::::::::::
// hud_CheckDuck
//   Copy/Paste of PM_
//:::::::::::::::::
// void hud_CheckDuck(pmove_t* pm, playerState_t* ps) {
//   trace_t trace;

//   if (ps->powerups[PW_INVULNERABILITY])  {
//     if (ps->pm_flags & PMF_INVULEXPAND) {
//       // invulnerability sphere has a 42 units radius
//       VectorSet(pm->mins, -42, -42, -42);
//       VectorSet(pm->maxs, 42, 42, 42);
//     } else {
//       VectorSet(pm->mins, -15, -15, MINS_Z);
//       VectorSet(pm->maxs, 15, 15, 16);
//     }
//     ps->pm_flags |= PMF_DUCKED;
//     ps->viewheight = CROUCH_VIEWHEIGHT;
//     return;
//   }
//   ps->pm_flags &= ~PMF_INVULEXPAND;

//   pm->mins[0] = -15;
//   pm->mins[1] = -15;

//   pm->maxs[0] = 15;
//   pm->maxs[1] = 15;

//   pm->mins[2] = MINS_Z;

//   if (ps->pm_type == PM_DEAD) {
//     pm->maxs[2]       = -8;
//     ps->viewheight = DEAD_VIEWHEIGHT;
//     return;
//   }

//   if (pm->cmd.upmove < 0) { // duck
//     ps->pm_flags |= PMF_DUCKED;
//   } else { // stand up if possible
//     if (ps->pm_flags & PMF_DUCKED) {
//       // try to stand up
//       pm->maxs[2] = 32;
//       trap_CM_BoxTrace(&trace, ps->origin, ps->origin, pm->mins, pm->maxs, 0, pm->tracemask);
//       if (!trace.allsolid) ps->pm_flags &= ~PMF_DUCKED;
//     }
//   }

//   if (ps->pm_flags & PMF_DUCKED) {
//     pm->maxs[2]    = 16;
//     ps->viewheight = CROUCH_VIEWHEIGHT;
//   } else {
//     pm->maxs[2]    = 32;
//     ps->viewheight = DEFAULT_VIEWHEIGHT;
//   }
// }
