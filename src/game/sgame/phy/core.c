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

//
// all phy/* code is part of bg_pmove  (both games player movement code)
// takes playerstate and usercmd as input, and returns a modifed playerstate
#include "local.h"
#include "../../qcommon/q_shared.h"
#include "../bg_public.h"

// Variables Declaration
float    phy_stopspeed;
// Acceleration
// float    phy_ground_accel;
// float    phy_air_accel;
float    phy_fly_accel;
// Slick
float    phy_slick_accel;
float    phy_slick_accelscalar;
// Friction
float    phy_friction;
float    phy_fly_friction;
float    phy_spectator_friction;
// New variables
float    phy_crouch_scale;
int      phy_crouch_feetraise;
//int   phy_movetype;
qboolean phy_skim_enable;
int      phy_step_size;
// Ground
float    phy_ground_basespeed;
float    phy_ground_accel;
// Air
float    phy_air_basespeed;
float    phy_air_accel;
// Air deceleration.
float    phy_air_decel;
float    phy_air_decelAngle;
float    phy_air_speedscalar;
// AirStrafe (aka AD turning)
float    phy_airstrafe_accel;
float    phy_airstrafe_basespeed;
// AirControl (aka W turning)
qboolean phy_aircontrol;
float    phy_aircontrol_amount;
float    phy_aircontrol_power;
float    phy_fw_accelerate;
// Jump
int      phy_step_maxvel;
qboolean phy_jump_auto;
int      phy_jump_type;
int      phy_jump_velocity;
float    phy_jump_scalar;
int      phy_jump_timebuffer;
int      phy_jump_dj_velocity;
qboolean phy_jump_holdboost;
int      phy_jump_hb_amount;
// Powerups
//extern float phy_haste_factor;
//extern float phy_quad_factor;
// Water
float    phy_water_accel;
float    phy_water_scale;
float    phy_water_friction;
// q3 math quirks
qboolean phy_snapvelocity;
qboolean phy_input_scalefix;
// float    phy_overbounce_scale;
float    overbounce_scale;
// Slidemove 
int      phy_slidemove_type;
// Crouchslide
float    phy_crouchslide_friction;
int      phy_crouchslide_accel;
int      phy_crouchslide_timemax;
int      phy_crouchslide_framecount;
// Rampslide
qboolean phy_rampslide;
int      phy_rampslide_type;
int      phy_rampslide_speedmin;


static void phy_reset(void); // bottom of this file

// Initialize Physics Values
qboolean phy_initialized = qfalse;
void phy_init(int movetype) {
  // Reset all values, in case each init function is missing any
  phy_reset();
  // Initialize physics variables.
  // TODO: Change to Cvars. Current is a TEMP solution until proper physics cvars support.
  switch (movetype){
    case VQ3: vq3_init(); break;
    case CPM: cpm_init(); break;
    case VQ1: vq1_init(); break;
    case VQ4: vq4_init(); break;
    case VJK: vjk_init(); break;
    case CQ3: cq3_init(); break;
    default:  Com_Printf("::ERR physics not initialized:  phy_movetype %i not recognized\n", movetype); break;
  }
  Com_Printf("Initialized: phy_movetype %i\n",movetype);
  phy_initialized  = qtrue;
}

// VectorReflect
// =============
// PM_ClipVelocity = VelocityProject = Vector_RotateAndScale = Vector_ReflectAndScale
//     Wrongly named "clip", due to doom naming inheritance. 
//     It no longer "clips" a vector, but instead reflects / projects it with a scale
//     This is used when we want the player to not simply just go through a surface.
//
// - Takes an incoming vector (in), and reflects it on a surface (normal), scaled by the desired amount (overbounce)
//   This is also used to reflect vectors based on any other arbitrary vector
//
static void VectorReflect_(vec3_t in, vec3_t normal, vec3_t out, float overbounce, int dimensions, qboolean onesided) {
  float backoff, change;
  int i;
  // Calculate direction of rotation / reflection
  backoff = DotProduct(in, normal);
  // Scale the direction
  if (backoff < 0)  { backoff *= overbounce; }
  else              { 
    if (!onesided)  { backoff /= overbounce; }  // Standard behavior
    else            { backoff = 0; }            // Ignore backoff when moving away from the surface   dot(in, normal) = positive
  }
  // Apply scale to the vector
  for (i = 0; i < dimensions; i++) {            // Will ignore Z when dimensions set to 2
    change = normal[i] * backoff;
    out[i] = in[i] - change; // An overbounce value of 1.000 completely negates incoming velocity, due to this line
  }
}
// VectorReflect: Standard Behavior (Unmodded Q3A)
void VectorReflect(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
  VectorReflect_(in, normal, out, overbounce, 3, qfalse);
}
// VectorReflect: Horizontal Only
void VectorReflect2D(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
  VectorReflect_(in, normal, out, overbounce, 2, qfalse);
}
// VectorReflect: One Sided
//   Doesn't affect the vector, when we are moving away from the surface   dot(in, normal) = positive
void VectorReflectOS(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
  VectorReflect_(in, normal, out, overbounce, 3, qtrue);
}
// VectorReflect: Bouncy
//   Increases incoming overbounce value
void VectorReflectBC(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
  VectorReflect_(in, normal, out, overbounce+1, 3, qfalse);
}

