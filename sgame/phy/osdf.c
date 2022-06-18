//
// all phy/* code is part of bg_pmove  (both games player movement code)
// takes playerstate and usercmd as input, and returns a modifed playerstate
#include "osdf.h"
#include "../../qcommon/q_shared.h"
#include "../bg_public.h"

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
float phy_overbounce_scale;     // OVERCLIP value. Removed to 1.000f for QW
// Slidemove 
int   phy_slidemove_type;       // Slidemove function type. For controlling general movement behavior (not slick_type)
// Sound
int   s_jump_interval;          // Dictates how often (ms) the jump sound can be played. Used to stop it from playing every tick/frame

void osdf_init(int movetype) {
  // Initialize physics variables.
  // TODO: Change to Cvars. Current is a TEMP solution until proper physics cvars support.
  if      (movetype == VQ3) { vq3_init() }
  else if (movetype == CPM) { cpm_init() } 
  else if (movetype == VQ1) { vq1_init() } 
  else if (movetype == VJK) { vjk_init() }
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

