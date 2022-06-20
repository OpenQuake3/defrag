/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t *pm;
pml_t pml;

// movement parameter
float pm_stopspeed = 100.0f;
float pm_duckScale = 0.25f;
float pm_swimScale = 0.50f;

float pm_accelerate = 10.0f;
float pm_airaccelerate = 1.0f;
float pm_wateraccelerate = 4.0f;
float pm_flyaccelerate = 8.0f;

float pm_friction = 6.0f;
float pm_waterfriction = 1.0f;
float pm_flightfriction = 3.0f;
float pm_spectatorfriction = 5.0f;

int c_pmove = 0;

/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent(int newEvent) {
  BG_AddPredictableEventToPlayerstate(newEvent, 0, pm->ps);
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt(int entityNum) {
  int i;

  if (entityNum == ENTITYNUM_WORLD) {
    return;
  }
  if (pm->numtouch == MAXTOUCH) {
    return;
  }

  // see if it is already added
  for (i = 0; i < pm->numtouch; i++) {
    if (pm->touchents[i] == entityNum) {
      return;
    }
  }

  // add it
  pm->touchents[pm->numtouch] = entityNum;
  pm->numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim(int anim) {
  if (pm->ps->pm_type >= PM_DEAD) {
    return;
  }
  pm->ps->torsoAnim =
      ((pm->ps->torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;
}
static void PM_StartLegsAnim(int anim) {
  if (pm->ps->pm_type >= PM_DEAD) {
    return;
  }
  if (pm->ps->legsTimer > 0) {
    return; // a high priority animation is running
  }
  pm->ps->legsAnim =
      ((pm->ps->legsAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;
}

static void PM_ContinueLegsAnim(int anim) {
  if ((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == anim) {
    return;
  }
  if (pm->ps->legsTimer > 0) {
    return; // a high priority animation is running
  }
  PM_StartLegsAnim(anim);
}

static void PM_ContinueTorsoAnim(int anim) {
  if ((pm->ps->torsoAnim & ~ANIM_TOGGLEBIT) == anim) {
    return;
  }
  if (pm->ps->torsoTimer > 0) {
    return; // a high priority animation is running
  }
  PM_StartTorsoAnim(anim);
}

static void PM_ForceLegsAnim(int anim) {
  pm->ps->legsTimer = 0;
  PM_StartLegsAnim(anim);
}

/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
  float backoff;
  float change;
  int i;

  backoff = DotProduct(in, normal);

  if (backoff < 0) {
    backoff *= overbounce;
  } else {
    backoff /= overbounce;
  }

  for (i = 0; i < 3; i++) {
    change = normal[i] * backoff;
    out[i] = in[i] - change;
  }
}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction(void) {
  vec3_t vec;
  float *vel;
  float speed, newspeed, control;
  float drop;

  vel = pm->ps->velocity;

  VectorCopy(vel, vec);
  if (pml.walking) {
    vec[2] = 0; // ignore slope movement
  }

  speed = VectorLength(vec);
  if (speed < 1) {
    vel[0] = 0;
    vel[1] = 0; // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  drop = 0;

  // apply ground friction
  if (pm->waterlevel <= 1) {
    if (pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK)) {
      // if getting knocked back, no friction
      if (!(pm->ps->pm_flags & PMF_TIME_KNOCKBACK)) {
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * pm_friction * pml.frametime;
      }
    }
  }

  // apply water friction even if just wading
  if (pm->waterlevel) {
    drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
  }

  // apply flying friction
  if (pm->ps->powerups[PW_FLIGHT]) {
    drop += speed * pm_flightfriction * pml.frametime;
  }

  if (pm->ps->pm_type == PM_SPECTATOR) {
    drop += speed * pm_spectatorfriction * pml.frametime;
  }

  // scale the velocity
  newspeed = speed - drop;
  if (newspeed < 0) {
    newspeed = 0;
  }
  newspeed /= speed;

  vel[0] = vel[0] * newspeed;
  vel[1] = vel[1] * newspeed;
  vel[2] = vel[2] * newspeed;
}

/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate(vec3_t wishdir, float wishspeed, float accel) {
#if 1
  // q2 style
  int i;
  float addspeed, accelspeed, currentspeed;

  currentspeed = DotProduct(pm->ps->velocity, wishdir);
  addspeed = wishspeed - currentspeed;
  if (addspeed <= 0) {
    return;
  }
  accelspeed = accel * pml.frametime * wishspeed;
  if (accelspeed > addspeed) {
    accelspeed = addspeed;
  }

  for (i = 0; i < 3; i++) {
    pm->ps->velocity[i] += accelspeed * wishdir[i];
  }
#else
  // proper way (avoids strafe jump maxspeed bug), but feels bad
  vec3_t wishVelocity;
  vec3_t pushDir;
  float pushLen;
  float canPush;

  VectorScale(wishdir, wishspeed, wishVelocity);
  VectorSubtract(wishVelocity, pm->ps->velocity, pushDir);
  pushLen = VectorNormalize(pushDir);

  canPush = accel * pml.frametime * wishspeed;
  if (canPush > pushLen) {
    canPush = pushLen;
  }

  VectorMA(pm->ps->velocity, canPush, pushDir, pm->ps->velocity);
#endif
}

/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale(usercmd_t *cmd) {
  int max;
  float total;
  float scale;

  max = abs(cmd->forwardmove);
  if (abs(cmd->rightmove) > max) {
    max = abs(cmd->rightmove);
  }
  if (abs(cmd->upmove) > max) {
    max = abs(cmd->upmove);
  }
  if (!max) {
    return 0;
  }

  total = sqrt(cmd->forwardmove * cmd->forwardmove +
               cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove);
  scale = (float)pm->ps->speed * max / (127.0 * total);

  return scale;
}

/*
================
PM_SetMovementDir

Determine the rotation of the legs relative
to the facing dir
================
*/
static void PM_SetMovementDir(void) {
  if (pm->cmd.forwardmove || pm->cmd.rightmove) {
    if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0) {
      pm->ps->movementDir = 0;
    } else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0) {
      pm->ps->movementDir = 1;
    } else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0) {
      pm->ps->movementDir = 2;
    } else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0) {
      pm->ps->movementDir = 3;
    } else if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0) {
      pm->ps->movementDir = 4;
    } else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0) {
      pm->ps->movementDir = 5;
    } else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0) {
      pm->ps->movementDir = 6;
    } else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0) {
      pm->ps->movementDir = 7;
    }
  } else {  
    // This block only activates when no keypresses on this frame, but the player has remaining sideways movement from the previous frame
    // If they aren't actively going directly sideways,
    // change the animation to the diagonal so they
    // don't stop too crooked
    if (pm->ps->movementDir == 2) {
      pm->ps->movementDir = 1;
    } else if (pm->ps->movementDir == 6) {
      pm->ps->movementDir = 7;
    }
  }
}

/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump(void) {
  if (pm->ps->pm_flags & PMF_RESPAWNED) {
    return qfalse; // don't allow jump until all buttons are up
  }

  if (pm->cmd.upmove < 10) {
    // not holding jump
    return qfalse;
  }

  // must wait for jump to be released
  if (pm->ps->pm_flags & PMF_JUMP_HELD) {
    // clear upmove so cmdscale doesn't lower running speed
    pm->cmd.upmove = 0;
    return qfalse;
  }

  pml.groundPlane = qfalse; // jumping away
  pml.walking = qfalse;
  pm->ps->pm_flags |= PMF_JUMP_HELD;

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  pm->ps->velocity[2] = JUMP_VELOCITY;
  PM_AddEvent(EV_JUMP);

  if (pm->cmd.forwardmove >= 0) {
    PM_ForceLegsAnim(LEGS_JUMP);
    pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
  } else {
    PM_ForceLegsAnim(LEGS_JUMPB);
    pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
  }

  return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean PM_CheckWaterJump(void) {
  vec3_t spot;
  int cont;
  vec3_t flatforward;

  if (pm->ps->pm_time) {
    return qfalse;
  }

  // check for water jump
  if (pm->waterlevel != 2) {
    return qfalse;
  }

  flatforward[0] = pml.forward[0];
  flatforward[1] = pml.forward[1];
  flatforward[2] = 0;
  VectorNormalize(flatforward);

  VectorMA(pm->ps->origin, 30, flatforward, spot);
  spot[2] += 4;
  cont = pm->pointcontents(spot, pm->ps->clientNum);
  if (!(cont & CONTENTS_SOLID)) {
    return qfalse;
  }

  spot[2] += 16;
  cont = pm->pointcontents(spot, pm->ps->clientNum);
  if (cont & (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY)) {
    return qfalse;
  }

  // jump out of water
  VectorScale(pml.forward, 200, pm->ps->velocity);
  pm->ps->velocity[2] = 350;

  pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
  pm->ps->pm_time = 2000;

  return qtrue;
}

//============================================================================

/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove(void) {
  // waterjump has no control, but falls

  PM_StepSlideMove(qtrue);

  pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  if (pm->ps->velocity[2] < 0) {
    // cancel as soon as we are falling down again
    pm->ps->pm_flags &= ~PMF_ALL_TIMES;
    pm->ps->pm_time = 0;
  }
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove(void) {
  int i;
  vec3_t wishvel;
  float wishspeed;
  vec3_t wishdir;
  float scale;
  float vel;

  if (PM_CheckWaterJump()) {
    PM_WaterJumpMove();
    return;
  }
#if 0
	// jump = head for surface
	if ( pm->cmd.upmove >= 10 ) {
		if (pm->ps->velocity[2] > -300) {
			if ( pm->watertype & CONTENTS_WATER ) {
				pm->ps->velocity[2] = 100;
			} else if ( pm->watertype & CONTENTS_SLIME ) {
				pm->ps->velocity[2] = 80;
			} else {
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
  PM_Friction();

  scale = PM_CmdScale(&pm->cmd);
  //
  // user intentions
  //
  if (!scale) {
    wishvel[0] = 0;
    wishvel[1] = 0;
    wishvel[2] = -60; // sink towards bottom
  } else {
    for (i = 0; i < 3; i++)
      wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove +
                   scale * pml.right[i] * pm->cmd.rightmove;

    wishvel[2] += scale * pm->cmd.upmove;
  }

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);

  if (wishspeed > pm->ps->speed * pm_swimScale) {
    wishspeed = pm->ps->speed * pm_swimScale;
  }

  PM_Accelerate(wishdir, wishspeed, pm_wateraccelerate);

  // make sure we can go up slopes easily under water
  if (pml.groundPlane &&
      DotProduct(pm->ps->velocity, pml.groundTrace.plane.normal) < 0) {
    vel = VectorLength(pm->ps->velocity);
    // slide along the ground plane
    PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal,
                    pm->ps->velocity, OVERCLIP);

    VectorNormalize(pm->ps->velocity);
    VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  }

  PM_SlideMove(qfalse);
}

#ifdef TEAMARENA
/*
===================
PM_InvulnerabilityMove

Only with the invulnerability powerup
===================
*/
static void PM_InvulnerabilityMove(void) {
  pm->cmd.forwardmove = 0;
  pm->cmd.rightmove = 0;
  pm->cmd.upmove = 0;
  VectorClear(pm->ps->velocity);
}
#endif

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove(void) {
  int i;
  vec3_t wishvel;
  float wishspeed;
  vec3_t wishdir;
  float scale;

  // normal slowdown
  PM_Friction();

  scale = PM_CmdScale(&pm->cmd);
  //
  // user intentions
  //
  if (!scale) {
    wishvel[0] = 0;
    wishvel[1] = 0;
    wishvel[2] = 0;
  } else {
    for (i = 0; i < 3; i++) {
      wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove +
                   scale * pml.right[i] * pm->cmd.rightmove;
    }

    wishvel[2] += scale * pm->cmd.upmove;
  }

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);

  PM_Accelerate(wishdir, wishspeed, pm_flyaccelerate);

  PM_StepSlideMove(qfalse);
}

/*
===================
PM_AirMove

===================
*/
static void PM_AirMove(void) {
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  float scale;
  usercmd_t cmd;

  PM_Friction();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  cmd = pm->cmd;
  scale = PM_CmdScale(&cmd);

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  for (i = 0; i < 2; i++) {
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  }
  wishvel[2] = 0;

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;

  // not on ground, so little effect on velocity
  PM_Accelerate(wishdir, wishspeed, pm_airaccelerate);

  // we may have a ground plane that is very steep, even
  // though we don't have a groundentity
  // slide along the steep plane
  if (pml.groundPlane) {
    PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal,
                    pm->ps->velocity, OVERCLIP);
  }

#if 0
	//ZOID:  If we are on the grapple, try stair-stepping
	//this allows a player to use the grapple to pull himself
	//over a ledge
	if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
		PM_StepSlideMove ( qtrue );
	else
		PM_SlideMove ( qtrue );
#endif

  PM_StepSlideMove(qtrue);
}

/*
===================
PM_GrappleMove

===================
*/
static void PM_GrappleMove(void) {
  vec3_t vel, v;
  float vlen;

  VectorScale(pml.forward, -16, v);
  VectorAdd(pm->ps->grapplePoint, v, v);
  VectorSubtract(v, pm->ps->origin, vel);
  vlen = VectorLength(vel);
  VectorNormalize(vel);

  if (vlen <= 100)
    VectorScale(vel, 10 * vlen, vel);
  else
    VectorScale(vel, 800, vel);

  VectorCopy(vel, pm->ps->velocity);

  pml.groundPlane = qfalse;
}

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove(void) {
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  float scale;
  usercmd_t cmd;
  float accelerate;
  float vel;

  if (pm->waterlevel > 2 &&
      DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0) {
    // begin swimming
    PM_WaterMove();
    return;
  }

  if (PM_CheckJump()) {
    // jumped away
    if (pm->waterlevel > 1) {
      PM_WaterMove();
    } else {
      PM_AirMove();
    }
    return;
  }

  PM_Friction();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  cmd = pm->cmd;
  scale = PM_CmdScale(&cmd);

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;

  // project the forward and right directions onto the ground plane
  PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward,
                  OVERCLIP);
  PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
  //
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  for (i = 0; i < 3; i++) {
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  }
  // when going up or down slopes the wish velocity should Not be zero
  //	wishvel[2] = 0;

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;

  // clamp the speed lower if ducking
  if (pm->ps->pm_flags & PMF_DUCKED) {
    if (wishspeed > pm->ps->speed * pm_duckScale) {
      wishspeed = pm->ps->speed * pm_duckScale;
    }
  }

  // clamp the speed lower if wading or walking on the bottom
  if (pm->waterlevel) {
    float waterScale;

    waterScale = pm->waterlevel / 3.0;
    waterScale = 1.0 - (1.0 - pm_swimScale) * waterScale;
    if (wishspeed > pm->ps->speed * waterScale) {
      wishspeed = pm->ps->speed * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK) ||
      pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    accelerate = pm_airaccelerate;
  } else {
    accelerate = pm_accelerate;
  }

  PM_Accelerate(wishdir, wishspeed, accelerate);

  // Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0],
  // pm->ps->velocity[1], pm->ps->velocity[2]); Com_Printf("velocity1 =
  // %1.1f\n", VectorLength(pm->ps->velocity));

  if ((pml.groundTrace.surfaceFlags & SURF_SLICK) ||
      pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  } else {
    // don't reset the z velocity for slopes
    //		pm->ps->velocity[2] = 0;
  }

  vel = VectorLength(pm->ps->velocity);

  // slide along the ground plane
  PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal,
                  pm->ps->velocity, OVERCLIP);

  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
    return;
  }

  PM_StepSlideMove(qfalse);

  // Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));
}