// Ground Trace
//   OBfix is applied here
// Rampslide happens in GroundTrace, when checking if the player is on the ground
//   If z velocity is greater than some value, the player is not allowed to be grounded.
// Forward declare
void q3a_GroundTrace(void);
void new_GroundTrace(void);
static void core_CrashLand(void);
// Select type
void core_GroundTrace(void) {
  switch (pm->movetype) {
    case CPM: q3a_GroundTrace(); break;
    case VQ3: q3a_GroundTrace(); break;
    case VJK: q3a_GroundTrace(); break;
    case CQ3: q3a_GroundTrace(); break;
    case VQ1: new_GroundTrace(); break;
    case VQ2: new_GroundTrace(); break;
    case VQ4: new_GroundTrace(); break;
    default: break;
  }
}
// Unmodded Q3A version
void q3a_GroundTrace(void) {
  vec3_t point;
  trace_t trace;

  point[0] = pm->ps->origin[0];
  point[1] = pm->ps->origin[1];
  point[2] = pm->ps->origin[2] - 0.25;

  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
  pml.groundTrace = trace;

  // do something corrective if the trace starts in a solid...
  if (trace.allsolid) { if (!PM_CorrectAllSolid(&trace)) {return;}}
  // if the trace didn't hit anything, we are in free fall
  if (trace.fraction == 1.0) {
    PM_GroundTraceMissed();
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }
  // check if getting thrown off the ground
  if (pm->ps->velocity[2] > 0 && DotProduct(pm->ps->velocity, trace.plane.normal) > 10) {
    if (pm->debugLevel) { Com_Printf("%i:kickoff\n", c_pmove);}
    // go into jump animation
    if (pm->cmd.forwardmove >= 0) { PM_ForceLegsAnim(LEGS_JUMP);  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; }
    else                          { PM_ForceLegsAnim(LEGS_JUMPB); pm->ps->pm_flags |=  PMF_BACKWARDS_JUMP; }
    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }

  // slopes that are too steep will not be considered onground
  if (trace.plane.normal[2] < MIN_WALK_NORMAL) {
    if (pm->debugLevel) { Com_Printf("%i:steep\n", c_pmove); }
    // FIXME: if they can't slide down the slope, let them walk (sharp crevices)
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
    if (pm->debugLevel) { Com_Printf("%i:Land\n", c_pmove); }
    PM_CrashLand();
    // OBfix: Reflect velocity on floor normal when landing
    if (pml.groundTrace.surfaceFlags & SURF_NOOB) {
      VectorReflect(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, overbounce_scale);
    }

    // don't do landing time if we were just going down a slope
    if (pml.previous_velocity[2] < -200) {
      // don't allow another jump for a little while
      pm->ps->pm_flags |= PMF_TIME_LAND;
      pm->ps->pm_time = 250;
    }
  }

  pm->ps->groundEntityNum = trace.entityNum;

  // OBfix: Remove vertical OBs from Flat surfaces
  if (pml.groundTrace.surfaceFlags & SURF_NOOB && trace.plane.normal[2] == 1.0f) {
    pm->ps->velocity[2] = 0;
  }

  PM_AddTouchEnt(trace.entityNum);
}
// New GroundTrace tech. Adds:
//   Rampslide
//   TODO: Skiing (downramp sliding)
void new_GroundTrace(void) {
  vec3_t  point;
  trace_t trace;
  pm->ps->stats[STAT_RAMPSLIDE] = qfalse; // Init rampslide state
  // Trace the player bbox down -0.25
  VectorSet(point, pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2]-0.25);
  pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
  pml.groundTrace = trace;
  // do something corrective if the trace starts in a solid...
  if (trace.allsolid) { if (!PM_CorrectAllSolid(&trace)) {return;}}
  // if the trace didn't hit anything, we are in free fall
  if (trace.fraction == 1.0) {
    PM_GroundTraceMissed();
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }
  // The trace hit something
  // check if getting thrown off the ground
  if (pm->ps->velocity[2] > 0 && DotProduct(pm->ps->velocity, trace.plane.normal) > 10) {
    if (pm->debugLevel) { Com_Printf("%i:kickoff\n", c_pmove);}
    // go into jump animation
    if (pm->cmd.forwardmove >= 0) { PM_ForceLegsAnim(LEGS_JUMP);  pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP; }
    else                          { PM_ForceLegsAnim(LEGS_JUMPB); pm->ps->pm_flags |=  PMF_BACKWARDS_JUMP; }
    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }
  // slopes that are too steep will not be considered onground
  if (trace.plane.normal[2] < MIN_WALK_NORMAL) {
    if (pm->debugLevel) { Com_Printf("%i:steep\n", c_pmove); }
    // FIXME: if they can't slide down the slope, let them walk (sharp crevices)
    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qtrue;
    pml.walking = qfalse;
    return;
  }

  // We hit something, and its not a steep ramp, so we are on the ground
  if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) { // hitting solid ground will end a waterjump
      pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
      pm->ps->pm_time = 0;
  }
  // Set ground state
  pml.groundPlane = qtrue;
  pml.walking = qtrue;
  if (pm->ps->groundEntityNum == ENTITYNUM_NONE) { // If there is no previous frame ground, we just landed
    qboolean rampslideMin  = (pm->ps->velocity[2] >= phy_rampslide_speedmin)  ? qtrue:qfalse;
    qboolean slideableRamp = (trace.plane.normal[2] > MIN_WALK_NORMAL)        ? qtrue:qfalse;
    qboolean notFlatGround = (trace.plane.normal[2] != 1)                     ? qtrue:qfalse;
    qboolean canRampslide  = (rampslideMin && slideableRamp && notFlatGround) ? qtrue:qfalse;
    if (phy_rampslide && canRampslide){  // Set rampsliding if enabled and conditions are met
      pml.walking = qfalse; // Switch it back to midair for rampslides
      pm->ps->stats[STAT_RAMPSLIDE] = qtrue;
      qboolean slick     = (pml.groundTrace.surfaceFlags & SURF_SLICK)  ? qtrue:qfalse;
      qboolean knockback = (pm->ps->pm_flags & PMF_TIME_KNOCKBACK)      ? qtrue:qfalse;
      qboolean slide     = (slick || knockback)                         ? qtrue:qfalse;
      qboolean style1    = (phy_rampslide_type == VQ1 && slide)         ? qtrue:qfalse;
      if (style1){
        if (pm->debugLevel) { Com_Printf("%i:Land.Rampslide->style1\n", c_pmove); }
        float spd = VectorLength(pm->ps->velocity);
        VectorReflect(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, overbounce_scale);
        VectorNormalize(pm->ps->velocity);
        VectorScale(pm->ps->velocity, spd, pm->ps->velocity);
      } else {
        if (pm->debugLevel) { Com_Printf("%i:Land.Rampslide->style2\n", c_pmove); }
        float spd = VectorLength(pm->ps->velocity);
        VectorReflect(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, overbounce_scale);
        VectorNormalize(pm->ps->velocity);
        VectorScale(pm->ps->velocity, spd, pm->ps->velocity);
      }
    } else {
      if (pm->debugLevel) { Com_Printf("%i:Land.Ground\n", c_pmove); }
      // OBfix: Reflect velocity on floor normal when landing. Stops random obs
      if (pml.groundTrace.surfaceFlags & SURF_NOOB) {
        VectorReflect(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, overbounce_scale);
      }
    }
    core_CrashLand();  // Apply fall damage
    // don't do landing time if we were just going down a slope
    if (pml.previous_velocity[2] < -200) {
      // don't allow another jump for a little while
      pm->ps->pm_flags |= PMF_TIME_LAND;
      pm->ps->pm_time = 250;
    }
  }
  if (!pm->ps->stats[STAT_RAMPSLIDE]){
    pm->ps->groundEntityNum = trace.entityNum; // Store collided entity as groundEntity
  }
  // OBfix: Remove vertical OBs from Flat surfaces
  if (pml.groundTrace.surfaceFlags & SURF_NOOB && trace.plane.normal[2] == 1.0f) {
    pm->ps->velocity[2] = 0;
  }
  PM_AddTouchEnt(trace.entityNum); // Add collided entity to the touch list
}

