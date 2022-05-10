/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Opensource-Defrag source code.

Opensource-Defrag source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Opensource-Defrag source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Opensource-Defrag source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// This file contains code from:
//    github.com/Jelvan1/cgame_proxymod
// methods are renamed and adapted for native integration within Opensource-Defrag

#include "cg_strafehud.h"

#include "bg_pmove.h"  //TODO: Why




#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "help.h"
#include "q_assert.h"

static vmCvar_t cgaz;
static vmCvar_t cgaz_trueness;
static vmCvar_t cgaz_min_speed;
static vmCvar_t cgaz_yh;
static vmCvar_t cgaz_rgbaNoAccel;
static vmCvar_t cgaz_rgbaPartialAccel;
static vmCvar_t cgaz_rgbaFullAccel;
static vmCvar_t cgaz_rgbaTurnZone;

static cvarTable_t cgaz_cvars[] = {
  { &cgaz, "mdd_cgaz", "0b1", CVAR_ARCHIVE_ND },
  { &cgaz_trueness, "mdd_cgaz_trueness", "0b110", CVAR_ARCHIVE_ND },
  { &cgaz_min_speed, "mdd_cgaz_min_speed", "1", CVAR_ARCHIVE_ND },
  { &cgaz_yh, "mdd_cgaz_yh", "180 8", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaNoAccel, "mdd_cgaz_rgbaNoAccel", ".25 .25 .25 .5", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaPartialAccel, "mdd_cgaz_rgbaPartialAccel", "0 1 0 .5", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaFullAccel, "mdd_cgaz_rgbaFullAccel", "0 .25 .25 .5", CVAR_ARCHIVE_ND },
  { &cgaz_rgbaTurnZone, "mdd_cgaz_rgbaTurnZone", "1 1 0 .5", CVAR_ARCHIVE_ND },
};

static help_t cgaz_help[] = {
  {
    cgaz_cvars + 0,
    BINARY_LITERAL,
    {
      "mdd_cgaz 0bX",
      "           |",
      "           +- draw hud",
    },
  },
#define CGAZ_DRAW 1
  {
    cgaz_cvars + 1,
    BINARY_LITERAL,
    {
      "mdd_cgaz_trueness 0bXXX",
      "                    |||",
      "                    ||+- show true jump/crouch zones",
      "                    |+-- show true CPM air control zones",
      "                    +--- show true ground zones",
    },
  },
#define CGAZ_JUMPCROUCH 1
#define CGAZ_CPM        2
#define CGAZ_GROUND     4
  {
    cgaz_cvars + 3,
    Y | H,
    {
      "mdd_cgaz_yh X X",
    },
  },
  {
    cgaz_cvars + 4,
    RGBA,
    {
      "mdd_cgaz_rgbaNoAccel X X X X",
    },
  },
  {
    cgaz_cvars + 5,
    RGBA,
    {
      "mdd_cgaz_rgbaPartialAccel X X X X",
    },
  },
  {
    cgaz_cvars + 6,
    RGBA,
    {
      "mdd_cgaz_rgbaFullAccel X X X X",
    },
  },
  {
    cgaz_cvars + 7,
    RGBA,
    {
      "mdd_cgaz_rgbaTurnZone X X X X",
    },
  },
};

void hud_cgaz_init(void) {
  init_cvars(cgaz_cvars, ARRAY_LEN(cgaz_cvars));
  init_help(cgaz_help, ARRAY_LEN(cgaz_help));
}

void hud_cgaz_update(void) {
  update_cvars(cgaz_cvars, ARRAY_LEN(cgaz_cvars));
  cgaz.integer          = cvar_getInteger("mdd_cgaz");
  cgaz_trueness.integer = cvar_getInteger("mdd_cgaz_trueness");
}

typedef struct {
  float g_squared; // 0 when not on slick.
  float v_squared;
  float vf_squared;
  float a_squared;

  float v;
  float vf;
  float a;

  float wishspeed;
} state_t;

