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
static vmCvar_t accel_rgba_none;
static vmCvar_t accel_rgba_partial;
static vmCvar_t accel_rgba_full;
static vmCvar_t accel_rgba_turn;

static cvarTable_t accel_cvartable[] = {
  { &accel,              "hud_accel",              "0b1",                 CVAR_ARCHIVE_ND },
  { &accel_trueness,     "hud_accel_trueness",     "0b110",               CVAR_ARCHIVE_ND },
  { &accel_min_speed,    "hud_accel_min_speed",    "1",                   CVAR_ARCHIVE_ND },
  { &accel_yh,           "hud_accel_yh",           "0.5 12",              CVAR_ARCHIVE_ND },
  { &accel_rgba_none,    "hud_accel_rgba_none",    "0.25 0.25 0.25 0.00", CVAR_ARCHIVE_ND },
  { &accel_rgba_partial, "hud_accel_rgba_partial", "1 0.5 0 0.9",         CVAR_ARCHIVE_ND },
  { &accel_rgba_full,    "hud_accel_rgba_full",    "1 0.5 0 0.7",         CVAR_ARCHIVE_ND },
  { &accel_rgba_turn,    "hud_accel_rgba_turn",    "0 1 1 0.5",           CVAR_ARCHIVE_ND },
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
  { accel_cvartable + 3, Y | H, {"hud_accel_yh X Y",}, },
  { accel_cvartable + 4,  RGBA, {"hud_accel_rgba_none R G B A",}, },
  { accel_cvartable + 5,  RGBA, {"hud_accel_rgba_partial R G B A",}, },
  { accel_cvartable + 6,  RGBA, {"hud_accel_rgba_full R G B A",}, },
  { accel_cvartable + 7,  RGBA, {"hud_accel_rgba_turn R G B A",}, },
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
static float       s_frametime = 0.008f;  // Cached from last valid prediction step
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

static void hud_accel_getData(void);
static void CG_DrawAccel(void);
static void dir_update(float, float, float);
void hud_accel_draw(void) {
  if (!accel.integer) {return;}
  s.ps  = (cg_synchronousClients.integer) ? &cg.snap->ps : &cg.predictedPlayerState;
  s.pmd = &cg.pmd;
  s.pml = &pml;
  if (VectorLengthSquared2D(s.ps->velocity) < accel_min_speed.value * accel_min_speed.value) {return;}
  hud_accel_getData();
  CG_DrawAccel();
}
//:::::::::::::::::
// hud_accel_getData
//   Computes acceleration zone data from native game state.
//   Unlike proxymod, we don't simulate pmove — we read directly from
//   the live pml/ps/pmd that the real prediction already computed.
//:::::::::::::::::
static void hud_accel_getData(void) {
  // Skip accel drawing cases — keep last valid state
  if (s.ps->powerups[PW_FLIGHT])           {return;}
  if (s.ps->pm_flags & PMF_GRAPPLE_PULL)   {return;}
  if (s.ps->pm_flags & PMF_TIME_WATERJUMP) {return;}
  // TODO: waterlevel > 1 check (needs waterlevel exposed from pmove)

  // Cache frametime from prediction when valid
  if (pml.frametime > 0) {
    s_frametime = pml.frametime;
  }

  // Get user input — use local copies so we don't mutate the real pmd
  signed char forwardmove = s.pmd->cmd.forwardmove;
  signed char rightmove   = s.pmd->cmd.rightmove;

  // Use default key combination when no user input
  if (!forwardmove && !rightmove) {
    int8_t const scale = s.pmd->cmd.buttons & BUTTON_WALKING ? 64 : 127;
    forwardmove = scale;
  }

  // Flatten forward and right vectors to horizontal plane
  vec3_t forward, right;
  VectorCopy(s.pml->forward, forward);
  VectorCopy(s.pml->right, right);
  forward[2] = 0;
  right[2] = 0;
  VectorNormalize(forward);
  VectorNormalize(right);

  // Compute wish velocity on the horizontal plane
  for (int axis = 0; axis < 2; axis++) {
    s.wishvel[axis] = forwardmove * forward[axis] + rightmove * right[axis];
  }

  // Use wishspeed from the real pmove prediction
  float wishspeed = s.pmd->wishspeed;
  if (wishspeed <= 0) {return;}

  // Determine acceleration factor and gravity based on movement state
  float accelerate;
  float gravity = 0;
  if (s.pml->walking) {
    accelerate = pm_accelerate;
    if (s.pml->groundTrace.surfaceFlags & SURF_SLICK || s.ps->pm_flags & PMF_TIME_KNOCKBACK) {
      gravity = s.ps->gravity * s_frametime;
    }
  } else {
    accelerate = pm_airaccelerate;
  }

  dir_update(wishspeed, accelerate, gravity);
}

//:::::::::::::::::
// CG_DrawAccel
//   Draw accel hud, based on the data stored in accel.state
//:::::::::::::::::
static void CG_DrawAccel(void) {
  VectorParse(accel_yh.string,           s.graph_yh,               2);
  VectorParse(accel_rgba_none.string,    s.graph_rgbaNoAccel,      4);
  VectorParse(accel_rgba_partial.string, s.graph_rgbaPartialAccel, 4);
  VectorParse(accel_rgba_full.string,    s.graph_rgbaFullAccel,    4);
  VectorParse(accel_rgba_turn.string,    s.graph_rgbaTurnZone,     4);

  // y is [0-1] proportional, h is pixel-exact
  float const yaw   = atan2f(s.wishvel[1], s.wishvel[0]) - s.d_vel;
  float const drawY = s.graph_yh[0] * GL_H;
  float const drawH = s.graph_yh[1];

  CG_FillAngleYaw(-s.slowDir, +s.slowDir, yaw, drawY, drawH, s.graph_rgbaNoAccel);
  CG_FillAngleYaw(+s.slowDir, +s.fastDir, yaw, drawY, drawH, s.graph_rgbaPartialAccel);
  CG_FillAngleYaw(-s.fastDir, -s.slowDir, yaw, drawY, drawH, s.graph_rgbaPartialAccel);
  CG_FillAngleYaw(+s.fastDir, +s.stopDir, yaw, drawY, drawH, s.graph_rgbaFullAccel);
  CG_FillAngleYaw(-s.stopDir, -s.fastDir, yaw, drawY, drawH, s.graph_rgbaFullAccel);
  CG_FillAngleYaw(+s.stopDir, +s.turnDir, yaw, drawY, drawH, s.graph_rgbaTurnZone);
  CG_FillAngleYaw(-s.turnDir, -s.stopDir, yaw, drawY, drawH, s.graph_rgbaTurnZone);
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
  state.vf_squared = VectorLengthSquared2D(s.pml->friction_velocity);
  state.wishspeed  = wishspeed;
  state.a          = accel * state.wishspeed * s_frametime;
  state.a_squared  = state.a * state.a;
  if (!(accel_trueness.integer & ACCEL_GROUND) 
      || state.v_squared - state.vf_squared >= 2 * state.a * state.wishspeed - state.a_squared) {
    state.v_squared = state.vf_squared;
  }
  state.v  = sqrtf(state.v_squared);
  state.vf = sqrtf(state.vf_squared);

  ASSERT_LE(state.a * s_frametime, 1);

  s.slowDir = slowDir_update(&state);
  s.fastDir = fastDir_update(&state);
  s.stopDir = stopDir_update(&state, s.fastDir);
  s.turnDir = turnDir_update(&state, s.stopDir);

  ASSERT_LE(s.slowDir, s.fastDir);
  ASSERT_LE(s.fastDir, s.stopDir);
  ASSERT_LE(s.stopDir, s.turnDir);

  s.d_vel = atan2f(s.pml->friction_velocity[1], s.pml->friction_velocity[0]);
}