/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove(void) {
  float forward;

  if (!pml.walking) {
    return;
  }

  // extra friction

  forward = VectorLength(pm->ps->velocity);
  forward -= 20;
  if (forward <= 0) {
    VectorClear(pm->ps->velocity);
  } else {
    VectorNormalize(pm->ps->velocity);
    VectorScale(pm->ps->velocity, forward, pm->ps->velocity);
  }
}

/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove(void) {
  float speed, drop, friction, control, newspeed;
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  float scale;

  pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

  // friction

  speed = VectorLength(pm->ps->velocity);
  if (speed < 1) {
    VectorCopy(vec3_origin, pm->ps->velocity);
  } else {
    drop = 0;

    friction = pm_friction * 1.5; // extra friction
    control = speed < pm_stopspeed ? pm_stopspeed : speed;
    drop += control * friction * pml.frametime;

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0)
      newspeed = 0;
    newspeed /= speed;

    VectorScale(pm->ps->velocity, newspeed, pm->ps->velocity);
  }

  // accelerate
  scale = PM_CmdScale(&pm->cmd);

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;

  for (i = 0; i < 3; i++)
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  wishvel[2] += pm->cmd.upmove;

  VectorCopy(wishvel, wishdir);
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;

  PM_Accelerate(wishdir, wishspeed, pm_accelerate);

  // move
  VectorMA(pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number appropriate for the groundsurface
================
*/
static int PM_FootstepForSurface(void) {
  if (pml.groundTrace.surfaceFlags & SURF_NOSTEPS) {
    return 0;
  }
  if (pml.groundTrace.surfaceFlags & SURF_METALSTEPS) {
    return EV_FOOTSTEP_METAL;
  }
  return EV_FOOTSTEP;
}

/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand(void) {
  float delta;
  float dist;
  float vel, acc;
  float t;
  float a, b, c, den;

  // decide which landing animation to use
  if (pm->ps->pm_flags & PMF_BACKWARDS_JUMP) {
    PM_ForceLegsAnim(LEGS_LANDB);
  } else {
    PM_ForceLegsAnim(LEGS_LAND);
  }

  pm->ps->legsTimer = TIMER_LAND;

  // calculate the exact velocity on landing
  dist = pm->ps->origin[2] - pml.previous_origin[2];
  vel = pml.previous_velocity[2];
  acc = -pm->ps->gravity;

  a = acc / 2;
  b = vel;
  c = -dist;

  den = b * b - 4 * a * c;
  if (den < 0) {
    return;
  }
  t = (-b - sqrt(den)) / (2 * a);

  delta = vel + t * acc;
  delta = delta * delta * 0.0001;

  // ducking while falling doubles damage
  if (pm->ps->pm_flags & PMF_DUCKED) {
    delta *= 2;
  }

  // never take falling damage if completely underwater
  if (pm->waterlevel == 3) {
    return;
  }

  // reduce falling damage if there is standing water
  if (pm->waterlevel == 2) {
    delta *= 0.25;
  }
  if (pm->waterlevel == 1) {
    delta *= 0.5;
  }

  if (delta < 1) {
    return;
  }

  // create a local entity event to play the sound

  // SURF_NODAMAGE is used for bounce pads where you don't ever
  // want to take damage or play a crunch sound
  if (!(pml.groundTrace.surfaceFlags & SURF_NODAMAGE)) {
    if (delta > 60) {
      PM_AddEvent(EV_FALL_FAR);
    } else if (delta > 40) {
      // this is a pain grunt, so don't play it if dead
      if (pm->ps->stats[STAT_HEALTH] > 0) {
        PM_AddEvent(EV_FALL_MEDIUM);
      }
    } else if (delta > 7) {
      PM_AddEvent(EV_FALL_SHORT);
    } else {
      PM_AddEvent(PM_FootstepForSurface());
    }
  }

  // start footstep cycle over
  pm->ps->bobCycle = 0;
}

/*
=============
PM_CheckStuck
=============
*/
/*
void PM_CheckStuck(void) {
        trace_t trace;

        pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin,
pm->ps->clientNum, pm->tracemask); if (trace.allsolid) {
                //int shit = qtrue;
        }
}
*/

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid(trace_t *trace) {
  int i, j, k;
  vec3_t point;

  if (pm->debugLevel) {
    Com_Printf("%i:allsolid\n", c_pmove);
  }

  // jitter around
  for (i = -1; i <= 1; i++) {
    for (j = -1; j <= 1; j++) {
      for (k = -1; k <= 1; k++) {
        VectorCopy(pm->ps->origin, point);
        point[0] += (float)i;
        point[1] += (float)j;
        point[2] += (float)k;
        pm->trace(trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum,
                  pm->tracemask);
        if (!trace->allsolid) {
          point[0] = pm->ps->origin[0];
          point[1] = pm->ps->origin[1];
          point[2] = pm->ps->origin[2] - 0.25;

          pm->trace(trace, pm->ps->origin, pm->mins, pm->maxs, point,
                    pm->ps->clientNum, pm->tracemask);
          pml.groundTrace = *trace;
          return qtrue;
        }
      }
    }
  }

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  pml.groundPlane = qfalse;
  pml.walking = qfalse;

  return qfalse;
}

/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed(void) {
  trace_t trace;
  vec3_t point;

  if (pm->ps->groundEntityNum != ENTITYNUM_NONE) {
    // we just transitioned into freefall
    if (pm->debugLevel) {
      Com_Printf("%i:lift\n", c_pmove);
    }

    // if they aren't in a jumping animation and the ground is a ways away,
    // force into it if we didn't do the trace, the player would be backflipping
    // down staircases
    VectorCopy(pm->ps->origin, point);
    point[2] -= 64;

    pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point,
              pm->ps->clientNum, pm->tracemask);
    if (trace.fraction == 1.0) {
      if (pm->cmd.forwardmove >= 0) {
        PM_ForceLegsAnim(LEGS_JUMP);
        pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
      } else {
        PM_ForceLegsAnim(LEGS_JUMPB);
        pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
      }
    }
  }

  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  pml.groundPlane = qfalse;
  pml.walking = qfalse;
}

/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace(void) {
  vec3_t point;
  trace_t trace;

  point[0] = pm->ps->origin[0];
  point[1] = pm->ps->origin[1];
  point[2] = pm->ps->origin[2] - 0.25;

  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point,
            pm->ps->clientNum, pm->tracemask);
  pml.groundTrace = trace;

  // do something corrective if the trace starts in a solid...
  if (trace.allsolid) {
    if (!PM_CorrectAllSolid(&trace))
      return;
  }

  // if the trace didn't hit anything, we are in free fall
  if (trace.fraction == 1.0) {
    PM_GroundTraceMissed();
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }

  // check if getting thrown off the ground
  if (pm->ps->velocity[2] > 0 &&
      DotProduct(pm->ps->velocity, trace.plane.normal) > 10) {
    if (pm->debugLevel) {
      Com_Printf("%i:kickoff\n", c_pmove);
    }
    // go into jump animation
    if (pm->cmd.forwardmove >= 0) {
      PM_ForceLegsAnim(LEGS_JUMP);
      pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
    } else {
      PM_ForceLegsAnim(LEGS_JUMPB);
      pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
    }

    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }

  // slopes that are too steep will not be considered onground
  if (trace.plane.normal[2] < MIN_WALK_NORMAL) {
    if (pm->debugLevel) {
      Com_Printf("%i:steep\n", c_pmove);
    }
    // FIXME: if they can't slide down the slope, let them
    // walk (sharp crevices)
    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qtrue;
    pml.walking = qfalse;
    return;
  }

  pml.groundPlane = qtrue;
  pml.walking = qtrue;

  // hitting solid ground will end a waterjump
  if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
    pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
    pm->ps->pm_time = 0;
  }

  if (pm->ps->groundEntityNum == ENTITYNUM_NONE) {
    // just hit the ground
    if (pm->debugLevel) {
      Com_Printf("%i:Land\n", c_pmove);
    }

    PM_CrashLand();

    // don't do landing time if we were just going down a slope
    if (pml.previous_velocity[2] < -200) {
      // don't allow another jump for a little while
      pm->ps->pm_flags |= PMF_TIME_LAND;
      pm->ps->pm_time = 250;
    }
  }

  pm->ps->groundEntityNum = trace.entityNum;

  // don't reset the z velocity for slopes
  //	pm->ps->velocity[2] = 0;

  PM_AddTouchEnt(trace.entityNum);
}

