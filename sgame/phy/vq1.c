/*
  ==============================
  Written by:
    id software :            Quake III Arena
    sOkam! :                 Opensource Defrag

  This file is part of Opensource Defrag.

  Opensource Defrag is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Opensource Defrag is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Opensource Defrag.  If not, see <http://www.gnu.org/licenses/>.
  ==============================
*/

#include "local.h"

void vq1_init(){
  phy_stopspeed          = 100; // QW value
  // Crouch
  phy_crouch_scale       = pm_duckScale;
  phy_crouch_feetraise   = 14;
  // Acceleration
  phy_fly_accel          = pm_flyaccelerate;
  // Friction
  phy_friction           = 4; // QW value. from 6 (q3a/pm_friction)
  phy_fly_friction       = pm_flightfriction;
  phy_spectator_friction = pm_spectatorfriction;
  // Water
  phy_water_accel        = 10; // QW value. from 4 (q3a/pm_wateraccelerate)
  phy_water_scale        = pm_swimScale;
  phy_water_friction     = 4; // QW value
  // New
  phy_skim_enable        = qtrue;
  phy_snapvelocity       = qfalse;
  phy_input_scalefix     = qtrue;
  overbounce_scale       = OVERCLIP;
  // Ground
  phy_ground_basespeed   = 320;
  phy_ground_accel       = 10;
  // Air movement
  phy_air_accel          = 90; // 70 = QW value 0.7 : q3a = 1 :: 100 was too much. 70 not enough :: 0.1/32as/90aa/125fps felt really good
  phy_air_speedscalar    = 0.11; //0.1125;=36 slightly too much //0.125=40 too much; // Value: 0.0938 = ~30, but its too slow. 0.1/32as/70aa/77fps super close to QW/vint
  // Jump
  phy_jump_auto          = qtrue;
  phy_jump_type          = VQ3;
  phy_jump_velocity      = JUMP_VELOCITY;
  phy_step_maxvel        = phy_jump_velocity;
  phy_jump_scalar        = 0.5;
}

