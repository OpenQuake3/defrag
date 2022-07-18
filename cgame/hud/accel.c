/*
  ==============================
  Written by:
    id software :            Quake III Arena
    nightmare, hk, Jelvan1 : mdd cgame Proxymod
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
#include "../../sgame/bg_local.h"
#include "../../qcommon/q_assert.h"

//:::::::::::::::::
// Cvars
static vmCvar_t accel;
static vmCvar_t accel_trueness;
static vmCvar_t accel_min_speed;
static vmCvar_t accel_yh;
static vmCvar_t accel_rgbaNoAccel;
static vmCvar_t accel_rgbaPartialAccel;
static vmCvar_t accel_rgbaFullAccel;
static vmCvar_t accel_rgbaTurnZone;

static cvarTable_t accel_cvartable[] = {
  { &accel, "hud_accel", "0b1", CVAR_ARCHIVE_ND },
  { &accel_trueness, "hud_accel_trueness", "0b110", CVAR_ARCHIVE_ND },
  { &accel_min_speed, "hud_accel_min_speed", "1", CVAR_ARCHIVE_ND },
  { &accel_yh, "hud_accel_yh", "180 8", CVAR_ARCHIVE_ND },
  { &accel_rgbaNoAccel, "hud_accel_rgbaNoAccel", ".25 .25 .25 .5", CVAR_ARCHIVE_ND },
  { &accel_rgbaPartialAccel, "hud_accel_rgbaPartialAccel", "0 1 0 .5", CVAR_ARCHIVE_ND },
  { &accel_rgbaFullAccel, "hud_accel_rgbaFullAccel", "0 .25 .25 .5", CVAR_ARCHIVE_ND },
  { &accel_rgbaTurnZone, "hud_accel_rgbaTurnZone", "1 1 0 .5", CVAR_ARCHIVE_ND },
};

static help_t accel_help[] = {
  { accel_cvartable + 0, BINARY_LITERAL, 
    { "hud_accel 0bX",
      "           |",
      "           +- draw hud", }, },
#define ACCEL_DRAW 1
  { accel_cvartable + 1, BINARY_LITERAL,
    { "hud_accel_trueness 0bXXX",
      "                    |||",
      "                    ||+- show true jump/crouch zones",
      "                    |+-- show true CPM air control zones",
      "                    +--- show true ground zones",   }, },
#define ACCEL_JUMPCROUCH 1
#define ACCEL_CPM        2
#define ACCEL_GROUND     4
  { accel_cvartable + 3, Y | H, {"hud_accel_yh X X",}, },
  { accel_cvartable + 4,  RGBA, {"hud_accel_rgbaNoAccel X X X X",}, },
  { accel_cvartable + 5,  RGBA, {"hud_accel_rgbaPartialAccel X X X X",}, },
  { accel_cvartable + 6,  RGBA, {"hud_accel_rgbaFullAccel X X X X",}, },
  { accel_cvartable + 7,  RGBA, {"hud_accel_rgbaTurnZone X X X X",}, },
};
//:::::::::::::::::
// hud_accel_init
//   Initializes everything in the cvar table, with the respective help info
//:::::::::::::::::
void hud_accel_init(void) {
  cvartable_init(accel_cvartable, ARRAY_LEN(accel_cvartable));
  hud_help_init (accel_help,      ARRAY_LEN(accel_help));
}
//:::::::::::::::::
// hud_accel_update
//   Updates the accel cvars data
//:::::::::::::::::
void hud_accel_update(void) {
  cvartable_update(accel_cvartable, ARRAY_LEN(accel_cvartable));
  accel.integer          = cvar_getInteger("hud_accel");
  accel_trueness.integer = cvar_getInteger("hud_accel_trueness");
}
//:::::::::::::::::


//:::::::::::::::::
typedef struct {
  vec2_t         graph_yh;
  vec4_t         graph_rgbaNoAccel;
  vec4_t         graph_rgbaPartialAccel;
  vec4_t         graph_rgbaFullAccel;
  vec4_t         graph_rgbaTurnZone;
  float          slowDir;   // dmin     : Minimum angle to gain any speed
  float          fastDir;   // dopt     : Where you gain more speed forward than in any other dir
  float          stopDir;   // dmax_cos : Absolute end of gaining speed in any direction
  float          turnDir;   // dmax     : Where you gain more speed in the turning direction than forward direction
  float          d_vel;
  vec2_t         wishvel;   // TODO: Should probably just be a pointer instead
  pmoveData_t*   pmd;       // Points to the actual cg.pmoveData_t data. Before, it stored a const copy of recalculated pm
  playerState_t* ps;        // Points to the actual playerState_t data (either predicted or snapshot). Before, it stored a const copy of the data
  pml_t*         pml;       // Points to the actual pml_t data. Before, it stored a const copy of the data
} accel_t;
//:::::::::::::::::
static accel_t     s;  // Stores accel hud state
//:::::::::::::::::


//:::::::::::::::::
// Draw Accel
//:::::::::::::::::
// void old_hud_accel_draw(void) {
//   if (!accel.integer) {return;}
//   s.ps = *getPs();
//   s.pm.tracemask = s.ps.pm_type == PM_DEAD ? MASK_PLAYERSOLID & ~CONTENTS_BODY : MASK_PLAYERSOLID;
//   if (VectorLengthSquared2D(s.ps.velocity) >= accel_min_speed.value * accel_min_speed.value) {hud_PmoveSingle();}
// }

static void hud_accel_getData(void);   // TODO: Remove
static void CG_DrawAccel(void);        // TODO: Remove
void hud_accel_draw(void) {
  // Exit if not active
  if (!accel.integer) {return;}
  // Get playerState_t, pmove_t and pml_t
  s.ps = (cg_synchronousClients.integer) ? &cg.snap->ps : &cg.predictedPlayerState;
  s.pmd = &cg.pmd;
  s.pml = &pml;
  // if vel.lenSquared2D >= min_speed.squared
  if (VectorLengthSquared2D(s.ps->velocity) >= powf(accel_min_speed.value,2)) {
    // Get data and Draw
    hud_accel_getData();
    CG_DrawAccel();
  }
}
static void dir_update(float, float, float); // TODO: Remove
static void hud_accel_getData(void){
  // Set Accel state
  //::::::::::::::::::::::::::::::::::::
  // hud_PmoveSingle();
  // Set scale, based on walking or not
  int8_t const scale = s.pmd->cmd.buttons & BUTTON_WALKING ? 64 : 127;
  // If not demo or not follow, get cmd
  // Else:  gets cmd from stats[13] instead
  //:
  //::::::::::::::::::::::::::::::::::::
  // Does standard pmovesingle
  //   Gets ps pm pml data
  //   We already have valid data, so use pointers instead
  //::::::::::::::::::::::::::::::::::::
  // clear all pmove local vars
  // save old velocity for crashlanding
  // update viewangles
  // Jump held or dead
  // Use default key combination when no user input
  if (!s.pmd->cmd.forwardmove && !s.pmd->cmd.rightmove) {s.pmd->cmd.forwardmove = scale;}
  //::::::::::::::::::::::::::::::::::::
  // set mins, maxs, and viewheight
  // PM_CheckDuck(&s.pm, &s.ps);  // Copy/Paste of the code
  //::::::::::::::::::::::::::::::::::::
  // set watertype, and waterlevel
  // hud_SetWaterLevel(&s.pm, &s.ps); // Skips all that's not updating data
  //::::::::::::::::::::::::::::::::::::
  // set groundentity
  // hud_GroundTrace(&s.pm, &s.ps, &s.pml); // Skips all that's not updating data
  //::::::::::::::::::::::::::::::::::::
  // Skip accel drawing cases
  //   s.ps.powerups[PW_FLIGHT]
  //   s.ps.pm_flags & PMF_GRAPPLE_PULL
  //   s.ps.pm_flags & PMF_TIME_WATERJUMP
  //   s.pm.waterlevel > 1
  //::::::::::::::::::::::::::::::::::::
  // else if (s.pml->walking) {hud_WalkMove();}
  // dir_update( with walkmove pm )
  //::::::::::::::::::::::::::::::::::::
  // else                     {hud_AirMove();}
  // Does Normal airmove, except for trueness
  // Selects AD, W or Diagonal. Gets correct values and does trueness
  // Else:
  dir_update(s.pmd->wishspeed, s.pmd->accel, 0);  // TODO: Slick

  //::::::::::::::::::::::::::::::::::::
}

//:::::::::::::::::
// CG_DrawAccel
//   Draw accel hud, based on the data stored in accel.state
//:::::::::::::::::
static void CG_DrawAccel(void) {
  float const yaw = atan2f(s.wishvel[1], s.wishvel[0]) - s.d_vel;

  VectorParse(accel_yh.string,               s.graph_yh,               2);
  VectorParse(accel_rgbaNoAccel.string,      s.graph_rgbaNoAccel,      4);
  VectorParse(accel_rgbaPartialAccel.string, s.graph_rgbaPartialAccel, 4);
  VectorParse(accel_rgbaFullAccel.string,    s.graph_rgbaFullAccel,    4);
  VectorParse(accel_rgbaTurnZone.string,     s.graph_rgbaTurnZone,     4);

  CG_FillAngleYaw(-s.slowDir, +s.slowDir, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaNoAccel);
  CG_FillAngleYaw(+s.slowDir, +s.fastDir, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);
  CG_FillAngleYaw(-s.fastDir, -s.slowDir, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);
  CG_FillAngleYaw(+s.fastDir, +s.stopDir, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);
  CG_FillAngleYaw(-s.stopDir, -s.fastDir, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);
  CG_FillAngleYaw(+s.stopDir, +s.turnDir, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);
  CG_FillAngleYaw(-s.turnDir, -s.stopDir, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);
}
//::::::::::::::::::
// slowDir  = dmin     : Minimum angle to gain any speed
static float slowDir_update(pmoveData_t const* state) {
  assert(state);
  #ifndef NDEBUG
  if (state->a == 0) { ASSERT_EQ(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared); } 
  else               { ASSERT_LT(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared); }
  #endif
  float const num_squared = state->wishspeed * state->wishspeed - state->v_squared + state->vf_squared + state->g_squared;
  ASSERT_GE(num_squared, 0);
  float const num = sqrtf(num_squared);
  return num >= state->vf ? 0 : acosf(num / state->vf);
}
//::::::::::::::::::
// fastDir  = dopt     : Where you gain more speed forward than in any other dir
static float fastDir_update(pmoveData_t const* state) {
  assert(state);
  float const num = state->wishspeed - state->a;
  return num >= state->vf ? 0 : acosf(num / state->vf);
}
//::::::::::::::::::
// turnDir  = dmax     : Where you gain more speed in the turning direction than forward direction
static float turnDir_update(pmoveData_t const* state, float stopDir) {
  assert(state);
  #ifndef NDEBUG
  if (state->a == 0) {ASSERT_EQ(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);}
  else               {ASSERT_LT(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);}
  #endif

  float const num = state->v_squared - state->vf_squared - state->a_squared - state->g_squared;
  float const den = 2 * state->a * state->vf;
  if      ( num >= den) {return 0;}
  else if (-num >= den) {return (float)M_PI;}

  float turnDir = acosf(num / den);
  if (turnDir < stopDir) {
    ASSERT_EQ(state->a, 0);
    turnDir = stopDir;
  }
  return turnDir;
}
//::::::::::::::::::
// stopDir  = dmax_cos : Absolute end of gaining speed in any direction
static float stopDir_update(pmoveData_t const* state, float fastDir) {
  assert(state);
  float const num     = sqrtf(state->v_squared - state->g_squared) - state->vf;
  float       stopDir = num >= state->a ? 0 : acosf(num / state->a);
  if (stopDir < fastDir) {
    ASSERT_FLOAT_GE(state->v * state->vf - state->vf_squared, state->a * state->wishspeed - state->a_squared);
    stopDir = fastDir;
  }
  return stopDir;
}

//::::::::::::::::::
// Update Direction
// slowDir  = dmin     : Minimum angle to gain any speed
// fastDir  = dopt     : Where you gain more speed forward than in any other dir
// turnDir  = dmax     : Where you gain more speed in the turning direction than forward direction
// stopDir  = dmax_cos : Absolute end of gaining speed in any direction
//::::::::::::::::::
static void dir_update(float wishspeed, float accel, float slickGravity) {
  // check that slickgravity is positive or 0, or we have slick or knockback
  ASSERT_GE(slickGravity, 0);
  assert(slickGravity == 0 || s.pml->groundTrace.surfaceFlags & SURF_SLICK || s.ps->pm_flags & PMF_TIME_KNOCKBACK);
  // Update state
  pmoveData_t state;
  state.g_squared  = slickGravity * slickGravity;
  state.v_squared  = VectorLengthSquared2D(s.pml->previous_velocity);
  state.vf_squared = VectorLengthSquared2D(s.ps->velocity);
  state.wishspeed  = wishspeed;
  state.a          = accel * state.wishspeed * pml.frametime;
  state.a_squared  = state.a * state.a;
  if (!(accel_trueness.integer & ACCEL_GROUND) 
      || state.v_squared - state.vf_squared >= 2 * state.a * state.wishspeed - state.a_squared) {
    state.v_squared = state.vf_squared;
  }
  state.v  = sqrtf(state.v_squared);
  state.vf = sqrtf(state.vf_squared);

  ASSERT_LE(state.a * pml.frametime, 1);

  s.slowDir = slowDir_update(&state);
  s.fastDir = fastDir_update(&state);
  s.stopDir = stopDir_update(&state, s.fastDir);
  s.turnDir = turnDir_update(&state, s.stopDir);

  ASSERT_LE(s.slowDir, s.fastDir);
  ASSERT_LE(s.fastDir, s.stopDir);
  ASSERT_LE(s.stopDir, s.turnDir);

  s.d_vel = atan2f(s.ps->velocity[1], s.ps->velocity[0]);
}