/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel(void) {
  vec3_t point;
  int cont;
  int sample1;
  int sample2;

  //
  // get waterlevel, accounting for ducking
  //
  pm->waterlevel = 0;
  pm->watertype = 0;

  point[0] = pm->ps->origin[0];
  point[1] = pm->ps->origin[1];
  point[2] = pm->ps->origin[2] + MINS_Z + 1;
  cont = pm->pointcontents(point, pm->ps->clientNum);

  if (cont & MASK_WATER) {
    sample2 = pm->ps->viewheight - MINS_Z;
    sample1 = sample2 / 2;

    pm->watertype = cont;
    pm->waterlevel = 1;
    point[2] = pm->ps->origin[2] + MINS_Z + sample1;
    cont = pm->pointcontents(point, pm->ps->clientNum);
    if (cont & MASK_WATER) {
      pm->waterlevel = 2;
      point[2] = pm->ps->origin[2] + MINS_Z + sample2;
      cont = pm->pointcontents(point, pm->ps->clientNum);
      if (cont & MASK_WATER) {
        pm->waterlevel = 3;
      }
    }
  }
}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_CheckDuck(void) {
  trace_t trace;

  if (pm->ps->powerups[PW_INVULNERABILITY]) {
    if (pm->ps->pm_flags & PMF_INVULEXPAND) {
      // invulnerability sphere has a 42 units radius
      VectorSet(pm->mins, -42, -42, -42);
      VectorSet(pm->maxs, 42, 42, 42);
    } else {
      VectorSet(pm->mins, -15, -15, MINS_Z);
      VectorSet(pm->maxs, 15, 15, 16);
    }
    pm->ps->pm_flags |= PMF_DUCKED;
    pm->ps->viewheight = CROUCH_VIEWHEIGHT;
    return;
  }
  pm->ps->pm_flags &= ~PMF_INVULEXPAND;

  pm->mins[0] = -15;
  pm->mins[1] = -15;

  pm->maxs[0] = 15;
  pm->maxs[1] = 15;

  pm->mins[2] = MINS_Z;

  if (pm->ps->pm_type == PM_DEAD) {
    pm->maxs[2] = -8;
    pm->ps->viewheight = DEAD_VIEWHEIGHT;
    return;
  }

  if (pm->cmd.upmove < 0) { // duck
    pm->ps->pm_flags |= PMF_DUCKED;
  } else { // stand up if possible
    if (pm->ps->pm_flags & PMF_DUCKED) {
      // try to stand up
      pm->maxs[2] = 32;
      pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin,
                pm->ps->clientNum, pm->tracemask);
      if (!trace.allsolid)
        pm->ps->pm_flags &= ~PMF_DUCKED;
    }
  }

  if (pm->ps->pm_flags & PMF_DUCKED) {
    pm->maxs[2] = 16;
    pm->ps->viewheight = CROUCH_VIEWHEIGHT;
  } else {
    pm->maxs[2] = 32;
    pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
  }
}

//===================================================================

/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps(void) {
  float bobmove;
  int old;
  qboolean footstep;

  //
  // calculate speed and cycle to be used for
  // all cyclic walking effects
  //
  pm->xyspeed = sqrt(pm->ps->velocity[0] * pm->ps->velocity[0] +
                     pm->ps->velocity[1] * pm->ps->velocity[1]);

  if (pm->ps->groundEntityNum == ENTITYNUM_NONE) {

    if (pm->ps->powerups[PW_INVULNERABILITY]) {
      PM_ContinueLegsAnim(LEGS_IDLECR);
    }
    // airborne leaves position in cycle intact, but doesn't advance
    if (pm->waterlevel > 1) {
      PM_ContinueLegsAnim(LEGS_SWIM);
    }
    return;
  }

  // if not trying to move
  if (!pm->cmd.forwardmove && !pm->cmd.rightmove) {
    if (pm->xyspeed < 5) {
      pm->ps->bobCycle = 0; // start at beginning of cycle again
      if (pm->ps->pm_flags & PMF_DUCKED) {
        PM_ContinueLegsAnim(LEGS_IDLECR);
      } else {
        PM_ContinueLegsAnim(LEGS_IDLE);
      }
    }
    return;
  }

  footstep = qfalse;

  if (pm->ps->pm_flags & PMF_DUCKED) {
    bobmove = 0.5; // ducked characters bob much faster
    if (pm->ps->pm_flags & PMF_BACKWARDS_RUN) {
      PM_ContinueLegsAnim(LEGS_BACKCR);
    } else {
      PM_ContinueLegsAnim(LEGS_WALKCR);
    }
    // ducked characters never play footsteps
    /*
    } else 	if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
            if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
                    bobmove = 0.4;	// faster speeds bob faster
                    footstep = qtrue;
            } else {
                    bobmove = 0.3;
            }
            PM_ContinueLegsAnim( LEGS_BACK );
    */
  } else {
    if (!(pm->cmd.buttons & BUTTON_WALKING)) {
      bobmove = 0.4f; // faster speeds bob faster
      if (pm->ps->pm_flags & PMF_BACKWARDS_RUN) {
        PM_ContinueLegsAnim(LEGS_BACK);
      } else {
        PM_ContinueLegsAnim(LEGS_RUN);
      }
      footstep = qtrue;
    } else {
      bobmove = 0.3f; // walking bobs slow
      if (pm->ps->pm_flags & PMF_BACKWARDS_RUN) {
        PM_ContinueLegsAnim(LEGS_BACKWALK);
      } else {
        PM_ContinueLegsAnim(LEGS_WALK);
      }
    }
  }

  // check for footstep / splash sounds
  old = pm->ps->bobCycle;
  pm->ps->bobCycle = (int)(old + bobmove * pml.msec) & 255;

  // if we just crossed a cycle boundary, play an appropriate footstep event
  if (((old + 64) ^ (pm->ps->bobCycle + 64)) & 128) {
    if (pm->waterlevel == 0) {
      // on ground will only play sounds if running
      if (footstep && !pm->noFootsteps) {
        PM_AddEvent(PM_FootstepForSurface());
      }
    } else if (pm->waterlevel == 1) {
      // splashing
      PM_AddEvent(EV_FOOTSPLASH);
    } else if (pm->waterlevel == 2) {
      // wading / swimming at surface
      PM_AddEvent(EV_SWIM);
    } else if (pm->waterlevel == 3) {
      // no sound when completely underwater
    }
  }
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents(void) { // FIXME?
  //
  // if just entered a water volume, play a sound
  //
  if (!pml.previous_waterlevel && pm->waterlevel) {
    PM_AddEvent(EV_WATER_TOUCH);
  }

  //
  // if just completely exited a water volume, play a sound
  //
  if (pml.previous_waterlevel && !pm->waterlevel) {
    PM_AddEvent(EV_WATER_LEAVE);
  }

  //
  // check for head just going under water
  //
  if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
    PM_AddEvent(EV_WATER_UNDER);
  }

  //
  // check for head just coming out of water
  //
  if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
    PM_AddEvent(EV_WATER_CLEAR);
  }
}

/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange(int weapon) {
  if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS) {
    return;
  }

  if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon))) {
    return;
  }

  if (pm->ps->weaponstate == WEAPON_DROPPING) {
    return;
  }

  PM_AddEvent(EV_CHANGE_WEAPON);
  pm->ps->weaponstate = WEAPON_DROPPING;
  pm->ps->weaponTime += 200;
  PM_StartTorsoAnim(TORSO_DROP);
}

/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange(void) {
  int weapon;

  weapon = pm->cmd.weapon;
  if (weapon < WP_NONE || weapon >= WP_NUM_WEAPONS) {
    weapon = WP_NONE;
  }

  if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon))) {
    weapon = WP_NONE;
  }

  pm->ps->weapon = weapon;
  pm->ps->weaponstate = WEAPON_RAISING;
  pm->ps->weaponTime += 250;
  PM_StartTorsoAnim(TORSO_RAISE);
}

/*
==============
PM_TorsoAnimation

==============
*/
static void PM_TorsoAnimation(void) {
  if (pm->ps->weaponstate == WEAPON_READY) {
    if (pm->ps->weapon == WP_GAUNTLET) {
      PM_ContinueTorsoAnim(TORSO_STAND2);
    } else {
      PM_ContinueTorsoAnim(TORSO_STAND);
    }
    return;
  }
}

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon(void) {
  int addTime;

  // don't allow attack until all buttons are up
  if (pm->ps->pm_flags & PMF_RESPAWNED) {
    return;
  }

  // ignore if spectator
  if (pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
    return;
  }

  // check for dead player
  if (pm->ps->stats[STAT_HEALTH] <= 0) {
    pm->ps->weapon = WP_NONE;
    return;
  }

  // check for item using
  if (pm->cmd.buttons & BUTTON_USE_HOLDABLE) {
    if (!(pm->ps->pm_flags & PMF_USE_ITEM_HELD)) {
      if (bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT &&
          pm->ps->stats[STAT_HEALTH] >= (pm->ps->stats[STAT_MAX_HEALTH] + 25)) {
        // don't use medkit if at max health
      } else {
        pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
        PM_AddEvent(EV_USE_ITEM0 +
                    bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag);
        pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
      }
      return;
    }
  } else {
    pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
  }

  // make weapon function
  if (pm->ps->weaponTime > 0) {
    pm->ps->weaponTime -= pml.msec;
  }

  // check for weapon change
  // can't change if weapon is firing, but can change
  // again if lowering or raising
  if (pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING) {
    if (pm->ps->weapon != pm->cmd.weapon) {
      PM_BeginWeaponChange(pm->cmd.weapon);
    }
  }

  if (pm->ps->weaponTime > 0) {
    return;
  }

  // change weapon if time
  if (pm->ps->weaponstate == WEAPON_DROPPING) {
    PM_FinishWeaponChange();
    return;
  }

  if (pm->ps->weaponstate == WEAPON_RAISING) {
    pm->ps->weaponstate = WEAPON_READY;
    if (pm->ps->weapon == WP_GAUNTLET) {
      PM_StartTorsoAnim(TORSO_STAND2);
    } else {
      PM_StartTorsoAnim(TORSO_STAND);
    }
    return;
  }

  // check for fire
  if (!(pm->cmd.buttons & BUTTON_ATTACK)) {
    pm->ps->weaponTime = 0;
    pm->ps->weaponstate = WEAPON_READY;
    return;
  }

  // start the animation even if out of ammo
  if (pm->ps->weapon == WP_GAUNTLET) {
    // the guantlet only "fires" when it actually hits something
    if (!pm->gauntletHit) {
      pm->ps->weaponTime = 0;
      pm->ps->weaponstate = WEAPON_READY;
      return;
    }
    PM_StartTorsoAnim(TORSO_ATTACK2);
  } else {
    PM_StartTorsoAnim(TORSO_ATTACK);
  }

  pm->ps->weaponstate = WEAPON_FIRING;

  // check for out of ammo
  if (!pm->ps->ammo[pm->ps->weapon]) {
    PM_AddEvent(EV_NOAMMO);
    pm->ps->weaponTime += 500;
    return;
  }

  // take an ammo away if not infinite
  if (pm->ps->ammo[pm->ps->weapon] != -1) {
    pm->ps->ammo[pm->ps->weapon]--;
  }

  // fire weapon
  PM_AddEvent(EV_FIRE_WEAPON);

  switch (pm->ps->weapon) {
  default:
  case WP_GAUNTLET:
    addTime = 400;
    break;
  case WP_LIGHTNING:
    addTime = 50;
    break;
  case WP_SHOTGUN:
    addTime = 1000;
    break;
  case WP_MACHINEGUN:
    addTime = 100;
    break;
  case WP_GRENADE_LAUNCHER:
    addTime = 800;
    break;
  case WP_ROCKET_LAUNCHER:
    addTime = 800;
    break;
  case WP_PLASMAGUN:
    addTime = 100;
    break;
  case WP_RAILGUN:
    addTime = 1500;
    break;
  case WP_BFG:
    addTime = 200;
    break;
  case WP_GRAPPLING_HOOK:
    addTime = 400;
    break;
#ifdef TEAMARENA
  case WP_NAILGUN:
    addTime = 1000;
    break;
  case WP_PROX_LAUNCHER:
    addTime = 800;
    break;
  case WP_CHAINGUN:
    addTime = 30;
    break;
#endif
  }

#ifdef TEAMARENA
  if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT) {
    addTime /= 1.5;
  } else if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag ==
             PW_AMMOREGEN) {
    addTime /= 1.3;
  } else
