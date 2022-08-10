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

void vq4_init(){
  phy_stopspeed              = pm_stopspeed;
  // Crouch
  phy_crouch_scale           = pm_duckScale;
  phy_crouch_feetraise       = -MINS_Z*0.5;
  // Acceleration
  phy_ground_accel           = 15;
  phy_air_accel              = pm_airaccelerate;
  phy_fly_accel              = pm_flyaccelerate;
  // Friction
  phy_friction               = pm_friction;
  phy_fly_friction           = pm_flightfriction;
  phy_spectator_friction     = pm_spectatorfriction;
  // Water
  phy_water_accel            = pm_wateraccelerate;
  phy_water_scale            = pm_swimScale;
  phy_water_friction         = pm_waterfriction;
  // New
  phy_skim_enable            = qtrue;
  phy_step_size              = STEPSIZE;
  phy_snapvelocity           = qfalse;
  phy_input_scalefix         = qtrue;
  phy_jump_type              = VQ3;
  phy_jump_auto              = qtrue;
  phy_jump_velocity          = JUMP_VELOCITY;
  phy_step_maxvel            = phy_jump_velocity;
  // Crouchslide
  phy_crouchslide_friction   = 0;
  phy_crouchslide_accel      = 20;
  phy_crouchslide_timemax    = 2000;
  phy_crouchslide_framecount = 2;
  // Rampslide
  phy_rampslide              = qtrue;
  phy_rampslide_type         = VQ2;
  phy_rampslide_speedmin     = phy_jump_velocity;
}

static void vq4_Friction( void ) {
  vec3_t vec;
  float* vel = pm->ps->velocity;
  VectorCopy(vel, vec);
  if (pml.walking) {vec[2] = 0;} // ignore slope movement
  float speed = VectorLength(vec);
  if (speed < 1) {vel[0] = 0; vel[1] = 0; return;} // allow sinking underwater.   FIXME: still have z friction underwater?

  float drop = 0;
  float control;
  // apply ground friction
  if ( pm->waterlevel <= 1 ) {
    if (pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK)) {
      if (!(pm->ps->pm_flags & PMF_TIME_KNOCKBACK)) { // if getting knocked back, no friction
        qboolean crouch      = (pm->ps->pm_flags & PMF_DUCKED) ? qtrue:qfalse;
        qboolean crouchslide = (crouch && pm->ps->stats[STAT_TIME_CROUCHSLIDE] > 0) ? qtrue:qfalse;
        qboolean recovering  = (crouch && pm->cmd.upmove == 0) ? qtrue:qfalse;
        control = speed < phy_stopspeed ? phy_stopspeed : speed;
        if (crouchslide || recovering) { drop += control*phy_crouchslide_friction*pml.frametime; } 
        else                           { drop += control*phy_friction            *pml.frametime; }
      }
    }
  }
  // apply water friction even if just wading
  if (pm->waterlevel) { drop += speed*phy_water_friction*pm->waterlevel*pml.frametime; }
  // apply flying friction
  if ( pm->ps->powerups[PW_FLIGHT])     { drop += speed*phy_fly_friction*pml.frametime; }
  if ( pm->ps->pm_type == PM_SPECTATOR) { drop += speed*phy_spectator_friction*pml.frametime; }
  // scale the velocity
  float newspeed = speed - drop;
  if (newspeed < 0) { newspeed = 0; }
  newspeed /= speed;
  VectorScale(vel, newspeed, vel);
}