static void q1_CheckDuck(void) {
  if (pm->ps->powerups[PW_INVULNERABILITY]) {
    if (pm->ps->pm_flags & PMF_INVULEXPAND) {
      // invulnerability sphere has a 42 units radius
      VectorSet(pm->mins, -42, -42, -42);
      VectorSet(pm->maxs,  42,  42,  42);
    } else {
      VectorSet(pm->mins, -15, -15, MINS_Z);
      VectorSet(pm->maxs,  15,  15, 16);
    }
    pm->ps->pm_flags |= PMF_DUCKED;
    pm->ps->viewheight = CROUCH_VIEWHEIGHT;
    return;
  }
  pm->ps->pm_flags &= ~PMF_INVULEXPAND;

  pm->mins[0] = -15;
  pm->mins[1] = -15;
  pm->maxs[0] =  15;
  pm->maxs[1] =  15;
  pm->mins[2] =  MINS_Z;

  if (pm->ps->pm_type == PM_DEAD) {
    pm->maxs[2] = -8;
    pm->ps->viewheight = DEAD_VIEWHEIGHT;
    return;
  }

  if (pm->cmd.upmove < 0) { // duck
     pm->mins[2] = (pml.groundPlane) ? pm->mins[2] : MINS_Z +phy_crouch_feetraise;  // Keep it the same on the ground. Else apply feetraise
     pm->ps->pm_flags |= PMF_DUCKED;
  } else { // stand up if possible
    if (pm->ps->pm_flags & PMF_DUCKED) {
      // try to stand up
      pm->maxs[2] = 32;
      trace_t trace; pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
      if (!trace.allsolid)// && !(pm->cmd.upmove < 0) )
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

  // QW JumpHeights
  // Downramps act like vq3 (set). Upramps act like QW (add)
  if (pm->ps->velocity[2] < 0){
    if (pm->cmd.buttons & BUTTON_WALKING) { pm->ps->velocity[2] += phy_jump_velocity; }
    else                                  { pm->ps->velocity[2]  = phy_jump_velocity; } // Downramp set
  } else {  // Flat or Upramp
    if (pm->cmd.buttons & BUTTON_WALKING) { pm->ps->velocity[2] += phy_jump_velocity*phy_jump_scalar; }
    else                                  { pm->ps->velocity[2] += phy_jump_velocity; }
  }  
  // Sound and Anim
  PM_AddEvent(EV_JUMP); // Ask the client to play the jump sound
  if (pm->cmd.forwardmove >= 0) { PM_ForceLegsAnim(LEGS_JUMP);  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; }
  else                          { PM_ForceLegsAnim(LEGS_JUMPB); pm->ps->pm_flags |=  PMF_BACKWARDS_JUMP; }
  // We have jumped
  if (pm->debugLevel) { Com_Printf("%i:Jump\n", c_pmove); }
  return qtrue;
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
		core_Accelerate (wishdir, wishspeed, phy_ground_accel, pm->ps->speed);
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
		core_StepSlideMove(qfalse);
	} else {	// not on ground, so little effect on velocity
		core_Accelerate (wishdir, wishspeed, phy_air_accel, pm->ps->speed);
		// add gravity
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
		core_StepSlideMove(qtrue);
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
  realWishSpd = wishspeed * core_CmdScale(&cmd, phy_input_scalefix);

  // not on ground, so little effect on velocity
  core_Accelerate(wishdir, realWishSpd, realAccel, realSpeed);

  // we may have a ground plane that is very steep, even though we don't have a
  // groundentity. slide along the steep plane
  if (pml.groundPlane) {
    // "Bouncy" version. Increases incoming overbounce value
    //    Close, but not quite there. 
    if (1) { VectorReflectBC(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, overbounce_scale); }
    // "One-sided" version. Ignores backoff when moving away from surface
    //    Issues with Vertical Velocity affecting surf physics
    else   { VectorReflectOS(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, overbounce_scale); }
  }
  // Do the movement
  if (1) { 
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
    core_StepSlideMove(qfalse);
  }
  else { core_StepSlideMove(qtrue);}
}


static void q1_WalkMove(void) {
  int       i;
  vec3_t    wishvel;
  float     fmove, smove;
  vec3_t    wishdir;
  float     wishspeed;
  float     scale;
  usercmd_t cmd;
  float     accelerate;
  float     vel;

  if (pm->waterlevel > 2 && DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0) {
    PM_WaterMove(); // begin swimming
    return;
  }

  if (q1_CheckJump()) {
    if (pm->waterlevel > 1) { PM_WaterMove(); } // jumped away
    else                    { q1_AirMove(); }
    return;
  }

  core_Friction();

  fmove = pm->cmd.forwardmove;
  smove = pm->cmd.rightmove;
  cmd = pm->cmd;
  scale = core_CmdScale(&cmd, phy_input_scalefix);

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();
  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  // project the forward and right directions onto the ground plane
  VectorReflect(pml.forward, pml.groundTrace.plane.normal, pml.forward, overbounce_scale);
  VectorReflect(pml.right, pml.groundTrace.plane.normal, pml.right, overbounce_scale);
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
  core_Accelerate(wishdir, wishspeed, accelerate, pm->ps->speed);
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK)
       || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  } else { //pm->ps->velocity[2] = 0; // don't reset the z velocity for slopes
  }

  // this is the part that causes overbounces
  vel = VectorLength(pm->ps->velocity);
  // slide along the ground plane
  VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, overbounce_scale);
  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {return;}
  // Do the movement
  core_StepSlideMove(qtrue);
}


void vq1_move(pmove_t *pmove) {
  // set mins, maxs, and viewheight
  q1_CheckDuck();

  // set groundentity
  PM_GroundTrace();
  // do deadmove  :moved to top
  // drop timers
  PM_DropTimers();

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
  core_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // Snap vertical velocity only, to preserve default max jump height
  pm->ps->velocity[2] = roundf(pm->ps->velocity[2]); 
}

