#include "osdf.h"

void cpm_init(){
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
}

void vq3_init(){
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