static qboolean vq4_CheckJump(void) {
  // Can't jump cases. Cannot jump again under these conditions
  if (pm->ps->pm_flags & PMF_RESPAWNED) { return qfalse; } // don't allow jump until all buttons are up
  if (pm->cmd.upmove < 10) { return qfalse; } // not holding jump
  if ((pm->ps->pm_flags & PMF_JUMP_HELD && !phy_jump_auto)) { // must wait for jump to be released
    pm->cmd.upmove = 0; // clear upmove so cmdscale doesn't lower running speed
    return qfalse;
  }
  if (pm->ps->stats[STAT_RAMPSLIDE]) { return qfalse; } // We are rampsliding

  // Else: Can jump. Do jump behavior
  pml.groundPlane          = qfalse; // jumping away
  pml.walking              = qfalse;
  pm->ps->pm_flags        |= PMF_JUMP_HELD;
  pm->ps->groundEntityNum  = ENTITYNUM_NONE;
  // SET vertical velocity
  pm->ps->velocity[2]      = phy_jump_velocity;
  // Sound and Anim
  PM_AddEvent(EV_JUMP);
  if (pm->cmd.forwardmove >= 0) { PM_ForceLegsAnim(LEGS_JUMP);  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; }
  else                          { PM_ForceLegsAnim(LEGS_JUMPB); pm->ps->pm_flags |=  PMF_BACKWARDS_JUMP; }
  // Reset crouchslide
  pm->ps->stats[STAT_TIME_CROUCHSLIDE] = 0;
  // We have jumped
  if (pm->debugLevel) { Com_Printf("%i:Jump\n", c_pmove); }
  return qtrue;
}


int IntClamp(int n, int min, int max) {if (n>max) {return max;} else if (n<min) {return min;} else {return n;}}
int IntMin  (int n, int min)          {if (n<min) {return min;} else {return n;}}
int IntMax  (int n, int max)          {if (n>max) {return max;} else {return n;}}
void vq4_AirMove(void) {
  int       i;
  vec3_t    wishvel;//, wishvel_c;
  float     fmove, smove;
  vec3_t    wishdir;
  float     wishspeed;//, wishspeed_c;
  usercmd_t cmd;
  qboolean  doSideMove, doForwMove;

  qboolean  doAircontrol = qfalse;
  float     realAccel;   // Acceleration to apply
  float     realSpeed;   // Called maxspeed. Actually just baseSpeed (320ups)
  float     realWishSpd; // Wishpeed to apply in each case

  // Add crouchslide time
  qboolean notCrouch  = (pm->cmd.upmove >= 0)     ? qtrue:qfalse;
  qboolean movingDown = (pm->ps->velocity[2] < 0) ? qtrue:qfalse;
  // if (notCrouch && movingDown) {
  // if (movingDown) {
  {
    int time = pm->ps->stats[STAT_TIME_CROUCHSLIDE];
    pm->ps->stats[STAT_TIME_CROUCHSLIDE] = IntMax(time+pml.msec*phy_crouchslide_framecount, phy_crouchslide_timemax);
  } 

  vq4_Friction();

    // fmove & smove = -127 to 127
    // upmove        =    0 to  20
  fmove = pm->cmd.forwardmove;  // AKA: finput, forward_cmd
  smove = pm->cmd.rightmove;    //      sinput
  cmd   = pm->cmd;              // Inputs for this AirMove = current inputs
  PM_SetMovementDir();          // set the movementDir so clients can rotate the legs for strafing

  // Project moves down to flat plane. Zero out z components of movement vectors
  pml.forward[2] = 0; // AKA: forward_viewangle
  pml.right[2] = 0;
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);
  // Desired velocity vector (wishvel)
  for (i = 0; i < 2; i++) { // Determine x and y parts of velocity
    wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
  }
  wishvel[2] = 0; // Zero out z part of velocity
  // Desired direction (aka normalized wishvel)
  VectorCopy(wishvel, wishdir);      // Store wishvel in wishdir
  VectorNormalize(wishdir);          // Normalize wishvel so its actually a 'dir'
  // Desired speed amount, based on wishvel (aka wishpeed)
  wishspeed = VectorLength(wishvel); // wishspeed = normalized speed (aka wishvel.length). Because speed = velocity.length

  // We do haste in xxx_move. Default is: pm->ps->speed  , which comes from g_active.c and has haste factor included in it.
  realAccel = phy_air_accel;
  realSpeed = pm->ps->speed;
  realWishSpd = wishspeed * core_CmdScale(&cmd, phy_input_scalefix);
  //::::::::::::::::::

  // not on ground, so little effect on velocity
  core_Accelerate(wishdir, realWishSpd, realAccel, realSpeed);

  // we may have a ground plane that is very steep, even though we don't have a groundentity. slide along the steep plane
  if (pml.groundPlane) { VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, overbounce_scale); }
  // Do the movement
  core_StepSlideMove(qtrue);
}

