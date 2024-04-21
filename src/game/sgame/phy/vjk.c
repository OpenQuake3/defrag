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
#include "q3a.h"

void vjk_init(){
  phy_stopspeed          = pm_stopspeed;
  phy_crouch_scale       = pm_duckScale;
  // Acceleration
  phy_ground_accel       = 12;
  phy_ground_basespeed   = 250; // TODO: Link to phy_speed
  phy_air_accel          = 4;  // and   phy_speed = 250
  phy_fly_accel          = pm_flyaccelerate;
  // Friction
  phy_friction           = pm_friction;
  phy_fly_friction       = pm_flightfriction;
  phy_spectator_friction = pm_spectatorfriction;
  // Water
  phy_water_accel        = pm_wateraccelerate;
  phy_water_scale        = pm_swimScale;
  phy_water_friction     = pm_waterfriction;
  // New
  phy_skim_enable        = qtrue;
  phy_snapvelocity       = qfalse;
  phy_input_scalefix     = qtrue;
  phy_aircontrol         = qfalse;
  phy_jump_type          = VQ3;
  phy_jump_auto          = qtrue;
  phy_jump_velocity      = 225; // vjk = 225. vq3 default = JUMP_VELOCITY = 270
  phy_jump_holdboost     = qtrue;
  phy_jump_hb_amount     = (int)((JUMP_VELOCITY - phy_jump_velocity)*0.12);  // Same height as vq3 in ~8 jumps = 45*0.12 = (270-225)*0.12
}

static qboolean vjk_CheckJump(void) {
  // Can't jump cases. Cannot jump again under these conditions
  if (pm->ps->pm_flags & PMF_RESPAWNED) { return qfalse; } // don't allow jump until all buttons are up
  if (pm->cmd.upmove < 10) { return qfalse; } // not holding jump
  if ((pm->ps->pm_flags & PMF_JUMP_HELD && !phy_jump_auto)) { // must wait for jump to be released
    pm->cmd.upmove = 0; // clear upmove so cmdscale doesn't lower running speed
    return qfalse;
  }
  // Choose holdboost amount
  qboolean canHoldBoost = (pm->cmd.upmove > 10 && phy_jump_holdboost
                        && VectorLength2D(pm->ps->velocity) > phy_ground_basespeed*1.5);
  if (canHoldBoost) { pm->ps->stats[STAT_JUMP_HOLDBOOST] += phy_jump_hb_amount; }
  else              { pm->ps->stats[STAT_JUMP_HOLDBOOST] = 0; }

  // Else: Can jump. Do jump behavior
  pml.groundPlane          = qfalse; // jumping away
  pml.walking              = qfalse;
  pm->ps->pm_flags        |= PMF_JUMP_HELD;
  pm->ps->groundEntityNum  = ENTITYNUM_NONE;
  // ups * frametime = upf
  if (canHoldBoost) {
    pm->ps->velocity[2] = phy_jump_velocity + pm->ps->stats[STAT_JUMP_HOLDBOOST]; // Base jumpvel + stored boost
  } else {
    pm->ps->velocity[2] = phy_jump_velocity; // SET vertical velocity. Default vq3 behavior
  }
  // Sound and Anim
  PM_AddEvent(EV_JUMP);
  if (pm->cmd.forwardmove >= 0) { PM_ForceLegsAnim(LEGS_JUMP);  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; }
  else                          { PM_ForceLegsAnim(LEGS_JUMPB); pm->ps->pm_flags |=  PMF_BACKWARDS_JUMP; }
  // We have jumped
  if (pm->debugLevel) { Com_Printf("%i:Jump\n", c_pmove); }
  // pm_time_lastjump = pm->cmd.serverTime;  // Store timer for when we last jumped
  return qtrue;
}



void vjk_WalkMove(void) {
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

  if (vjk_CheckJump()) {
    if (pm->waterlevel > 1) { PM_WaterMove(); } // jumped away
    else                    { q3a_AirMove();  }
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
  //   wishvel[2] = 0;
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
  // when a player gets hit, they temporarily lose full control, which allows them to be moved a bit
  qboolean crouchslide = (pm->ps->pm_flags & PMF_DUCKED && pm->ps->stats[STAT_TIME_CROUCHSLIDE] > 0) ? qtrue:qfalse;
  if (pml.groundTrace.surfaceFlags & SURF_SLICK || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    accelerate = phy_air_accel;
  } else if (crouchslide) {
    accelerate = phy_crouchslide_accel;
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
  VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, overbounce_scale);
  // don't decrease velocity when going up or down a slope
  VectorNormalize(pm->ps->velocity);
  VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
  // don't do anything if standing still
  if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
    return;
  }
  // Do the movement
  core_StepSlideMove(qtrue);
}

void vjk_move(pmove_t *pmove) {
  // set mins, maxs, and viewheight
  PM_CheckDuck();
  // set groundentity
  core_GroundTrace();
  // do deadmove  :moved to top
  // drop timers
  PM_DropTimers();

  if (VectorLength2D(pm->ps->velocity) < pm->ps->speed*1.5) { // TODO: Fix this. upmove is 0 in some frames when holding jump
  // if (pm->cmd.upmove < 10 || VectorLength2D(pm->ps->velocity) < pm->ps->speed*1.5) {
    // if (pm->ps->stats[STAT_JUMP_HOLDBOOST]) { Com_Printf("%i: Removed holdboost\n", c_pmove); }
    pm->ps->stats[STAT_JUMP_HOLDBOOST] = 0;  // Remove holdboost when not holding jump this frame, or too slow
  }
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
    vjk_WalkMove(); // walking on ground
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