#endif
      if (pm->ps->powerups[PW_HASTE]) {
    addTime /= 1.3;
  }

  pm->ps->weaponTime += addTime;
}

/*
================
PM_Animate
================
*/

static void PM_Animate(void) {
  if (pm->cmd.buttons & BUTTON_GESTURE) {
    if (pm->ps->torsoTimer == 0) {
      PM_StartTorsoAnim(TORSO_GESTURE);
      pm->ps->torsoTimer = TIMER_GESTURE;
      PM_AddEvent(EV_TAUNT);
    }
#ifdef TEAMARENA
  } else if (pm->cmd.buttons & BUTTON_GETFLAG) {
    if (pm->ps->torsoTimer == 0) {
      PM_StartTorsoAnim(TORSO_GETFLAG);
      pm->ps->torsoTimer = 600; // TIMER_GESTURE;
    }
  } else if (pm->cmd.buttons & BUTTON_GUARDBASE) {
    if (pm->ps->torsoTimer == 0) {
      PM_StartTorsoAnim(TORSO_GUARDBASE);
      pm->ps->torsoTimer = 600; // TIMER_GESTURE;
    }
  } else if (pm->cmd.buttons & BUTTON_PATROL) {
    if (pm->ps->torsoTimer == 0) {
      PM_StartTorsoAnim(TORSO_PATROL);
      pm->ps->torsoTimer = 600; // TIMER_GESTURE;
    }
  } else if (pm->cmd.buttons & BUTTON_FOLLOWME) {
    if (pm->ps->torsoTimer == 0) {
      PM_StartTorsoAnim(TORSO_FOLLOWME);
      pm->ps->torsoTimer = 600; // TIMER_GESTURE;
    }
  } else if (pm->cmd.buttons & BUTTON_AFFIRMATIVE) {
    if (pm->ps->torsoTimer == 0) {
      PM_StartTorsoAnim(TORSO_AFFIRMATIVE);
      pm->ps->torsoTimer = 600; // TIMER_GESTURE;
    }
  } else if (pm->cmd.buttons & BUTTON_NEGATIVE) {
    if (pm->ps->torsoTimer == 0) {
      PM_StartTorsoAnim(TORSO_NEGATIVE);
      pm->ps->torsoTimer = 600; // TIMER_GESTURE;
    }
#endif
  }
}

/*
================
PM_DropTimers
================
*/
static void PM_DropTimers(void) {
  // drop misc timing counter
  if (pm->ps->pm_time) {
    if (pml.msec >= pm->ps->pm_time) {
      pm->ps->pm_flags &= ~PMF_ALL_TIMES;
      pm->ps->pm_time = 0;
    } else {
      pm->ps->pm_time -= pml.msec;
    }
  }

  // drop animation counter
  if (pm->ps->legsTimer > 0) {
    pm->ps->legsTimer -= pml.msec;
    if (pm->ps->legsTimer < 0) {
      pm->ps->legsTimer = 0;
    }
  }

  if (pm->ps->torsoTimer > 0) {
    pm->ps->torsoTimer -= pml.msec;
    if (pm->ps->torsoTimer < 0) {
      pm->ps->torsoTimer = 0;
    }
  }
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated instead of a full move
================
*/
void PM_UpdateViewAngles(playerState_t *ps, const usercmd_t *cmd) {
  short temp;
  int i;

  if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
    return; // no view changes at all
  }

  if (ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0) {
    return; // no view changes at all
  }

  // circularly clamp the angles with deltas
  for (i = 0; i < 3; i++) {
    temp = cmd->angles[i] + ps->delta_angles[i];
    if (i == PITCH) {
      // don't let the player look up or down more than 90 degrees
      if (temp > 16000) {
        ps->delta_angles[i] = 16000 - cmd->angles[i];
        temp = 16000;
      } else if (temp < -16000) {
        ps->delta_angles[i] = -16000 - cmd->angles[i];
        temp = -16000;
      }
    }
    ps->viewangles[i] = SHORT2ANGLE(temp);
  }
}

/*
================
PmoveSingle

================
*/
void trap_SnapVector(float *v);

void PmoveSingle(pmove_t *pmove) {
  pm = pmove;

  // this counter lets us debug movement problems with a journal
  // by setting a conditional breakpoint fot the previous frame
  c_pmove++;

  // clear results
  pm->numtouch = 0;
  pm->watertype = 0;
  pm->waterlevel = 0;

  if (pm->ps->stats[STAT_HEALTH] <= 0) {
    pm->tracemask &= ~CONTENTS_BODY; // corpses can fly through bodies
  }

  // make sure walking button is clear if they are running, to avoid
  // proxy no-footsteps cheats
  if (abs(pm->cmd.forwardmove) > 64 || abs(pm->cmd.rightmove) > 64) {
    pm->cmd.buttons &= ~BUTTON_WALKING;
  }

  // set the talk balloon flag
  if (pm->cmd.buttons & BUTTON_TALK) {
    pm->ps->eFlags |= EF_TALK;
  } else {
    pm->ps->eFlags &= ~EF_TALK;
  }

  // set the firing flag for continuous beam weapons
  if (!(pm->ps->pm_flags & PMF_RESPAWNED) &&
      pm->ps->pm_type != PM_INTERMISSION && pm->ps->pm_type != PM_NOCLIP &&
      (pm->cmd.buttons & BUTTON_ATTACK) && pm->ps->ammo[pm->ps->weapon]) {
    pm->ps->eFlags |= EF_FIRING;
  } else {
    pm->ps->eFlags &= ~EF_FIRING;
  }

  // clear the respawned flag if attack and use are cleared
  if (pm->ps->stats[STAT_HEALTH] > 0 &&
      !(pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE))) {
    pm->ps->pm_flags &= ~PMF_RESPAWNED;
  }

  // if talk button is down, dissallow all other input
  // this is to prevent any possible intercept proxy from
  // adding fake talk balloons
  if (pmove->cmd.buttons & BUTTON_TALK) {
    // keep the talk button set tho for when the cmd.serverTime > 66 msec
    // and the same cmd is used multiple times in Pmove
    pmove->cmd.buttons = BUTTON_TALK;
    pmove->cmd.forwardmove = 0;
    pmove->cmd.rightmove = 0;
    pmove->cmd.upmove = 0;
  }

  // clear all pmove local vars
  memset(&pml, 0, sizeof(pml));

  // determine the time
  pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
  if (pml.msec < 1) {
    pml.msec = 1;
  } else if (pml.msec > 200) {
    pml.msec = 200;
  }
  pm->ps->commandTime = pmove->cmd.serverTime;

  // save old org in case we get stuck
  VectorCopy(pm->ps->origin, pml.previous_origin);

  // save old velocity for crashlanding
  VectorCopy(pm->ps->velocity, pml.previous_velocity);

  pml.frametime = pml.msec * 0.001;

  // update the viewangles
  PM_UpdateViewAngles(pm->ps, &pm->cmd);

  AngleVectors(pm->ps->viewangles, pml.forward, pml.right, pml.up);

  if (pm->cmd.upmove < 10) {
    // not holding jump
    pm->ps->pm_flags &= ~PMF_JUMP_HELD;
  }

  // decide if backpedaling animations should be used
  if (pm->cmd.forwardmove < 0) {
    pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
  } else if (pm->cmd.forwardmove > 0 ||
             (pm->cmd.forwardmove == 0 && pm->cmd.rightmove)) {
    pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
  }

  if (pm->ps->pm_type >= PM_DEAD) {
    pm->cmd.forwardmove = 0;
    pm->cmd.rightmove = 0;
    pm->cmd.upmove = 0;
  }

  if (pm->ps->pm_type == PM_SPECTATOR) {
    PM_CheckDuck();
    PM_FlyMove();
    PM_DropTimers();
    return;
  }

  if (pm->ps->pm_type == PM_NOCLIP) {
    PM_NoclipMove();
    PM_DropTimers();
    return;
  }

  if (pm->ps->pm_type == PM_FREEZE) {
    return; // no movement at all
  }

  if (pm->ps->pm_type == PM_INTERMISSION ||
      pm->ps->pm_type == PM_SPINTERMISSION) {
    return; // no movement at all
  }

  // set watertype, and waterlevel
  PM_SetWaterLevel();
  pml.previous_waterlevel = pmove->waterlevel;

  // set mins, maxs, and viewheight
  if (pm->movetype == VQ1) { q1_CheckDuck (); } //::OSDF added. Allows crouch while jumping
  else { PM_CheckDuck(); }

  // set groundentity
  PM_GroundTrace();

  if (pm->ps->pm_type == PM_DEAD) {
    PM_DeadMove();
  }

  PM_DropTimers();

  //::OSDF modded: Flow control
#if 1
  osdf_move(pm);
  return;
#endif
  // OSDF end

#ifdef TEAMARENA
  if (pm->ps->powerups[PW_INVULNERABILITY]) {
    PM_InvulnerabilityMove();
  } else