typedef struct {
  vec2_t graph_yh;

  vec4_t graph_rgbaNoAccel;
  vec4_t graph_rgbaPartialAccel;
  vec4_t graph_rgbaFullAccel;
  vec4_t graph_rgbaTurnZone;

  float d_min;
  float d_opt;
  float d_max_cos;
  float d_max;

  float d_vel;

  vec2_t wishvel;

  pmove_t       pm;
  playerState_t pm_ps;
  pml_t         pml;
} cgaz_t;

static cgaz_t s;

static void PmoveSingle(void);
static void PM_AirMove(void);
static void PM_WalkMove(void);

static void CG_DrawCGaz(void);

void hud_cgaz_draw(void) {
  if (!cgaz.integer) {return;}

  s.pm_ps = *getPs();
  s.pm.tracemask = s.pm_ps.pm_type == PM_DEAD ? MASK_PLAYERSOLID & ~CONTENTS_BODY : MASK_PLAYERSOLID;
  if (VectorLengthSquared2(s.pm_ps.velocity) >= cgaz_min_speed.value * cgaz_min_speed.value) PmoveSingle();
}

static void PmoveSingle(void) {
  int8_t const scale = s.pm_ps.stats[13] & PSF_USERINPUT_WALK ? 64 : 127;
  if (!cg.demoPlayback && !(s.pm_ps.pm_flags & PMF_FOLLOW)) {
    int32_t const cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd(cmdNum, &s.pm.cmd);
  } else {
    s.pm.cmd.forwardmove = scale * ((s.pm_ps.stats[13] & PSF_USERINPUT_FORWARD) / PSF_USERINPUT_FORWARD -
                                    (s.pm_ps.stats[13] & PSF_USERINPUT_BACKWARD) / PSF_USERINPUT_BACKWARD);
    s.pm.cmd.rightmove   = scale * ((s.pm_ps.stats[13] & PSF_USERINPUT_RIGHT) / PSF_USERINPUT_RIGHT -
                                    (s.pm_ps.stats[13] & PSF_USERINPUT_LEFT) / PSF_USERINPUT_LEFT);
    s.pm.cmd.upmove      = scale * ((s.pm_ps.stats[13] & PSF_USERINPUT_JUMP) / PSF_USERINPUT_JUMP -
                                    (s.pm_ps.stats[13] & PSF_USERINPUT_CROUCH) / PSF_USERINPUT_CROUCH);
  }

  // clear all pmove local vars
  memset(&s.pml, 0, sizeof(s.pml));
  // save old velocity for crashlanding
  VectorCopy(s.pm_ps.velocity, s.pml.previous_velocity);
  // set viewangles
  AngleVectors(s.pm_ps.viewangles, s.pml.forward, s.pml.right, s.pml.up);

  if (s.pm.cmd.upmove < 10) {
    // not holding jump
    s.pm_ps.pm_flags &= ~PMF_JUMP_HELD;
  }

  if (s.pm_ps.pm_type >= PM_DEAD) {
    s.pm.cmd.forwardmove = 0;
    s.pm.cmd.rightmove   = 0;
    s.pm.cmd.upmove      = 0;
  }

  // Use default key combination when no user input
  if (!s.pm.cmd.forwardmove && !s.pm.cmd.rightmove) {
    s.pm.cmd.forwardmove = scale;
  }

  // set mins, maxs, and viewheight
  PM_CheckDuck(&s.pm, &s.pm_ps);

  // set watertype, and waterlevel
  PM_SetWaterLevel(&s.pm, &s.pm_ps);

  // set groundentity
  PM_GroundTrace(&s.pm, &s.pm_ps, &s.pml);

  // if ( s.pm_ps.pm_type == PM_DEAD ) {
  //   PM_DeadMove ();
  // }

  if (s.pm_ps.powerups[PW_FLIGHT]) {
    // // flight powerup doesn't allow jump and has different friction
    // PM_FlyMove();
    return;
  } else if (s.pm_ps.pm_flags & PMF_GRAPPLE_PULL) {
    // PM_GrappleMove();
    // // We can wiggle a bit
    // PM_AirMove();
    return;
  } else if (s.pm_ps.pm_flags & PMF_TIME_WATERJUMP) {
    // PM_WaterJumpMove();
    return;
  } else if (s.pm.waterlevel > 1) {
    // // swimming
    // PM_WaterMove();
    return;
  } else if (s.pml.walking) {
    // walking on ground
    PM_WalkMove();
  } else {
    // airborne
    PM_AirMove();
  }
  CG_DrawCGaz();
}

