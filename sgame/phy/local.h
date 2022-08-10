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
#ifndef   PHY_LOCAL_H
#define   PHY_LOCAL_H

#include "../bg_pmove.h"
#include "../bg_local.h"

// Flow Macros. For quickly selecting differently named functions (if OSDF: thing; else if IOQ3: otherThing; ... etc)
#define   IOQ3 0 // Select Default named functions (PM_ etc) come from IoQuake3 gamecode
#define   Q3A  1 // Select q3a named functions (q3a_ etc). (Includes cpm mod). They also revert IoQuake3 gamecode changes back to q3a-1.32
#define   OSDF 2 // Select Modded name functions (osdf_ etc). They include the new physics in them.

// Physics variables
extern qboolean phy_initialized;
// TODO: Change to cvars
//  General
extern float    phy_stopspeed;
// Acceleration
// extern float    phy_ground_accel;
// extern float    phy_air_accel;
extern float    phy_fly_accel;
// Friction
extern float    phy_friction;
extern float    phy_fly_friction;
extern float    phy_spectator_friction;

// New variables
extern float    phy_crouch_scale;         // Default renamed
extern int      phy_crouch_feetraise;     // Amount to raise feet when crouching (default MINS_Z is -24, this will + to that)
// extern int   phy_movetype;                // pm->movetype; // Physics type selection
// Skim
extern qboolean phy_skim_enable;          // Enable/disable skimming
// Slick
extern float    phy_slick_accel;          // Acceleration to apply during slick, on modes where this is separate (VQ2/VQ4/CQ3).
extern float    phy_slick_accelscalar;    // Scales incoming ps->speed from ClientThink_real by this number, to calculate real slick accel. Haste affects slick
// Ground
extern float    phy_ground_basespeed;     // Movement speed on the ground (aka maxspeed). Equivalent to the default g_speed
extern float    phy_ground_accel;         // Acceleration when on the ground. sv_accelerate
// Air
extern float    phy_air_basespeed;        // Maxspeed on air when in strafe mode VQ3, or when strafing diagonally in modes CPM and DIFF
extern float    phy_air_accel;            // Acceleration when in strafe mode VQ3, or when strafing diagonally in modes CPM and DIFF
// Air deceleration. To have different accel values for stopping down than for accelerating normally.
extern float    phy_air_decel;            // Factor to scale down air acceleration, when the current angle is over decelAngle
extern float    phy_air_decelAngle;       // Angle at which air deceleration will change
extern float    phy_air_speedscalar;      // Q1-ag movement. Scales incoming ps->speed from ClientThink_real by this number, to calculate real air speed.
// AirStrafe (aka AD turning)
extern float    phy_airstrafe_accel;      // Acceleration in strafe mode VQ1, or when strafing "quakeworld style" in modes CPM and DIFF
extern float    phy_airstrafe_basespeed;  // Maxspeed on air when in strafe mode VQ3, or when strafing diagonally in modes CPM and DIFF
// AirControl (aka W turning)
extern qboolean phy_aircontrol;           // Turns aircontrol on or off 
extern float    phy_aircontrol_amount;    // Amount you can control yourself with W/S
extern float    phy_aircontrol_power;     // Aircontrol formula exponent
extern float    phy_fw_accelerate;        // How much (ups) to accelerate when holding forwards(W) only
// Stepup
extern int      phy_step_size;            // Distance that will be moved up/down for step behavior. (default = STEPSIZE = 18)
extern int      phy_step_maxvel;          // When set, it limits the maximum vertical speed at which you can multi/double jump. Prevents stairs-climb crazyness
// Jump
extern qboolean phy_jump_auto;            // Enable/disable autojump
extern int      phy_jump_type;            // Jump type selection. Available VQ3, CPM    TODO: DEV
extern int      phy_jump_velocity;        // Vertical velocity that will be set/added when jumping (default = JUMP_VELOCITY = 270)
extern float    phy_jump_scalar;          // Factor to scale jump velocity by when pressing "+speed"
extern int      phy_jump_timebuffer;         // Amount of time(ms) since last jump, where CPM dj behavior can happen. (default CPM = 400)
extern int      phy_jump_dj_velocity;     // Amount of velocity to add to CPM dj behavior. (default CPM = 100)
extern qboolean phy_jump_holdboost;       // Enable/disable holdboost mechanic. Gives more height the longer jump key is held.
extern int      phy_jump_hb_amount;       // Amount of ups to add on each frame that jump key is held
// Powerups
//extern float phy_haste_factor;           // Multiplier to apply during haste powerup (q3 default = 1.3)
//extern float phy_quad_factor;            // Multiplier to apply during quad powerup  (q3 default = 3)
// Water
extern float    phy_water_accel;
extern float    phy_water_scale;          // phy_swimScale;
extern float    phy_water_friction;
// q3 math quirks
extern qboolean phy_snapvelocity;         // Enables rounding velocity to ints every frame (default q3a = qtrue)
extern qboolean phy_input_scalefix;       // Fixes slowdown on jump input if enabled (default q3a = qfalse)
// extern float    phy_overbounce_scale;     // OVERCLIP value. Removed to 1.000f for QW
extern float    overbounce_scale;      // OVERCLIP value. Removed to 1.000f for QW
// Crouchslide
extern qboolean phy_crouchlide;           // Enable/disable crouchslide
extern float    phy_crouchslide_friction; // Friction that will be applied during crouchslide
extern int      phy_crouchslide_accel;    // Acceleration that will be used for crouchsliding
extern int      phy_crouchslide_timemax;  // Maximum crouchsliding time cap
extern int      phy_crouchslide_framecount; // Amount of frames of crouchsliding earned on each air frame
// Rampslide
extern qboolean phy_rampslide;            // Enable/disable rampslide
extern int      phy_rampslide_type;       // 1: VQ1, 2:VQ2
extern int      phy_rampslide_speedmin;   // Minimum vertical speed at which rampslides can happen