#endif
      if (pm->ps->powerups[PW_FLIGHT]) {
    // flight powerup doesn't allow jump and has different friction
    PM_FlyMove();
  } else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
    PM_GrappleMove();
    // We can wiggle a bit
    PM_AirMove();
  } else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
    PM_WaterJumpMove();
  } else if (pm->waterlevel > 1) {
    // swimming
    PM_WaterMove();
  } else if (pml.walking) {
    // walking on ground
    PM_WalkMove();
  } else {
    // airborne
    PM_AirMove();
  }

  PM_Animate();

  // set groundentity, watertype, and waterlevel
  PM_GroundTrace();
  PM_SetWaterLevel();

  // weapons
  PM_Weapon();

  // torso animation
  PM_TorsoAnimation();

  // footstep events / legs animations
  PM_Footsteps();

  // entering / leaving water splashes
  PM_WaterEvents();

  // snap some parts of playerstate to save network bandwidth
  trap_SnapVector(pm->ps->velocity);
}

/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove(pmove_t *pmove) {
  int finalTime;

  finalTime = pmove->cmd.serverTime;

  if (finalTime < pmove->ps->commandTime) {
    return; // should not happen
  }

  if (finalTime > pmove->ps->commandTime + 1000) {
    pmove->ps->commandTime = finalTime - 1000;
  }

  pmove->ps->pmove_framecount =
      (pmove->ps->pmove_framecount + 1) & ((1 << PS_PMOVEFRAMECOUNTBITS) - 1);

  // chop the move up if it is too long, to prevent framerate
  // dependent behavior
  while (pmove->ps->commandTime != finalTime) {
    int msec;

    msec = finalTime - pmove->ps->commandTime;

    if (pmove->pmove_fixed) {
      if (msec > pmove->pmove_msec) {
        msec = pmove->pmove_msec;
      }
    } else {
      if (msec > 66) {
        msec = 66;
      }
    }
    pmove->cmd.serverTime = pmove->ps->commandTime + msec;
    if (1){ PmoveSingle(pmove); }
    else  { osdf_PmoveSingle(pmove); }

    if (pmove->ps->pm_flags & PMF_JUMP_HELD) {
      pmove->cmd.upmove = 20;
    }
  }

  // PM_CheckStuck();
}

//::OSDF modded
//::::::::::::::::::::::
qboolean osdf_initialized = qfalse;
// TODO: Change to cvars
//  General
float phy_stopspeed;
float phy_crouch_scale;
// Acceleration
// float    phy_ground_accel;
// float    phy_air_accel;
float phy_fly_accel;
// Friction
float phy_friction;
float phy_fly_friction;
float phy_spectator_friction;

// New variables
//int   phy_movetype;           // pm->movetype; // Physics type selection. Available: VQ3, CPM    TODO: DEV
// Ground
float phy_ground_basespeed;     // Movement speed on the ground (aka maxspeed). Equivalent to the default g_speed
float phy_ground_accel;         // Acceleration when on the ground. sv_accelerate
// Air
float phy_air_basespeed;        // Maxspeed on air when in strafe mode VQ3, or when strafing diagonally in modes CPM and DIFF
float phy_air_accel;            // Acceleration when in strafe mode VQ3, or when strafing diagonally in modes CPM and DIFF
// Air deceleration. To have different accel values for stopping down than for accelerating normally.
float phy_air_decel;            // Factor to scale down air acceleration, when the current angle is over decelAngle
float phy_air_decelAngle;       // Angle at which air deceleration will change
float phy_air_speedscalar;      // Q1-ag movement. Scales incoming ps->speed from ClientThink_real by this number, to calculate real air speed.
// AirStrafe (aka AD turning)
float phy_airstrafe_accel;      // Acceleration in strafe mode VQ1, or when strafing "quakeworld style" in modes CPM and DIFF
float phy_airstrafe_basespeed;  // Maxspeed on air when in strafe mode VQ3, or when strafing diagonally in modes CPM and DIFF
// AirControl (aka W turning)
qboolean phy_aircontrol;        // Turns aircontrol on or off 
float phy_aircontrol_amount;    // Amount you can control yourself with W/S
float phy_aircontrol_power;     // Aircontrol formula exponent
float phy_fw_accelerate;        // How much (ups) to accelerate when holding forwards(W) only
// Jump
qboolean phy_jump_auto;         // Enable/disable autojump
int   phy_jump_type;            // Jump type selection. Available VQ3, CPM    TODO: DEV
int   phy_jump_velocity;        // Vertical velocity that will be set/added when jumping (default = JUMP_VELOCITY = 270)
int   phy_jump_dj_time;         // Amount of time(ms) since last jump, where CPM dj behavior can happen. (default CPM = 400)
int   phy_jump_dj_velocity;     // Amount of velocity to add to CPM dj behavior. (default CPM = 100)
// Powerups
//float phy_haste_factor;         // Multiplier to apply during haste powerup (q3 default = 1.3)
//float phy_quad_factor;          // Multiplier to apply during quad powerup  (q3 default = 3)
// Water
float phy_water_accel;
float phy_water_scale;          // phy_swimScale;
float phy_water_friction;
// q3 math quirks
qboolean phy_snapvelocity;      // Enables rounding velocity to ints every frame (default q3a = qtrue)
qboolean phy_input_scalefix;    // Fixes slowdown on jump input if enabled (default q3a = qfalse)
float q1_overbounce_scale;     // OVERCLIP value. Removed to 1.000f for QW
// Slidemove 
int   phy_slidemove_type;       // Slidemove function type. For controlling general movement behavior (not slick_type)
// Sound
int   s_jump_interval;          // Dictates how often (ms) the jump sound can be played. Used to stop it from playing every tick/frame

void osdf_init(int movetype) {
  // Initialize physics variables.
  // TODO: Change to Cvars. Current is a TEMP solution until proper physics cvars support.
  if (movetype == VQ3) {
    phy_stopspeed = pm_stopspeed;
    phy_crouch_scale = pm_duckScale;
    // Acceleration
    phy_ground_accel = pm_accelerate;
    phy_air_accel = pm_airaccelerate;
    phy_fly_accel = pm_flyaccelerate;
    // Friction
    phy_friction = pm_friction;
    phy_fly_friction = pm_flightfriction;
    phy_spectator_friction = pm_spectatorfriction;
    // Water
    phy_water_accel = pm_wateraccelerate;
    phy_water_scale = pm_swimScale;
    phy_water_friction = pm_waterfriction;
    // New
    phy_slidemove_type = Q3A;
    phy_snapvelocity = qtrue;
    phy_input_scalefix = qfalse;
    phy_aircontrol = qfalse;
    phy_jump_type = VQ3;
    phy_jump_auto = qfalse;
    phy_jump_velocity = JUMP_VELOCITY;

  } else if (movetype == CPM) {
    phy_stopspeed = pm_stopspeed;
    phy_crouch_scale = pm_duckScale;
    // Acceleration
    phy_fly_accel = pm_flyaccelerate;
    // Friction
    phy_friction = pm_friction;
    phy_fly_friction = pm_flightfriction;
    phy_spectator_friction = pm_spectatorfriction;
    // Water
    phy_water_accel = pm_wateraccelerate;
    phy_water_scale = pm_swimScale;
    phy_water_friction = 0.5;
    // New
    phy_slidemove_type = Q3A;
    phy_snapvelocity = qtrue;
    phy_input_scalefix = qfalse;
    phy_ground_basespeed = 320;
    phy_ground_accel = 15;
    // Air movement
    phy_air_basespeed = 320;
    phy_air_accel = 1;
    phy_air_decel = 2.5;
    phy_air_decelAngle = 100;
    // W turning
    phy_aircontrol = qtrue;
    phy_aircontrol_amount = 150;
    phy_aircontrol_power = 2;
    phy_fw_accelerate = 0;
    // AD turning
    phy_airstrafe_basespeed = 30;
    phy_airstrafe_accel = 70;
    // Jump
    phy_jump_auto = qfalse;
    phy_jump_type = CPM;
    phy_jump_velocity = JUMP_VELOCITY;
    phy_jump_dj_time = 400;
    phy_jump_dj_velocity = 100;

  } else if (movetype == VQ1) {
    phy_stopspeed = 100; // QW value
    phy_crouch_scale = pm_duckScale;
    // Acceleration
    phy_fly_accel = pm_flyaccelerate;
    // Friction
    phy_friction = 4; // QW value. from 6 (q3a/pm_friction)
    phy_fly_friction = pm_flightfriction;
    phy_spectator_friction = pm_spectatorfriction;
    // Water
    phy_water_accel = 10; // QW value. from 4 (q3a/pm_wateraccelerate)
    phy_water_scale = pm_swimScale;
    phy_water_friction = 4; // QW value
    // New
    phy_slidemove_type = Q3A;
    phy_snapvelocity = qfalse;
    phy_input_scalefix = qtrue;
    q1_overbounce_scale = 1.000f;
    // Ground
    phy_ground_basespeed = 320;
    phy_ground_accel = 10;
    // Air movement
    phy_air_accel = 90; // 70 = QW value 0.7 : q3a = 1 :: 100 was too much. 70 not enough :: 0.1/32as/90aa/125fps felt really good
    phy_air_speedscalar = 0.11; //0.1125;=36 slightly too much //0.125=40 too much; // Value: 0.0938 = ~30, but its too slow. 0.1/32as/70aa/77fps super close to QW/vint
    // Jump
    phy_jump_velocity = JUMP_VELOCITY;
    phy_jump_auto = qtrue;
    phy_jump_type = VQ3;
    s_jump_interval = 250;

  } else if (movetype == VJK) {
    phy_stopspeed = pm_stopspeed;
    phy_crouch_scale = pm_duckScale;
    // Acceleration
    phy_ground_accel = 12;
    phy_air_accel = 4;  // and   phy_speed = 250 
    phy_fly_accel = pm_flyaccelerate;
    // Friction
    phy_friction = pm_friction;
    phy_fly_friction = pm_flightfriction;
    phy_spectator_friction = pm_spectatorfriction;
    // Water
    phy_water_accel = pm_wateraccelerate;
    phy_water_scale = pm_swimScale;
    phy_water_friction = pm_waterfriction;
    // New
    phy_slidemove_type = Q3A;
    phy_snapvelocity = qtrue;
    phy_input_scalefix = qfalse;
    phy_aircontrol = qfalse;
    phy_jump_type = VQ3;
    phy_jump_auto = qtrue;
    phy_jump_velocity = JUMP_VELOCITY; // vjk = 225. vq3 default = JUMP_VELOCITY = 270
    }

  osdf_initialized = qtrue;
}

// VectorReflect
// =============
// PM_ClipVelocity = VelocityProject = Vector_RotateAndScale = Vector_ReflectAndScale
//     Wrongly named "clip", due to doom naming inheritance. 
//     It no longer "clips" or "slides" a vector, but instead reflects / projects it
//     This is used when we want the player to not simply just go through a surface.
//
// - Takes an incoming vector (in), and reflects it on a surface (normal), scaled by the desired amount (overbounce)
//   This is also used to reflect vectors based on any other arbitrary vector
//
void q3a_VectorReflect(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
  float backoff, change;
  int i;
  // Calculate direction of rotation / reflection
  backoff = DotProduct(in, normal);
  // Scale the direction
  if (backoff < 0)  { backoff *= overbounce; }
  else              { backoff /= overbounce; }
  // Apply scale to the vector
  for (i = 0; i < 3; i++) {
    change = normal[i] * backoff;
    out[i] = in[i] - change; // An overbounce value of 1.000 completely negates the incoming velocity, due to this line
  }
}
// VectorReflect One Sided
void VectorReflectOS(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
  float backoff, change;
  int i;
  // Calculate direction of rotation / reflection
  backoff = DotProduct(in, normal);
  // Scale the direction
  if (backoff < 0)  { backoff *= overbounce; }
  else              { backoff  = 0; }
  // Apply scale to the vector
  for (i = 0; i < 3; i++) {
    change = normal[i] * backoff;
    out[i] = in[i] - change; // An overbounce value of 1.000 completely negates the incoming velocity, due to this line
  }
}