static void CG_DrawCGaz(void) {
  float const yaw = atan2f(s.wishvel[1], s.wishvel[0]) - s.d_vel;

  ParseVec(cgaz_yh.string, s.graph_yh, 2);
  ParseVec(cgaz_rgbaNoAccel.string, s.graph_rgbaNoAccel, 4);
  ParseVec(cgaz_rgbaPartialAccel.string, s.graph_rgbaPartialAccel, 4);
  ParseVec(cgaz_rgbaFullAccel.string, s.graph_rgbaFullAccel, 4);
  ParseVec(cgaz_rgbaTurnZone.string, s.graph_rgbaTurnZone, 4);

  CG_FillAngleYaw(-s.d_min, +s.d_min, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaNoAccel);

  CG_FillAngleYaw(+s.d_min, +s.d_opt, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);
  CG_FillAngleYaw(-s.d_opt, -s.d_min, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);

  CG_FillAngleYaw(+s.d_opt, +s.d_max_cos, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);
  CG_FillAngleYaw(-s.d_max_cos, -s.d_opt, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);

  CG_FillAngleYaw(+s.d_max_cos, +s.d_max, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);
  CG_FillAngleYaw(-s.d_max, -s.d_max_cos, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);
}

/*
==================
PM_Friction
Handles both ground friction and water friction
==================
*/
// TODO: Write assert to assume 0 <= cT <= 1
static void PM_Friction(void) {
  // ignore slope movement
  float const speed = s.pml.walking ? VectorLength2(s.pm_ps.velocity) : VectorLength(s.pm_ps.velocity);
  if (speed < 1) {
    s.pm_ps.velocity[0] = 0;
    s.pm_ps.velocity[1] = 0; // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  // apply ground friction
  float drop = 0;
  if (s.pm.waterlevel <= 1 && s.pml.walking && !(s.pml.groundTrace.surfaceFlags & SURF_SLICK)
      && !(s.pm_ps.pm_flags & PMF_TIME_KNOCKBACK))
  {
    float const control = speed < pm_stopspeed ? pm_stopspeed : speed;
    drop += control * pm_friction * pm_frametime;
  }

  // apply water friction even if just wading
  if (s.pm.waterlevel) {
    drop += speed * pm_waterfriction * s.pm.waterlevel * pm_frametime;
  }

  // apply flying friction
  if (s.pm_ps.powerups[PW_FLIGHT]) {
    drop += speed * pm_flightfriction * pm_frametime;
  }

  if (s.pm_ps.pm_type == PM_SPECTATOR) {
    drop += speed * pm_spectatorfriction * pm_frametime;
  }

  // scale the velocity
  float newspeed = speed - drop;
  if (newspeed < 0) {newspeed = 0;}
  newspeed /= speed;
  for (uint8_t i = 0; i < 3; ++i) s.pm_ps.velocity[i] *= newspeed;
}

static float update_d_min(state_t const* state) {
  assert(state);
#ifndef NDEBUG
  if (state->a == 0) {
    ASSERT_EQ(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);
  } else {
    ASSERT_LT(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);
  }
#endif
  float const num_squared = state->wishspeed * state->wishspeed - state->v_squared + state->vf_squared + state->g_squared;
  ASSERT_GE(num_squared, 0);
  float const num = sqrtf(num_squared);
  return num >= state->vf ? 0 : acosf(num / state->vf);
}

static float update_d_opt(state_t const* state) {
  assert(state);
  float const num = state->wishspeed - state->a;
  return num >= state->vf ? 0 : acosf(num / state->vf);
}

static float update_d_max_cos(state_t const* state, float d_opt) {
  assert(state);
  float const num       = sqrtf(state->v_squared - state->g_squared) - state->vf;
  float       d_max_cos = num >= state->a ? 0 : acosf(num / state->a);
  if (d_max_cos < d_opt) {
    ASSERT_FLOAT_GE(state->v * state->vf - state->vf_squared, state->a * state->wishspeed - state->a_squared);
    d_max_cos = d_opt;
  }
  return d_max_cos;
}

static float update_d_max(state_t const* state, float d_max_cos) {
  assert(state);
#ifndef NDEBUG
  if (state->a == 0) {
    ASSERT_EQ(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);
  } else {
    ASSERT_LT(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);
  }
#endif
  float const num = state->v_squared - state->vf_squared - state->a_squared - state->g_squared;
  float const den = 2 * state->a * state->vf;
  if (num >= den) {
    return 0;
  } else if (-num >= den) {
    return (float)M_PI;
  }
  float d_max = acosf(num / den);
  if (d_max < d_max_cos) {
    ASSERT_EQ(state->a, 0);
    d_max = d_max_cos;
  }
  return d_max;
}

static void update_d(float wishspeed, float accel, float slickGravity) {
  ASSERT_GE(slickGravity, 0);
  assert(slickGravity == 0 || s.pml.groundTrace.surfaceFlags & SURF_SLICK || s.pm_ps.pm_flags & PMF_TIME_KNOCKBACK);

  state_t state;
  state.g_squared  = slickGravity * slickGravity;
  state.v_squared  = VectorLengthSquared2(s.pml.previous_velocity);
  state.vf_squared = VectorLengthSquared2(s.pm_ps.velocity);
  state.wishspeed  = wishspeed;
  state.a          = accel * state.wishspeed * pm_frametime;
  state.a_squared  = state.a * state.a;
  if (!(cgaz_trueness.integer & CGAZ_GROUND) 
      || state.v_squared - state.vf_squared >= 2 * state.a * state.wishspeed - state.a_squared)
  {
    state.v_squared = state.vf_squared;
  }
  state.v  = sqrtf(state.v_squared);
  state.vf = sqrtf(state.vf_squared);

  ASSERT_LE(state.a * pm_frametime, 1);

  s.d_min     = update_d_min(&state);
  s.d_opt     = update_d_opt(&state);
  s.d_max_cos = update_d_max_cos(&state, s.d_opt);
  s.d_max     = update_d_max(&state, s.d_max_cos);

  ASSERT_LE(s.d_min, s.d_opt);
  ASSERT_LE(s.d_opt, s.d_max_cos);
  ASSERT_LE(s.d_max_cos, s.d_max);

  s.d_vel = atan2f(s.pm_ps.velocity[1], s.pm_ps.velocity[0]);
}

/*
==============
PM_Accelerate
Handles user intended acceleration
==============
*/
static void PM_Accelerate(float wishspeed, float accel) { update_d(wishspeed, accel, 0); }
static void PM_SlickAccelerate(float wishspeed, float accel) { update_d(wishspeed, accel, s.pm_ps.gravity * pm_frametime); }

/*
===================
PM_AirMove
===================
*/
static void PM_AirMove(void) {
  PM_Friction();

  float const scale = cgaz_trueness.integer & CGAZ_JUMPCROUCH ? PM_CmdScale(&s.pm_ps, &s.pm.cmd)
                                                              : PM_AltCmdScale(&s.pm_ps, &s.pm.cmd);

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2]   = 0;
  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  for (uint8_t i = 0; i < 2; ++i) {
    s.wishvel[i] = s.pm.cmd.forwardmove * s.pml.forward[i] + s.pm.cmd.rightmove * s.pml.right[i];
  }

  float const wishspeed = scale * VectorLength2(s.wishvel);

  if (cgaz_trueness.integer & CGAZ_CPM && s.pm_ps.pm_flags & PMF_PROMODE) {
    if (s.pm.cmd.forwardmove == 0 && s.pm.cmd.rightmove != 0) {
      PM_Accelerate(wishspeed > cpm_airwishspeed ? cpm_airwishspeed : wishspeed, cpm_airstrafeaccelerate);
    } else {
      // Air control when s.pm.cmd.forwardmove != 0 && s.pm.cmd.rightmove == 0 only changes direction
      PM_Accelerate(wishspeed, pm_airaccelerate);
      // TODO: clean up
      if (s.d_max > (float)M_PI / 2) {
        float       v_squared  = VectorLengthSquared2(s.pml.previous_velocity);
        float const vf_squared = VectorLengthSquared2(s.pm_ps.velocity);
        float const a          = cpm_airstopaccelerate * wishspeed * pm_frametime;
        if (v_squared - vf_squared >= 2 * a * wishspeed - a * a) v_squared = vf_squared;
        float const vf = sqrtf(vf_squared); {
          float const num = v_squared - vf_squared - a * a;
          float const den = 2 * a * vf;
          if (num >= den) {
            s.d_max = 0;
          } else if (-num >= den) {
            s.d_max = (float)M_PI;
          } else {
            s.d_max = acosf(num / den);
          }
        }
        ASSERT_LE(s.d_max_cos, s.d_max);
      }
    }
  } else {
    PM_Accelerate(wishspeed, pm_airaccelerate);
  }

  // // we may have a ground plane that is very steep, even
  // // though we don't have a groundentity
  // // slide along the steep plane
  // if (s.pml->groundPlane)
  // {
  //   PM_ClipVelocity(vf, s.pml->groundTrace.plane.normal, vf, OVERCLIP);
  // }

#if 0
  // ZOID:  If we are on the grapple, try stair-stepping
  // this allows a player to use the grapple to pull himself
  // over a ledge
  if (s.pm_ps.pm_flags & PMF_GRAPPLE_PULL)
    PM_StepSlideMove(qtrue);
  else
    PM_SlideMove(qtrue);
#endif

  // PM_StepSlideMove(qtrue);
}