// Physics indexes
#define   CPM  0 // CPM
#define   VQ1  1 // Q1/QW/AG
#define   VQ3  3 // Unmodded q3a
#define   VQ4  4 // Q4 inspired
#define   VJK  5 // Star Knight Physics
#define   CQ3  6 // VQ3 with CPM doublejumps and rampjumps
//TODO
#define   VQ2  2 // Q2 inspired
#define   DIF  7 // New physics

// Initialize
void     phy_init          (int movetype);    // Calls all other initializer functions
void     cpm_init          (void);
void     vq1_init          (void);
void     vq3_init          (void);
void     vq4_init          (void);
void     vjk_init          (void);
void     cq3_init          (void);

// Movement
void     phy_PmoveSingle   (pmove_t* pmove);  // Core movement entrypoint
void     phy_move          (pmove_t* pmove);  // Calls all other movement functions
void     cpm_move          (pmove_t* pmove);
void     vq1_move          (pmove_t* pmove);
void     vq3_move          (pmove_t* pmove);
void     vq4_move          (pmove_t* pmove);
void     vjk_move          (pmove_t* pmove);
void     cq3_move          (pmove_t* pmove);

// Core functions (common to all/most)
void     core_Accelerate   (vec3_t wishdir, float wishspeed, float accel, float basespeed);
void     core_Friction     (void);
void     core_Weapon       (void);
void     core_GroundTrace  (void);
float    core_CmdScale     (usercmd_t* cmd, qboolean fix);
qboolean core_SlideMove    (qboolean gravity);
void     core_StepSlideMove(qboolean gravity);

// Math
void VectorReflect  (vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void VectorReflect2D(vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void VectorReflectOS(vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void VectorReflectBC(vec3_t in, vec3_t normal, vec3_t out, float overbounce);


#endif // PHY_LOCAL_H