// core_CrashLand
//   Check for hard landings that generate sound events
static void core_CrashLand(void) {
  // decide which landing animation to use
  if (pm->ps->pm_flags & PMF_BACKWARDS_JUMP) { PM_ForceLegsAnim(LEGS_LANDB); }
  else                                       { PM_ForceLegsAnim(LEGS_LAND); }
  pm->ps->legsTimer = TIMER_LAND;
  // calculate the exact velocity on landing
  float dist = pm->ps->origin[2] - pml.previous_origin[2];
  float vel  = pml.previous_velocity[2];
  float acc  = -pm->ps->gravity;

  float a    = acc / 2;
  float b    = vel;
  float c    = -dist;

  float den  = b * b - 4 * a * c;
  if (den < 0) { return; }
  float t = (-b - sqrt(den)) / (2 * a);

  float delta = vel + t * acc;
  delta = delta * delta * 0.0001;

  // ducking while falling doubles damage
  if (pm->ps->pm_flags & PMF_DUCKED) { delta *= 2; }
  // never take falling damage if completely underwater
  if (pm->waterlevel == 3) { return; }
  // reduce falling damage if there is standing water
  if (pm->waterlevel == 2) { delta *= 0.25; }
  if (pm->waterlevel == 1) { delta *= 0.5; }
  // Dont do crashland if no delta
  if (delta < 1) { return; }

  // create a local entity event to play the sound

  // SURF_NODAMAGE is used for bounce pads where you don't ever take damage or play a crunch sound
  if (!(pml.groundTrace.surfaceFlags & SURF_NODAMAGE)) {
    if (delta > 60) {
      PM_AddEvent(EV_FALL_FAR);
    } else if (delta > 40) {
      if (pm->ps->stats[STAT_HEALTH] > 0) { PM_AddEvent(EV_FALL_MEDIUM); } // pain grunt, so don't play it if dead
    } else if (delta > 7) { 
      if (!pm->ps->stats[STAT_RAMPSLIDE]) { PM_AddEvent(EV_FALL_SHORT); } // Don't do fallshort sound during rampsliding
    } else {
      PM_AddEvent(PM_FootstepForSurface());
    }
  }
  // start footstep cycle over
  pm->ps->bobCycle = 0;
}