/*
===================
PM_WalkMove
===================
*/
static void PM_WalkMove(void) {
  if (s.pm.waterlevel > 2 && DotProduct(s.pml.forward, s.pml.groundTrace.plane.normal) > 0) {
    // // begin swimming
    // PM_WaterMove();
    return;
  }

  if (PM_CheckJump(&s.pm, &s.pm_ps, &s.pml)) {
    // jumped away
    if (s.pm.waterlevel > 1) {
      // PM_WaterMove();
    } else {
      PM_AirMove();
    }
    return;
  }

  PM_Friction();

  float const scale = cgaz_trueness.integer & CGAZ_JUMPCROUCH ? PM_CmdScale(&s.pm_ps, &s.pm.cmd)
                                                              : PM_AltCmdScale(&s.pm_ps, &s.pm.cmd);

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2]   = 0;

  // TODO: only flat ground correct now
  // // project the forward and right directions onto the ground plane
  // PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
  // PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
  //
  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  for (uint8_t i = 0; i < 2; ++i) {
    s.wishvel[i] = s.pm.cmd.forwardmove * s.pml.forward[i] + s.pm.cmd.rightmove * s.pml.right[i];
  }

  float wishspeed = scale * VectorLength2(s.wishvel);

  // clamp the speed lower if ducking
  if (s.pm_ps.pm_flags & PMF_DUCKED && wishspeed > s.pm_ps.speed * pm_duckScale) {
    wishspeed = s.pm_ps.speed * pm_duckScale;
  }

  // clamp the speed lower if wading or walking on the bottom
  if (s.pm.waterlevel) {
    float const waterScale = 1.f - (1.f - pm_swimScale) * s.pm.waterlevel / 3.f;
    if (wishspeed > s.pm_ps.speed * waterScale) {
      wishspeed = s.pm_ps.speed * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if (s.pml.groundTrace.surfaceFlags & SURF_SLICK || s.pm_ps.pm_flags & PMF_TIME_KNOCKBACK) {
    // PM_Accelerate(wishspeed, s.pm_ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.d_min, s.d_opt, s.d_max_cos, s.d_max));
    PM_SlickAccelerate(wishspeed, s.pm_ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.d_min, s.d_opt, s.d_max_cos, s.d_max));
  } else {
    // don't reset the z velocity for slopes
    // s.pm_ps.velocity[2] = 0;
    PM_Accelerate(wishspeed, s.pm_ps.pm_flags & PMF_PROMODE ? cpm_accelerate : pm_accelerate);
  }

  // // don't do anything if standing still
  // if (!s.pm_ps.velocity[0] && !s.pm_ps.velocity[1]) { return; }

  // PM_StepSlideMove(qfalse);
}