// Scale factor to apply to inputs (cmd).
// Modified to (optionally) allow fixing slowdown when holding jump.
//  .. fix is ignored for VQ3/CPM movetypes.
static float q3a_CmdScale(usercmd_t *cmd) {
  qboolean fix;
  int max;
  float total, scale;
  float fmove, smove, umove;

  // Aliases, for readability
  fix = phy_input_scalefix; // Input scaling fix (active/inactive). For fixing slowdown on jump-hold
  fmove = (float)abs(cmd->forwardmove);
  smove = (float)abs(cmd->rightmove);
  umove = (float)abs(cmd->upmove);

  // Select maximum input value (absolute)
  max = fmove;
  if (smove > max)         { max = smove; }
  if (umove > max && !fix) { max = umove; } // Ignore umove for scalefix
  if (!max)                { return 0; }

  // Calculate total input value
  if (fix) { total = sqrt(fmove * fmove + smove * smove); } // Ignore umove for scalefix
  else     { total = sqrt(fmove * fmove + smove * smove + umove * umove); }

  // Calculate scale value and return it
  //scale = basespeed * max / (127.0 * total);
  scale = (float)pm->ps->speed * max / (127.0 * total);  // This is default behavior, without a   basespeed   input
  return scale;
}

// Changed from q3a-gpl behavior to include basespeed.
void q3a_Accelerate(vec3_t wishdir, float wishspeed, float accel, float basespeed) {
  int i;
  float addspeed, accelspeed, currentspeed;
  float wishspeed_c;
  vec3_t accelVelocity;

  // Clamp wishpeed to a maximum of basespeed
  wishspeed_c = wishspeed; // Initialize clamped wishspeed
  if (wishspeed_c > basespeed) { wishspeed_c = basespeed; }

  // Determine veer amount
  currentspeed = DotProduct(pm->ps->velocity, wishdir);
  // See how much to add
  addspeed = wishspeed_c - currentspeed;
  // If not adding any, done.
  if (addspeed <= 0) {
    return;
  }
  // Acceleration speed to add after accel
  accelspeed = accel * wishspeed_c * pml.frametime;
  // Cap it
  if (accelspeed > addspeed) {
    accelspeed = addspeed;
  }
  // Adjust player velocity
  for (i = 0; i < 3; i++) {
    accelVelocity[i] = accelspeed * wishdir[i]; // dir*speed = velocity
    pm->ps->velocity[i] += accelVelocity[i];    // Vector addition, the typical visualization explained in videos of strafing math theory
  }
}