//:::::::::::::::::::::::::::::::::::::::::
// core_CmdScale
//   Scale factor to apply to inputs (cmd).
//   Modified to (optionally) allow fixing slowdown when holding jump.
//   fix = Input scaling fix (active/inactive). For fixing slowdown on jump-hold
//   .. fix is ignored for VQ3/CPM movetypes.
float core_CmdScale(usercmd_t *cmd, qboolean fix) {
  // Aliases, for readability
  float fmove = (float)abs(cmd->forwardmove);
  float smove = (float)abs(cmd->rightmove);
  float umove = (float)abs(cmd->upmove);

  // Select maximum input value (absolute)
  int max = fmove;
  if (smove > max)         { max = smove; }
  if (umove > max && !fix) { max = umove; } // Ignore umove for scalefix
  if (!max)                { return 0; }

  // Calculate total input value
  float total;
  if (fix) { total = sqrt(fmove * fmove + smove * smove); } // Ignore umove for scalefix
  else     { total = sqrt(fmove * fmove + smove * smove + umove * umove); }

  // Calculate scale value and return it
  return (float)pm->ps->speed * max / (127.0 * total);
}

// Changed from q3a-gpl behavior to include basespeed.
void core_Accelerate(vec3_t wishdir, float wishspeed, float accel, float basespeed) {
  float  addspeed, accelspeed, currentspeed;
  float  wishspeed_c;
  vec3_t accelVelocity;

  // Clamp wishpeed to a maximum of basespeed
  wishspeed_c = wishspeed; // Initialize clamped wishspeed
  if (wishspeed_c > basespeed) { wishspeed_c = basespeed; }
  // Determine veer amount
  currentspeed = DotProduct(pm->ps->velocity, wishdir);
  // See how much to add
  addspeed = wishspeed_c - currentspeed;
  // If not adding any, done.
  if (addspeed <= 0) {return;}
  // Acceleration speed to add after accel
  accelspeed = accel * wishspeed_c * pml.frametime;
  // Cap it
  if (accelspeed > addspeed) { accelspeed = addspeed; }
  // Adjust player velocity
  for (int i = 0; i < 3; i++) {
    accelVelocity[i] = accelspeed * wishdir[i]; // dir*speed = velocity
    pm->ps->velocity[i] += accelVelocity[i];    // Vector addition, the typical visualization explained in videos of strafing math theory
  }
  // Store (internal) pmoveData
  pm->pmd.wishspeed  = wishspeed_c;  // Wishspeed, as calculated in pmove
}

