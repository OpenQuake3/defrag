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

void dif_init(){
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
  phy_jump_timebuffer = 400;
  phy_jump_dj_velocity = 100;
}

static qboolean dif_CheckJump(void) {
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
    qboolean djtimerOn = (( djtimer <= phy_jump_timebuffer ) && (djtimer > 0)) ? qtrue : qfalse; // We can dj when this is true.
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


void q3a_AirMove(void) {
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
      realWishSpd = wishspeed * core_CmdScale(&cmd, phy_input_scalefix);
    } else {
      realAccel = phy_air_accel;
      realSpeed = pm->ps->speed;
      realWishSpd = wishspeed * core_CmdScale(&cmd, phy_input_scalefix);

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
    realWishSpd = wishspeed * core_CmdScale(&cmd, phy_input_scalefix);

  } else if (pm->movetype == VJK) {
    realAccel = phy_air_accel;
    realSpeed = pm->ps->speed;
    realWishSpd = wishspeed * core_CmdScale(&cmd, phy_input_scalefix);   // Some games don't scale inputs

  } else { Com_Printf("Undefined movetype in q3a_ function. pm->movetype = %i", pm->movetype); return; }  // Undefined physics
  //::::::::::::::::::

  // not on ground, so little effect on velocity
  core_Accelerate(wishdir, realWishSpd, realAccel, realSpeed);
  if (doAircontrol) {
    q3a_AirControl(wishdir, realWishSpd);
  }

  // we may have a ground plane that is very steep, even though we don't have a
  // groundentity. slide along the steep plane
  if (pml.groundPlane) {
    VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
  }
  // Do the movement
  switch (phy_slidemove_type) {
  case Q3A:  core_StepSlideMove(qtrue); break;
  //case OSDF: osdf_StepSlideMove(qtrue); break;
  default:   PM_StepSlideMove(qtrue); break;
  }
}

void q3a_WalkMove(void) {
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
  scale = core_CmdScale(&cmd, phy_input_scalefix);

  // set the movementDir so clients can rotate the legs for strafing
  PM_SetMovementDir();
  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  // project the forward and right directions onto the ground plane
  VectorReflect(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
  VectorReflect(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
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
  core_Accelerate(wishdir, wishspeed, accelerate, pm->ps->speed);
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK)
       || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
  } else { //pm->ps->velocity[2] = 0; // don't reset the z velocity for slopes
  }

  // this is the part that causes overbounces
  vel = VectorLength(pm->ps->velocity);
  // slide along the ground plane
  VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
    return;
  }
  
  // Do the movement
  switch (phy_slidemove_type) {
  case Q3A:  core_StepSlideMove(qtrue); break;
  //case OSDF: osdf_StepSlideMove(qtrue); break;
  default:   PM_StepSlideMove(qtrue); break;
  }
}


void cpm_move(pmove_t *pmove) {
  // set mins, maxs, and viewheight
  PM_CheckDuck();
  // set groundentity
  core_GroundTrace();

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
    q3a_WalkMove(); // walking on ground
  } else {
    q3a_AirMove(); // airborne
  }
  // animations
  PM_Animate();
  // set groundentity, watertype, and waterlevel
  core_GroundTrace();
  PM_SetWaterLevel();
  // weapons
  core_Weapon();
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

void vq3_move(pmove_t *pmove) {
  // set mins, maxs, and viewheight
  PM_CheckDuck();
  // set groundentity
  core_GroundTrace();

  // do deadmove  :moved to top
  // drop timers
  PM_DropTimers();

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
  core_GroundTrace();
  PM_SetWaterLevel();
  // weapons
  core_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // snap some parts of playerstate to save network bandwidth
  trap_SnapVector(pm->ps->velocity);
}