static void q3a_Friction( void ) {
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;
	
	vel = pm->ps->velocity;
	
	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0;		// allow sinking underwater
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
			// if getting knocked back, no friction
			if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
				control = speed < phy_stopspeed ? phy_stopspeed : speed;
				drop += control*phy_friction*pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		drop += speed*phy_water_friction*pm->waterlevel*pml.frametime;
	}

	// apply flying friction
	if ( pm->ps->powerups[PW_FLIGHT]) {
		drop += speed*phy_fly_friction*pml.frametime;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR) {
		drop += speed*phy_spectator_friction*pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

void q1_CheckDuck(void) {
  trace_t trace;

  if (pm->ps->powerups[PW_INVULNERABILITY]) {
    if (pm->ps->pm_flags & PMF_INVULEXPAND) {
      // invulnerability sphere has a 42 units radius
      VectorSet(pm->mins, -42, -42, -42);
      VectorSet(pm->maxs, 42, 42, 42);
    } else {
      VectorSet(pm->mins, -15, -15, MINS_Z);
      VectorSet(pm->maxs, 15, 15, 16);
    }
    pm->ps->pm_flags |= PMF_DUCKED;
    pm->ps->viewheight = CROUCH_VIEWHEIGHT;
    return;
  }
  pm->ps->pm_flags &= ~PMF_INVULEXPAND;

  pm->mins[0] = -15;
  pm->mins[1] = -15;

  pm->maxs[0] = 15;
  pm->maxs[1] = 15;

  pm->mins[2] = MINS_Z;

  if (pm->ps->pm_type == PM_DEAD) {
    pm->maxs[2] = -8;
    pm->ps->viewheight = DEAD_VIEWHEIGHT;
    return;
  }

  if (pm->cmd.upmove < 0) { // duck
     pm->ps->pm_flags |= PMF_DUCKED;
  } else { // stand up if possible
    if (pm->ps->pm_flags & PMF_DUCKED) {
      // try to stand up
      pm->maxs[2] = 32;
      pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
      if ( !trace.allsolid)// && !(pm->cmd.upmove < 0) )
        {pm->ps->pm_flags &= ~PMF_DUCKED;}
    }
  }

  if (pm->ps->pm_flags & PMF_DUCKED) {
    pm->maxs[2] = 16;
    pm->ps->viewheight = CROUCH_VIEWHEIGHT;
  } else {
    pm->maxs[2] = 32;
    pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
  }
}

static qboolean q1_CheckJump(void) {
  // Can't jump cases. Cannot jump again under these conditions
  // don't allow jump until all buttons are up
  if (pm->ps->pm_flags & PMF_RESPAWNED) {return qfalse;}
  // not holding jump
  if (pm->cmd.upmove < 10) {return qfalse;}  // Default Q3 behavior. Don't allow jump on crouch
  // if (pm->cmd.upmove < 10) {   //TODO: Jumpcrouch
    // We are either pressing jumpcrouch, crouch or noinput
    // qboolean noinput = (pm->cmd.upmove < 10) && !(pm->ps->pm_flags & PMF_DUCKED) ? qtrue:qfalse;
    // qboolean crouch  = (pm->cmd.upmove <  0)                                     ? qtrue:qfalse;
    // if (noinput || crouch) {return qfalse;}
    // qboolean crouchj = (pm->cmd.upmove < 10) &&  (pm->ps->pm_flags & PMF_DUCKED) ? qtrue:qfalse;
  // }

 // must wait for jump to be released, when autojump is disabled
  if ((pm->ps->pm_flags & PMF_JUMP_HELD) && !phy_jump_auto) {
    pm->cmd.upmove = 0; // clear upmove so cmdscale doesn't lower running speed
    return qfalse;
  }
  // Else: Can jump. Do jump behavior
  //
  pml.groundPlane = qfalse; // jumping away
  pml.walking = qfalse;
  pm->ps->pm_flags |= PMF_JUMP_HELD;
  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  
  if (pm->ps->pm_flags & PMF_DUCKED) { pm->ps->velocity[2] *= 0.5; }

  // QW downramps
  // Downramps act like vq3 (set). Upramps act like QW (add)
  if (pm->ps->velocity[2] < 0){
    if (pm->ps->pm_flags & PMF_DUCKED) {
      if (pm->debugLevel) { Com_Printf("%i:jump add\n", c_pmove); }
      pm->ps->velocity[2] += phy_jump_velocity;         // Downramp add
    } else { pm->ps->velocity[2] = phy_jump_velocity; } // Downramp set
  } else { pm->ps->velocity[2] += phy_jump_velocity; }  // Flat or Upramp
  //
  // Jump end

  if ((pm->cmd.serverTime - pm->ps->stats[STAT_TIME_LASTJUMP]) > s_jump_interval){
    PM_AddEvent(EV_JUMP); // Ask the client to play the jump sound
    if (pm->debugLevel) { Com_Printf("%i:Jump+Sound :: Lastjump=%i\n", c_pmove, pm->ps->stats[STAT_TIME_LASTJUMP]); }
    pm->ps->pm_flags &= ~PMF_JUMP_HELD;
  }
  if (pm->cmd.forwardmove >= 0) {
    PM_ForceLegsAnim(LEGS_JUMP);
    pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
  } else {
    PM_ForceLegsAnim(LEGS_JUMPB);
    pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
  }
  return qtrue;
}


static qboolean q3a_CheckJump(void) {
  qboolean canDoubleJump;

  // Can't jump cases. Cannot jump again under these conditions
  if (pm->ps->pm_flags & PMF_RESPAWNED) {
    return qfalse;  // don't allow jump until all buttons are up
  }
  if (pm->cmd.upmove < 10) { 
    return qfalse;  // not holding jump
  }
  if ((pm->ps->pm_flags & PMF_JUMP_HELD && !phy_jump_auto)) { // must wait for jump to be released
    pm->cmd.upmove = 0; // clear upmove so cmdscale doesn't lower running speed
    return qfalse;
  }
  // Else: Can jump. Do jump behavior
  //
  pml.groundPlane = qfalse; // jumping away
  pml.walking = qfalse;
  pm->ps->pm_flags |= PMF_JUMP_HELD;
  pm->ps->groundEntityNum = ENTITYNUM_NONE;

  //:: vq3 or CPM jump selection
  //
  // Select ADD or SET vertical velocity.
  canDoubleJump      = (pm->ps->velocity[2] > 0) ? qtrue: qfalse;
  if (canDoubleJump) { pm->ps->velocity[2] += phy_jump_velocity; } // ADD velocity, without resetting current
  else               { pm->ps->velocity[2]  = phy_jump_velocity; } // SET velocity, resets current
  //
  // Timer check.   ::This is only a check. The values are set in osdf_cpm (aka PmoveSingle)
  if (pm->movetype == CPM) {
    int      djtimer   = pm->cmd.serverTime - pm->ps->stats[STAT_TIME_LASTJUMP];
    qboolean djtimerOn = (( djtimer <= phy_jump_dj_time ) && (djtimer > 0)) ? qtrue : qfalse; // We can dj when this is true.
    if (djtimerOn) { 
      pm->ps->velocity[2] += phy_jump_dj_velocity; 
      if (pm->debugLevel) { Com_Printf(":: DoubleJump -> Timer= %i, Lastjump= %i, servertime= %i\n", djtimer, pm->ps->stats[STAT_TIME_LASTJUMP], pm->cmd.serverTime); }
      } // Increase height by +100 (default cpm). We can jump, and timer is on.
    else { pm->ps->stats[STAT_TIME_LASTJUMP] = pm->cmd.serverTime; } // Reset the timer: We can jump, but dj timer is off. 
  } 
  //:: vq3 or CPM jump selection end

  PM_AddEvent(EV_JUMP);
  if (pm->cmd.forwardmove >= 0) {
    PM_ForceLegsAnim(LEGS_JUMP);
    pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
  } else {
    PM_ForceLegsAnim(LEGS_JUMPB);
    pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
  }
  return qtrue;
}

static void q3a_AirControl(vec3_t wishdir, float wishspeed) {
  float k; //, kMult;
  float speed, dot, zVel;
  vec3_t fwAccel;

  // Initial values
  zVel = pm->ps->velocity[2];
  pm->ps->velocity[2] = 0;
  speed = VectorLength(pm->ps->velocity);
  VectorNormalize(pm->ps->velocity);
  k = 32;  // Magic constant. Why 32?
  
  //    Xonotic
  //kMult = wishspeed / phy_air_basespeed;
  //kMult = Com_Clamp(0, 1, kMult);
  //k *= kMult;
  //    Xonotic

  // Calculate turning amount
  dot = DotProduct(pm->ps->velocity, wishdir);
  if (dot > 0) {
    k = k * phy_aircontrol_amount * powf(dot, phy_aircontrol_power) * pml.frametime;
    //speed = MAX(0, speed);  // Xonotic: Clamp negative speeds to 0
    VectorMAM(speed, pm->ps->velocity, k, wishdir, pm->ps->velocity);
    VectorNormalize(pm->ps->velocity);
  }
  // Apply speed
  pm->ps->velocity[0] *= speed;
  pm->ps->velocity[1] *= speed;
  pm->ps->velocity[2] = zVel; // Restore starting vertical velocity

  // W only acceleration. Results in +0 to velocity if the value is 0.
  VectorScale(wishdir, phy_fw_accelerate * pml.frametime, fwAccel);
  VectorAdd(pm->ps->velocity, fwAccel, pm->ps->velocity);
}

void q1_AirMoveQW (void) {
	int       i;
	vec3_t    wishvel;
	float     fmove, smove;
	vec3_t		wishdir;
	float     wishspeed;
  //usercmd_t cmd;

    // fmove & smove = -127 to 127
    // upmove        =    0 to  20
  fmove = pm->cmd.forwardmove;  // AKA: finput, forward_cmd
  smove = pm->cmd.rightmove;    //      sinput
  //cmd   = pm->cmd;              // Inputs for this AirMove = current inputs
  PM_SetMovementDir();          // set the movementDir so clients can rotate the legs for strafing

  // Project moves down to flat plane. Zero out z components of movement vectors
  // Forward & Right x/y ranges are 0/640 and 0/480 //??Not convinced
  pml.forward[2] = 0; // AKA: forward_viewangle
  pml.right[2] = 0;
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  // Calculate player desired velocity vector (wishvel)
  for (i = 0; i < 2; i++) { // Determine x and y parts of velocity
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  }
  wishvel[2] = 0; // Zero out z part of velocity

  // Calculate desired direction (aka normalized wishvel)
  VectorCopy(wishvel, wishdir);      // Store wishvel in wishdir
  VectorNormalize(wishdir);          // Normalize wishvel so its actually a 'dir'

  // QW start:
  PM_GroundTrace();
	if ( pml.walking )	{
		pm->ps->velocity[2] = 0;
		q3a_Accelerate (wishdir, wishspeed, phy_ground_accel, pm->ps->speed);
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
		q3a_StepSlideMove(qfalse);
	} else {	// not on ground, so little effect on velocity
		q3a_Accelerate (wishdir, wishspeed, phy_air_accel, pm->ps->speed);
		// add gravity
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
		q3a_StepSlideMove(qtrue);
	}
}

static void q1_AirMove(void) {
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  usercmd_t cmd;

  float realAccel;   // Acceleration to apply
  float realSpeed;   // Called maxspeed. Actually just baseSpeed (320ups)
  float realWishSpd; // Wishpeed to apply in each case

  //float angle;
  //vec3_t vel2D;

  PM_Friction();

    // fmove & smove = -127 to 127
    // upmove        =    0 to  20
  fmove = pm->cmd.forwardmove;  // AKA: finput, forward_cmd
  smove = pm->cmd.rightmove;    //      sinput
  cmd   = pm->cmd;              // Inputs for this AirMove = current inputs
  PM_SetMovementDir();          // set the movementDir so clients can rotate the legs for strafing

  // Project moves down to flat plane. Zero out z components of movement vectors
  // Forward & Right x/y ranges are 0/640 and 0/480 //??Not convinced
  pml.forward[2] = 0; // AKA: forward_viewangle
  pml.right[2] = 0;
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  // Calculate player desired velocity vector (wishvel)
  for (i = 0; i < 2; i++) { // Determine x and y parts of velocity
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  }
  wishvel[2] = 0; // Zero out z part of velocity

  // Calculate desired direction (aka normalized wishvel)
  VectorCopy(wishvel, wishdir);      // Store wishvel in wishdir
  VectorNormalize(wishdir);          // Normalize wishvel so its actually a 'dir'

  // Calculate desired speed amount, based on wishvel (aka wishpeed)
  wishspeed = VectorLength(wishvel); // wishspeed = normalized speed (aka wishvel.length). Because speed = velocity.length

  // pm->ps->speed comes from g_active.c and has haste factor included in it.
  realAccel = phy_air_accel;
  realSpeed = (int)(pm->ps->speed * phy_air_speedscalar); // Reduce to airstrafe speed (~30) but still apply haste from pm->ps->speed
  realWishSpd = wishspeed * q3a_CmdScale(&cmd);

  // not on ground, so little effect on velocity
  q3a_Accelerate(wishdir, realWishSpd, realAccel, realSpeed);

  // we may have a ground plane that is very steep, even though we don't have a
  // groundentity. slide along the steep plane
  if (pml.groundPlane) {
    VectorReflectOS(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, q1_overbounce_scale);
  }
  // Do the movement
  q3a_StepSlideMove(qtrue);
}


static void q3a_AirMove(void) {
  int i;
  vec3_t wishvel;//, wishvel_c;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;//, wishspeed_c;
  usercmd_t cmd;
  qboolean doSideMove, doForwMove;

  qboolean doAircontrol = qfalse;
  float realAccel;   // Acceleration to apply
  float realSpeed;   // Called maxspeed. Actually just baseSpeed (320ups)
  float realWishSpd; // Wishpeed to apply in each case

  //float angle;
  //vec3_t vel2D;

  PM_Friction();

    // fmove & smove = -127 to 127
    // upmove        =    0 to  20
  fmove = pm->cmd.forwardmove;  // AKA: finput, forward_cmd
  smove = pm->cmd.rightmove;    //      sinput
  cmd   = pm->cmd;              // Inputs for this AirMove = current inputs
  PM_SetMovementDir();          // set the movementDir so clients can rotate the legs for strafing

  // Project moves down to flat plane. Zero out z components of movement vectors
  // Forward & Right x/y ranges are 0/640 and 0/480 //??Not convinced
  pml.forward[2] = 0; // AKA: forward_viewangle
  pml.right[2] = 0;
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  // Calculate player desired velocity vector (wishvel)
  for (i = 0; i < 2; i++) { // Determine x and y parts of velocity
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  }
  wishvel[2] = 0; // Zero out z part of velocity

  // Calculate desired direction (aka normalized wishvel)
  VectorCopy(wishvel, wishdir);      // Store wishvel in wishdir
  VectorNormalize(wishdir);          // Normalize wishvel so its actually a 'dir'

  // Calculate desired speed amount, based on wishvel (aka wishpeed)
  wishspeed = VectorLength(wishvel); // wishspeed = normalized speed (aka wishvel.length). Because speed = velocity.length


  // CPM specific
  doSideMove = (smove > 0.1 || smove < -0.1) ? qtrue : qfalse;
  doForwMove = (fmove > 0.1 || fmove < -0.1) ? qtrue : qfalse;

  if (phy_aircontrol && doForwMove && !doSideMove) {
    doAircontrol = qtrue;
  }
  if (pm->movetype == CPM) {
    // We do haste in xxx_move. Default is: pm->ps->speed  , which comes from g_active.c and has haste factor included in it.
    if (doSideMove && !doForwMove) {
      realAccel = phy_airstrafe_accel;
      realSpeed = phy_airstrafe_basespeed;
      realWishSpd = wishspeed * q3a_CmdScale(&cmd);
    } else {
      realAccel = phy_air_accel;
      realSpeed = pm->ps->speed;
      realWishSpd = wishspeed * q3a_CmdScale(&cmd);

      // Deceleration behavior
      /*
      VectorCopy(pm->ps->velocity, vel2D);    // Store velocity in 2D vector
      vel2D[0] = 0;                           // Zero out its vertical velocity
      angle = acos_alt(DotProduct(wishdir, vel2D) /
                      (VectorLength(wishdir) * VectorLength2D(vel2D)));
      angle *= (180 / M_PI);                  // Convert radians to degrees
      if (angle > phy_air_decelAngle){        // If the angle is over the decel angle
        realAccel *= phy_air_decel;           // Scale down air accel by decel factor
      }
      */
    }
  } else if (pm->movetype == VQ3) {
    realAccel = phy_air_accel;
    realSpeed = pm->ps->speed;
    realWishSpd = wishspeed * q3a_CmdScale(&cmd);

  } else if (pm->movetype == VJK) {
    realAccel = phy_air_accel;
    realSpeed = pm->ps->speed;
    realWishSpd = wishspeed * q3a_CmdScale(&cmd);   // Some games don't scale inputs

  } else { Com_Printf("Undefined movetype in q3a_ function. pm->movetype = %i", pm->movetype); return; }  // Undefined physics
  //::::::::::::::::::

  // not on ground, so little effect on velocity
  q3a_Accelerate(wishdir, realWishSpd, realAccel, realSpeed);
  if (doAircontrol) {
    q3a_AirControl(wishdir, realWishSpd);
  }

  // we may have a ground plane that is very steep, even though we don't have a
  // groundentity. slide along the steep plane
  if (pml.groundPlane) {
    q3a_VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
  }
  // Do the movement
  switch (phy_slidemove_type) {
  case Q3A:  q3a_StepSlideMove(qtrue); break;
  //case OSDF: osdf_StepSlideMove(qtrue); break;
  default:   PM_StepSlideMove(qtrue); break;
  }
}

static void q1_WalkMove(void) {
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  float scale;
  usercmd_t cmd;
  float accelerate;
  float vel;

  if (pm->waterlevel > 2 && DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0) {
    PM_WaterMove(); // begin swimming
    return;
  }

  if (q1_CheckJump()) {
    if (pm->waterlevel > 1) { PM_WaterMove(); } // jumped away
    else                    { q1_AirMove(); }
    return;
  }

  q3a_Friction();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;
  cmd = pm->cmd;
  scale = q3a_CmdScale(&cmd);

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();
  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  // project the forward and right directions onto the ground plane
  q3a_VectorReflect(pml.forward, pml.groundTrace.plane.normal, pml.forward, q1_overbounce_scale);
  q3a_VectorReflect(pml.right, pml.groundTrace.plane.normal, pml.right, q1_overbounce_scale);
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  for (i = 0; i < 3; i++) {
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  }
  // when going up or down slopes the wish velocity should Not be zero
  //	wishvel[2] = 0;
  VectorCopy(wishvel, wishdir); // Determine magnitude of speed of move
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;
  // clamp the speed lower if ducking
  if (pm->ps->pm_flags & PMF_DUCKED) {
    if (wishspeed > pm->ps->speed * phy_crouch_scale) {
      wishspeed = pm->ps->speed * phy_crouch_scale;
    }
  }
  // clamp the speed lower if wading or walking on the bottom
  if (pm->waterlevel) {
    float waterScale;
    waterScale = pm->waterlevel / 3.0;
    waterScale = 1.0 - (1.0 - phy_water_scale) * waterScale;
    if (wishspeed > pm->ps->speed * waterScale) {
      wishspeed = pm->ps->speed * waterScale;
    }
  }
  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  accelerate = phy_ground_accel;
  q3a_Accelerate(wishdir, wishspeed, accelerate, pm->ps->speed);
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK)
       || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  } else { //pm->ps->velocity[2] = 0; // don't reset the z velocity for slopes
  }

  // this is the part that causes overbounces
  vel = VectorLength(pm->ps->velocity);
  // slide along the ground plane
  q3a_VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, q1_overbounce_scale);
  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
    return;
  }
  
  // Do the movement
  q3a_StepSlideMove(qtrue);
}