void core_Friction(void) {
  vec3_t vec;
  float* vel;
  float  speed, newspeed, control;
  float  drop;

  vel = pm->ps->velocity;

  VectorCopy(vel, vec);
  if (pml.walking) { vec[2] = 0; } // ignore slope movement

  speed = VectorLength(vec);
  // allow sinking underwater   FIXME: still have z friction underwater?
  if (speed < 1) { vel[0] = 0; vel[1] = 0; return; }
  drop = 0;
  // apply ground friction
  if ( pm->waterlevel <= 1 ) {
    if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
      if ( !(pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {  // if getting knocked back, no friction
        control = speed < phy_stopspeed ? phy_stopspeed : speed;
        drop += control*phy_friction*pml.frametime;
      }
    }
  }
  // apply water friction even if just wading
  if ( pm->waterlevel ) { drop += speed*phy_water_friction*pm->waterlevel*pml.frametime; }
  // apply flying friction
  if ( pm->ps->powerups[PW_FLIGHT])     { drop += speed*phy_fly_friction*pml.frametime; }
  if ( pm->ps->pm_type == PM_SPECTATOR) { drop += speed*phy_spectator_friction*pml.frametime; }

  // scale the velocity
  newspeed = speed - drop;
  if (newspeed < 0) { newspeed = 0; }
  newspeed /= speed;
  VectorScale(vel, newspeed, vel);
}

static void core_FinishWeaponChange( void ) {
  int   weapon;

  weapon = pm->cmd.weapon;
  if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) { weapon = WP_NONE; }
  if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) { weapon = WP_NONE; }

  pm->ps->weapon = weapon;
  pm->ps->weaponstate = WEAPON_RAISING;
  pm->ps->weaponTime += pm->movetype == CPM ? 0 : 250;  // Instant weapon switch for cpm
  PM_StartTorsoAnim( TORSO_RAISE );
}

static void core_BeginWeaponChange( int weapon ) {
  if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) { return; }
  if ( !( pm->ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) { return; }
  if ( pm->ps->weaponstate == WEAPON_DROPPING ) { return; }

  PM_AddEvent( EV_CHANGE_WEAPON );
  pm->ps->weaponstate = WEAPON_DROPPING;
  pm->ps->weaponTime += pm->movetype == CPM ? 0 : 250;
  PM_StartTorsoAnim( TORSO_DROP );
}

void core_Weapon( void ) {
  int   addTime;

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
      core_BeginWeaponChange( pm->cmd.weapon );
    }
  }
  if ( pm->ps->weaponTime > 0 ) { return; }

  // change weapon if time
  if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
    core_FinishWeaponChange();
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
  case WP_GAUNTLET:         addTime =  400; break;
  case WP_LIGHTNING:        addTime =   50; break;
  case WP_SHOTGUN:          addTime = 1000; break;
  case WP_MACHINEGUN:       addTime =  100; break;
  case WP_GRENADE_LAUNCHER: addTime =  800; break;
  case WP_ROCKET_LAUNCHER:  addTime =  800; break;
  case WP_PLASMAGUN:        addTime =  100; break;
  case WP_RAILGUN:          addTime = 1500; break;
  case WP_BFG:              addTime =  200; break;
  case WP_GRAPPLING_HOOK:   addTime =  400; break;

  #ifdef TEAMARENA
  case WP_NAILGUN:          addTime = 1000; break;
  case WP_PROX_LAUNCHER:    addTime =  800; break;
  case WP_CHAINGUN:         addTime =   30; break;
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