void vq4_WalkMove(void) {
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

  if (vq4_CheckJump()) {
    if (pm->waterlevel > 1) { PM_WaterMove(); } // jumped away
    else                    { vq4_AirMove();  }
    return;
  }

  vq4_Friction();

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
  qboolean crouchslide = (pm->ps->pm_flags & PMF_DUCKED && pm->ps->stats[STAT_TIME_CROUCHSLIDE] > 0) ? qtrue:qfalse;
  if (pml.groundTrace.surfaceFlags & SURF_SLICK || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
    // when a player gets hit, they temporarily lose full control, which allows them to be moved a bit
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

void vq4_DropTimers(void) {
  // drop misc timing counter
  if (pm->ps->pm_time) {
    if (pml.msec >= pm->ps->pm_time) { pm->ps->pm_flags &= ~PMF_ALL_TIMES; pm->ps->pm_time = 0; } 
    else                             { pm->ps->pm_time -= pml.msec; }
  }
  // drop crouchslide timer
  if (pm->ps->stats[STAT_TIME_CROUCHSLIDE]) {
    if (pml.msec >= pm->ps->stats[STAT_TIME_CROUCHSLIDE]) { pm->ps->stats[STAT_TIME_CROUCHSLIDE]  = 0; }
    else                                                  { pm->ps->stats[STAT_TIME_CROUCHSLIDE] -= pml.msec;}
  }
  // drop animation counter
  if (pm->ps->legsTimer > 0) {
    pm->ps->legsTimer -= pml.msec;
    if (pm->ps->legsTimer < 0) { pm->ps->legsTimer = 0; }
  }
  if (pm->ps->torsoTimer > 0) {
    pm->ps->torsoTimer -= pml.msec;
    if (pm->ps->torsoTimer < 0) { pm->ps->torsoTimer = 0; }
  }
}

static void vq4_CheckDuck(void) {
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
  pm->maxs[0] = 15;
  pm->maxs[1] = 15;
  pm->mins[2] = MINS_Z;

  if (pm->ps->pm_type == PM_DEAD) {
    pm->maxs[2] = -8;
    pm->ps->viewheight = DEAD_VIEWHEIGHT;
    return;
  }

  if (pm->cmd.upmove < 0) { // duck
     qboolean cantFeetraise = (pml.groundPlane || pm->ps->stats[STAT_RAMPSLIDE]) ? qtrue:qfalse;
     pm->mins[2] = (cantFeetraise) ? pm->mins[2] : MINS_Z +phy_crouch_feetraise;  // Keep it the same on the ground. Else apply feetraise
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


void vq4_move(pmove_t* pmove) {
  // set mins, maxs, and viewheight
  vq4_CheckDuck();
  // set groundentity
  core_GroundTrace();
  // do deadmove  :moved to top
  // drop timers
  vq4_DropTimers();

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
    vq4_WalkMove(); // walking on ground
  } else {
    vq4_AirMove(); // airborne
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
  if (phy_snapvelocity) { trap_SnapVector(pm->ps->velocity); } // Default Q3 behavior
  else                  { pm->ps->velocity[2] = roundf(pm->ps->velocity[2]); } // No snapzones. Always snap vertical velocity, to preserve default max jump height
  // if (!(pml.walking) && pm->debugLevel) { Com_Printf("%i:Moved.Trigger\n", c_pmove); }
}