static void q3a_WalkMove(void) {
  int i;
  vec3_t wishvel;
  float fmove, smove;
  vec3_t wishdir;
  float wishspeed;
  float scale;
  usercmd_t cmd;
  float accelerate;
  float vel;

  if (pm->waterlevel > 2 && DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0) {
    PM_WaterMove(); // begin swimming
    return;
  }

  if (q3a_CheckJump()) {
    if (pm->waterlevel > 1) {
      PM_WaterMove();
    } // jumped away
    else {
      q3a_AirMove();
    }
    return;
  }

  PM_Friction();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;
  cmd = pm->cmd;
  scale = q3a_CmdScale(&cmd);

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();
  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  // project the forward and right directions onto the ground plane
  q3a_VectorReflect(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
  q3a_VectorReflect(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  for (i = 0; i < 3; i++) {
    wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
  }
  // when going up or down slopes the wish velocity should Not be zero
  //	wishvel[2] = 0;
  VectorCopy(wishvel, wishdir); // Determine magnitude of speed of move
  wishspeed = VectorNormalize(wishdir);
  wishspeed *= scale;
  // clamp the speed lower if ducking
  if (pm->ps->pm_flags & PMF_DUCKED) {
    if (wishspeed > pm->ps->speed * phy_crouch_scale) {
      wishspeed = pm->ps->speed * phy_crouch_scale;
    }
  }
  // clamp the speed lower if wading or walking on the bottom
  if (pm->waterlevel) {
    float waterScale;
    waterScale = pm->waterlevel / 3.0;
    waterScale = 1.0 - (1.0 - phy_water_scale) * waterScale;
    if (wishspeed > pm->ps->speed * waterScale) {
      wishspeed = pm->ps->speed * waterScale;
    }
  }
  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK 
       || pm->ps->pm_flags & PMF_TIME_KNOCKBACK)
       && pm->movetype != CPM) {
    accelerate = phy_air_accel;
  } else {
    accelerate = phy_ground_accel;
  }
  q3a_Accelerate(wishdir, wishspeed, accelerate, pm->ps->speed);
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK)
       || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  } else { //pm->ps->velocity[2] = 0; // don't reset the z velocity for slopes
  }

  // this is the part that causes overbounces
  vel = VectorLength(pm->ps->velocity);
  // slide along the ground plane
  q3a_VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
    return;
  }
  
  // Do the movement
  switch (phy_slidemove_type) {
  case Q3A:  q3a_StepSlideMove(qtrue); break;
  //case OSDF: osdf_StepSlideMove(qtrue); break;
  default:   PM_StepSlideMove(qtrue); break;
  }
}

static void q3a_FinishWeaponChange( void ) {
	int		weapon;

	weapon = pm->cmd.weapon;
	if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) { weapon = WP_NONE; }
	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) { weapon = WP_NONE; }

	pm->ps->weapon = weapon;
	pm->ps->weaponstate = WEAPON_RAISING;
  pm->ps->weaponTime += pm->movetype == CPM ? 0 : 250;  // Instant weapon switch for cpm
	PM_StartTorsoAnim( TORSO_RAISE );
}

static void q3a_BeginWeaponChange( int weapon ) {
	if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) { return; }
	if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) { return; }
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) { return; }

	PM_AddEvent( EV_CHANGE_WEAPON );
	pm->ps->weaponstate = WEAPON_DROPPING;
	pm->ps->weaponTime += pm->movetype == CPM ? 0 : 250;
	PM_StartTorsoAnim( TORSO_DROP );
}

static void q3a_Weapon( void ) {
	int		addTime;

	// don't allow attack until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) { return; }
	// ignore if spectator
	if ( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) { return; }
	// check for dead player
	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) { pm->ps->weapon = WP_NONE; return; }
	// check for item using
	if ( pm->cmd.buttons & BUTTON_USE_HOLDABLE ) {
		if ( ! ( pm->ps->pm_flags & PMF_USE_ITEM_HELD ) ) {
			if ( bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT
				&& pm->ps->stats[STAT_HEALTH] >= (pm->ps->stats[STAT_MAX_HEALTH] + 25) ) {
				// don't use medkit if at max health
			} else {
				pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
				PM_AddEvent( EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag );
				pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
			}
			return;
		}
	} else {
		pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
	}


	// make weapon function
	if ( pm->ps->weaponTime > 0 ) {
		pm->ps->weaponTime -= pml.msec;
	}

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising
	if ( pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING ) {
		if ( pm->ps->weapon != pm->cmd.weapon ) {
			q3a_BeginWeaponChange( pm->cmd.weapon );
		}
	}
	if ( pm->ps->weaponTime > 0 ) { return; }

	// change weapon if time
	if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
		q3a_FinishWeaponChange();
		return;
	}

	if ( pm->ps->weaponstate == WEAPON_RAISING ) {
		pm->ps->weaponstate = WEAPON_READY;
		if ( pm->ps->weapon == WP_GAUNTLET ) {
			PM_StartTorsoAnim( TORSO_STAND2 );
		} else {
			PM_StartTorsoAnim( TORSO_STAND );
		}
		return;
	}

	// check for fire
	if ( ! (pm->cmd.buttons & BUTTON_ATTACK) ) {
		pm->ps->weaponTime = 0;
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}

	// start the animation even if out of ammo
	if ( pm->ps->weapon == WP_GAUNTLET ) {
		// the guantlet only "fires" when it actually hits something
		if ( !pm->gauntletHit ) {
			pm->ps->weaponTime = 0;
			pm->ps->weaponstate = WEAPON_READY;
			return;
		}
		PM_StartTorsoAnim( TORSO_ATTACK2 );
	} else {
		PM_StartTorsoAnim( TORSO_ATTACK );
	}

	pm->ps->weaponstate = WEAPON_FIRING;

	// check for out of ammo
	if ( ! pm->ps->ammo[ pm->ps->weapon ] ) {
		PM_AddEvent( EV_NOAMMO );
		pm->ps->weaponTime += 500;
		return;
	}

	// take an ammo away if not infinite
	if ( pm->ps->ammo[ pm->ps->weapon ] != -1 ) {
		pm->ps->ammo[ pm->ps->weapon ]--;
	}

	// fire weapon
	PM_AddEvent( EV_FIRE_WEAPON );

	switch( pm->ps->weapon ) {
	default:
	case WP_GAUNTLET:         addTime = 400;  break;
	case WP_LIGHTNING:        addTime = 50;   break;
	case WP_SHOTGUN:      		addTime = 1000;	break;
	case WP_MACHINEGUN:     	addTime = 100;	break;
	case WP_GRENADE_LAUNCHER:	addTime = 800;	break;
	case WP_ROCKET_LAUNCHER:	addTime = 800;	break;
	case WP_PLASMAGUN:      	addTime = 100;	break;
	case WP_RAILGUN:      		addTime = 1500;	break;
	case WP_BFG:          		addTime = 200;	break;
	case WP_GRAPPLING_HOOK:		addTime = 400;	break;
#ifdef TEAMARENA
  case WP_NAILGUN:      		addTime = 1000;	break;
	case WP_PROX_LAUNCHER:		addTime = 800;	break;
	case WP_CHAINGUN:     		addTime = 30;		break;
#endif
	}

#ifdef TEAMARENA
	if( bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT ) {
		addTime /= 1.5;
	}
	else
	if( bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN ) {
		addTime /= 1.3;
  }
  else
#endif
	if ( pm->ps->powerups[PW_HASTE] ) {
		addTime /= 1.3;
	}

	pm->ps->weaponTime += addTime;
}


void q3a_cpm(pmove_t *pmove) {
  if (pm->ps->powerups[PW_FLIGHT]) {
    PM_FlyMove(); // flight powerup doesn't allow jump and has different friction
  } else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
    PM_GrappleMove();
    PM_AirMove(); // We can wiggle a bit
  } else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
    PM_WaterJumpMove();
  } else if (pm->waterlevel > 1) {
    PM_WaterMove(); // swimming
  } else if (pml.walking) {
    q3a_WalkMove(); // walking on ground
  } else {
    q3a_AirMove(); // airborne
  }
  // animations
  PM_Animate();
  // set groundentity, watertype, and waterlevel
  PM_GroundTrace();
  PM_SetWaterLevel();
  // weapons
  q3a_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // Snapzones: Snap some parts of playerstate to save network bandwidth
  if (phy_snapvelocity) { trap_SnapVector(pm->ps->velocity); } // Default Q3 behavior
  else                  { pm->ps->velocity[2] = roundf(pm->ps->velocity[2]); } // No snapzones. Always snap vertical velocity, to preserve default max jump height
}

void q3a_vq3(pmove_t *pmove) {
  if (pm->ps->powerups[PW_FLIGHT]) {
    PM_FlyMove(); // flight powerup doesn't allow jump and has different
                  // friction
  } else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
    PM_GrappleMove();
    PM_AirMove(); // We can wiggle a bit
  } else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
    PM_WaterJumpMove();
  } else if (pm->waterlevel > 1) {
    PM_WaterMove(); // swimming
  } else if (pml.walking) {
    q3a_WalkMove(); // walking on ground
  } else {
    q3a_AirMove(); // airborne
  }
  // animations
  PM_Animate();
  // set groundentity, watertype, and waterlevel
  PM_GroundTrace();
  PM_SetWaterLevel();
  // weapons
  q3a_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // snap some parts of playerstate to save network bandwidth
  trap_SnapVector(pm->ps->velocity);
}

void q3a_vq1(pmove_t *pmove) {
  if (pm->ps->powerups[PW_FLIGHT]) {
    PM_FlyMove(); // flight powerup doesn't allow jump and has different friction
  } else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
    PM_GrappleMove();
    PM_AirMove(); // We can wiggle a bit
  } else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
    PM_WaterJumpMove();
  } else if (pm->waterlevel > 1) {
    PM_WaterMove(); // swimming
  } else if (pml.walking) {
    q1_WalkMove(); // walking on ground
  } else {
    q1_AirMove(); // airborne
  }
  // animations
  PM_Animate();
  // set groundentity, watertype, and waterlevel
  PM_GroundTrace();
  PM_SetWaterLevel();
  // weapons
  PM_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // Snap vertical velocity only, to preserve default max jump height
  pm->ps->velocity[2] = roundf(pm->ps->velocity[2]); 
}

void q3a_vjk(pmove_t *pmove) {
  if (pm->ps->powerups[PW_FLIGHT]) {
    PM_FlyMove(); // flight powerup doesn't allow jump and has different friction
  } else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
    PM_GrappleMove();
    PM_AirMove(); // We can wiggle a bit
  } else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
    PM_WaterJumpMove();
  } else if (pm->waterlevel > 1) {
    PM_WaterMove(); // swimming
  } else if (pml.walking) {
    q3a_WalkMove(); // walking on ground
  } else {
    q3a_AirMove(); // airborne
  }
  // animations
  PM_Animate();
  // set groundentity, watertype, and waterlevel
  PM_GroundTrace();
  PM_SetWaterLevel();
  // weapons
  q3a_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // snap some parts of playerstate to save network bandwidth
  trap_SnapVector(pm->ps->velocity);
}

// Select the type of movement to execute. Flow control only. 
// Behavior happens inside each function
void osdf_move(pmove_t *pmove) {
  if (!osdf_initialized) { osdf_init(pmove->movetype); }
  switch (pmove->movetype) {
  case CPM: q3a_cpm(pmove); break;
  case VQ1: q3a_vq1(pmove); break;
  case VQ3: q3a_vq3(pmove); break;
  case VJK: q3a_vjk(pmove); break;
  default:  Com_Printf("::ERR phy_movetype %i not recognized\n", pmove->movetype); break;
  }
}
//::::::::::::::::::::::
//::OSDF end