//================
// PmoveSingle
//================
void phy_PmoveSingle(pmove_t *pmove) {
  //::::::::::::::
  memset(&pmove->pmd, 0, sizeof(pmove->pmd)); // Zero out (internal) pmoveData before PmoveSingle happens
  //::::::::::::::
  // Initialize
  pm = pmove;
  // this counter lets us debug movement problems with a journal
  // by setting a conditional breakpoint for the previous frame
  c_pmove++;
  // clear results
  pm->numtouch   = 0;
  pm->watertype  = 0;
  pm->waterlevel = 0;

   // corpses can fly through bodies
  if (pm->ps->stats[STAT_HEALTH] <= 0) { pm->tracemask &= ~CONTENTS_BODY; }
  // make sure walking button is clear if running, avoids proxy no-footsteps cheats
  if (abs(pm->cmd.forwardmove) > 64 || abs(pm->cmd.rightmove) > 64) {
    pm->cmd.buttons &= ~BUTTON_WALKING;
  }
  // set the talk balloon flag
  if (pm->cmd.buttons & BUTTON_TALK) { pm->ps->eFlags |=  EF_TALK; }
  else                               { pm->ps->eFlags &= ~EF_TALK; }
  // set the firing flag for continuous beam weapons
  if (!(pm->ps->pm_flags & PMF_RESPAWNED)
      && pm->ps->pm_type != PM_INTERMISSION && pm->ps->pm_type != PM_NOCLIP
      && (pm->cmd.buttons & BUTTON_ATTACK) && pm->ps->ammo[pm->ps->weapon]) {
    pm->ps->eFlags |= EF_FIRING;
  } else {
    pm->ps->eFlags &= ~EF_FIRING;
  }
  // clear the respawned flag if attack and use are cleared
  if (pm->ps->stats[STAT_HEALTH] > 0 && !(pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE))) {
    pm->ps->pm_flags &= ~PMF_RESPAWNED;
  }

  // if talk button is down, disallow all other input. Prevents potential intercept proxies from adding fake talk balloons
  if (pmove->cmd.buttons & BUTTON_TALK) {
    // keep talk button set, for when cmd.serverTime > 66 msec and the same cmd is used multiple times in Pmove
    pmove->cmd.buttons = BUTTON_TALK;
    pmove->cmd.forwardmove = 0;
    pmove->cmd.rightmove = 0;
    pmove->cmd.upmove = 0;
  }
  // clear all pmove local vars
  memset(&pml, 0, sizeof(pml));
  // determine the time
  pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
  if      (pml.msec < 1)   { pml.msec = 1; }
  else if (pml.msec > 200) { pml.msec = 200; }
  pm->ps->commandTime = pmove->cmd.serverTime;
  pml.frametime = pml.msec * 0.001;

  // save old org in case we get stuck
  VectorCopy(pm->ps->origin, pml.previous_origin);
  // save old velocity for crashlanding
  VectorCopy(pm->ps->velocity, pml.previous_velocity);

  // update viewangles
  PM_UpdateViewAngles(pm->ps, &pm->cmd);
  AngleVectors(pm->ps->viewangles, pml.forward, pml.right, pml.up);
  // not holding jump
  if (pm->cmd.upmove < 10) { pm->ps->pm_flags &= ~PMF_JUMP_HELD; }
  // decide if backpedaling animations should be used
  if (pm->cmd.forwardmove < 0) {
    pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
  } else if (pm->cmd.forwardmove > 0 || (pm->cmd.forwardmove == 0 && pm->cmd.rightmove)) {
    pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
  }
  // remove inputs if dead
  if (pm->ps->pm_type >= PM_DEAD) {
    pm->cmd.forwardmove = 0;
    pm->cmd.rightmove = 0;
    pm->cmd.upmove = 0;
  }
  // do spectator move
  if (pm->ps->pm_type == PM_SPECTATOR) {
    PM_CheckDuck();
    PM_FlyMove();
    PM_DropTimers();
    return;
  }
  // do noclip move
  if (pm->ps->pm_type == PM_NOCLIP) {
    PM_NoclipMove();
    PM_DropTimers();
    return;
  }
  // dont do anything else if frozen or intermission
  if (pm->ps->pm_type == PM_FREEZE) { return; }
  if (pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION) { return; }
  // do deadmove
  if (pm->ps->pm_type == PM_DEAD) { PM_DeadMove(); }

  // set watertype, and waterlevel
  PM_SetWaterLevel();
  pml.previous_waterlevel = pmove->waterlevel;

  // do physics movement
  phy_move(pm);
  // Store updated pmoveData
  pm->pmd.movetype   = pm->movetype;
  pm->pmd.cmd        = pm->cmd;
  pm->pmd.tracemask  = pm->tracemask;
  pm->pmd.v          = 0.0;  // norm velocity (starts as prev_velocity)
  pm->pmd.vf         = 0.0;  // norm velocity (friction)
  pm->pmd.a_squared  = 0.0;  // Accel squared
  pm->pmd.v_squared  = 0.0;  // norm velocity squared (starts as previous_velocity)
  pm->pmd.vf_squared = 0.0;  // norm velocity (friction), squared
  pm->pmd.g_squared  = 0.0;  // gravity squared.  0 when not on slick.
  // Finish
  return;
}

