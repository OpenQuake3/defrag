#include "osdf.h"

void vq1_init(){
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
  phy_overbounce_scale = 1.000f;
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
    //  pm->ps->pm_flags |= PMF_DUCKED;
  } else { // stand up if possible
    if (pm->ps->pm_flags & PMF_DUCKED) {
      // try to stand up
      pm->maxs[2] = 32;
      pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
      if ( !trace.allsolid && !(pm->cmd.upmove < 0) )
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

static qboolean q1_CheckJump(void) {
  // Can't jump cases. Cannot jump again under these conditions
  if (pm->ps->pm_flags & PMF_RESPAWNED) {
    return qfalse;  // don't allow jump until all buttons are up
  }
  if (pm->cmd.upmove < 10) { 
    return qfalse; 
  }
  if ((pm->ps->pm_flags & PMF_JUMP_HELD) && !phy_jump_auto) { // must wait for jump to be released
    pm->cmd.upmove = 0; // clear upmove so cmdscale doesn't lower running speed
    return qfalse;
  }
  // Else: Can jump. Do jump behavior
  //

  jump              = (pm->ps->pm_flags & PMF_JUMP_HELD)  ? qtrue : qfalse;
  crouch            = (pm->ps->pm_flags & PMF_DUCKED)     ? qtrue : qfalse;
  standOrJumpcrouch = (pm->cmd.upmove = 0)                ? qtrue : qfalse;

  // standing   = upmove 0 && !jump && !crouch
  standing          = ( standOrJumpcrouch && !jump )      ? qtrue : qfalse;
  if (standing)       { return qfalse; } //   cant jump
  // crouch     = upmove < 0
  crouchOnly        = ( crouch && !jump )                 ? qtrue : qfalse;
  if (crouchOnly)     { return qfalse; } //   cant jump
  // jumpcrouch = upmove 0 && jump && crouch 
  //   can jump

  // Rest of the function:
  //   Will jump




  pml.groundPlane = qfalse; // jumping away
  pml.walking = qfalse;
  pm->ps->pm_flags |= PMF_JUMP_HELD;
  pm->ps->groundEntityNum = ENTITYNUM_NONE;
  
  if (pm->ps->pm_flags & PMF_DUCKED) { pm->ps->velocity[2] *= 0.5; }

  /*
  // QW downramps
  if (phy_jump_type == VQ1) {
    pm->ps->velocity[2] += phy_jump_velocity; // Always adds
  } else
  */
  {
    // Downramps act like vq3 (set). Upramps act like QW (add)
    if (pm->ps->velocity[2] < 0){
      if (DotProduct(pm->ps->velocity, pml.groundTrace.plane.normal) > 0) {
        pm->ps->velocity[2] = phy_jump_velocity; // Downramp
      } else { pm->ps->velocity[2] += phy_jump_velocity; } // Flat
    } else { pm->ps->velocity[2] += phy_jump_velocity; } // Upramp
  }
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
  qboolean doSideMove, doForwMove;

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

  // pm->ps->speed comes from g_active.c and has haste factor included in it.
  if (doSideMove && !doForwMove) {
    realAccel = phy_air_accel;
    realSpeed = (int)(pm->ps->speed * phy_air_speedscalar); // Reduce to airstrafe speed (30) but still apply haste from pm->ps->speed
    realWishSpd = wishspeed * q3a_CmdScale(&cmd);
  } else {
    //realAccel = phy_air_accel;
    //realSpeed = pm->ps->speed;
    //realWishSpd = wishspeed * q3a_CmdScale(&cmd);
  }
    // Deceleration behavior
    /*
    VectorCopy(pm->ps->velocity, vel2D);    // Store velocity in 2D vector
    vel2D[0] = 0;                           // Zero out its vertical velocity
    angle = acos_alt(DotProduct(wishdir, vel2D) / (VectorLength(wishdir) * VectorLength2D(vel2D)));
    angle *= (180 / M_PI);                  // Convert radians to degrees
    if (angle > phy_air_decelAngle){        // If the angle is over the decel angle
      realAccel *= phy_air_decel;           // Scale down air accel by decel factor
    }
    */
  //::::::::::::::::::

  // not on ground, so little effect on velocity
  q3a_Accelerate(wishdir, realWishSpd, realAccel, realSpeed);

  // we may have a ground plane that is very steep, even though we don't have a
  // groundentity. slide along the steep plane
  if (pml.groundPlane) {
    q3a_VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, phy_overbounce_scale);
  }
  // Do the movement
  q3a_StepSlideMove(qtrue);
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
  q3a_VectorReflect(pml.forward, pml.groundTrace.plane.normal, pml.forward, phy_overbounce_scale);
  q3a_VectorReflect(pml.right, pml.groundTrace.plane.normal, pml.right, phy_overbounce_scale);
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
  q3a_VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, phy_overbounce_scale);
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