// Select the type of movement to execute. Flow control only. 
// Behavior happens inside each function
void phy_move(pmove_t *pmove) {
  if (!phy_initialized) { phy_init(pmove->movetype); }
  switch (pmove->movetype) {
    case CPM: cpm_move(pmove); break;
    case VQ1: vq1_move(pmove); break;
    case VQ3: vq3_move(pmove); break;
    case VQ4: vq4_move(pmove); break;
    case VJK: vjk_move(pmove); break;
    case CQ3: cq3_move(pmove); break;
    default:  Com_Printf("::ERR phy_movetype %i not recognized\n", pmove->movetype); break;
    // case VQ2: vq2_move(pmove); break;
  }
}

//:::::::::::::
// phy_reset
//  Reset all values to vq3 behavior, in case some init function is missing one of them
//:::::::::::::
void phy_reset(void) {
  phy_stopspeed              = pm_stopspeed;
  // Crouch
  phy_crouch_scale           = pm_duckScale;
  phy_crouch_feetraise       = 0;
  // Slick
  phy_slick_accel            = 1;
  phy_slick_accelscalar      = 0;
  // Acceleration
  phy_ground_accel           = pm_accelerate;
  phy_air_accel              = pm_airaccelerate;
  phy_fly_accel              = pm_flyaccelerate;
  // Friction
  phy_friction               = pm_friction;
  phy_fly_friction           = pm_flightfriction;
  phy_spectator_friction     = pm_spectatorfriction;
  // New variables
  phy_skim_enable            = 1;
  // Ground
  phy_ground_basespeed       = 320;
  phy_ground_accel           = 10;
  // Air
  phy_air_basespeed          = 320;
  phy_air_accel              = 1;
  // Air deceleration.
  phy_air_decel              = 0;
  phy_air_decelAngle         = 0;
  phy_air_speedscalar        = 0;
  // AirStrafe (aka AD turning)
  phy_airstrafe_accel        = 0;
  phy_airstrafe_basespeed    = 0;
  // AirControl (aka W turning)
  phy_aircontrol             = qfalse;
  phy_aircontrol_amount      = 0;
  phy_aircontrol_power       = 0;
  phy_fw_accelerate          = 0;
  // Stepup
  phy_step_size              = STEPSIZE;
  phy_step_maxvel            = JUMP_VELOCITY;
  // Jump
  phy_jump_type              = VQ3;
  phy_jump_auto              = qfalse;
  phy_jump_velocity          = JUMP_VELOCITY;
  phy_jump_scalar            = 0;
  phy_jump_timebuffer        = 0;
  phy_jump_dj_velocity       = 0;
  phy_jump_holdboost         = qfalse;
  phy_jump_hb_amount         = 0;
  // Powerups
  // phy_haste_factor        = 0;
  // phy_quad_factor         = 0;
  // Water
  phy_water_accel            = pm_wateraccelerate;
  phy_water_scale            = pm_swimScale;
  phy_water_friction         = pm_waterfriction;
  // q3 math quirks
  phy_snapvelocity           = qtrue;
  phy_input_scalefix         = qfalse;
  overbounce_scale           = OVERCLIP;
  // Slidemove 
  phy_slidemove_type         = 0;
  // Crouchslide
  phy_crouchslide_friction   = 0;
  phy_crouchslide_accel      = 0;
  phy_crouchslide_timemax    = 0;
  phy_crouchslide_framecount = 0;
  // Rampslide
  phy_rampslide              = 0;
  phy_rampslide_type         = 0;
  phy_rampslide_speedmin     = 0;
  Com_Printf("Physics Reset\n");
}
