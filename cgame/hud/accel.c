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
    {
      "hud_accel 0bX",
      "           |",
      "           +- draw hud",
    },
  },
#define ACCEL_DRAW 1
  { accel_cvartable + 1, BINARY_LITERAL,
    {
      "hud_accel_trueness 0bXXX",
      "                    |||",
      "                    ||+- show true jump/crouch zones",
      "                    |+-- show true CPM air control zones",
      "                    +--- show true ground zones",
    },
  },
#define ACCEL_JUMPCROUCH 1
#define ACCEL_CPM        2
#define ACCEL_GROUND     4
  { accel_cvartable + 3, Y | H, {"hud_accel_yh X X",}, },
  { accel_cvartable + 4,  RGBA, {"hud_accel_rgbaNoAccel X X X X",}, },
  { accel_cvartable + 5,  RGBA, {"hud_accel_rgbaPartialAccel X X X X",}, },
  { accel_cvartable + 6,  RGBA, {"hud_accel_rgbaFullAccel X X X X",}, },
  { accel_cvartable + 7,  RGBA, {"hud_accel_rgbaTurnZone X X X X",}, },
};

void hud_accel_init(void) {
  cvartable_init(accel_cvartable, ARRAY_LEN(accel_cvartable));
  hud_help_init(accel_help, ARRAY_LEN(accel_help));
}

void hud_accel_update(void) {
  cvartable_update(accel_cvartable, ARRAY_LEN(accel_cvartable));
  accel.integer          = cvar_getInteger("hud_accel");
  accel_trueness.integer = cvar_getInteger("hud_accel_trueness");
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

  float dmin;
  float dopt;
  float dmax_cos;
  float dmax;

  float d_vel;

  vec2_t wishvel;

  pmove_t       pm;
  playerState_t ps;
  pml_t         pml;
} accel_t;

static accel_t s;

static void hud_PmoveSingle(void);
static void hud_AirMove(void);
static void hud_WalkMove(void);

static void CG_DrawAccel(void);

void hud_accel_draw(void) {
  if (!accel.integer) {return;}

  s.ps = *getPs();
  s.pm.tracemask = s.ps.pm_type == PM_DEAD ? MASK_PLAYERSOLID & ~CONTENTS_BODY : MASK_PLAYERSOLID;
  if (VectorLengthSquared2(s.ps.velocity) >= accel_min_speed.value * accel_min_speed.value) {hud_PmoveSingle();}
}

static void hud_PmoveSingle(void) {
  int8_t const scale = s.ps.stats[13] & PSF_USERINPUT_WALK ? 64 : 127;
  if (!cg.demoPlayback && !(s.ps.pm_flags & PMF_FOLLOW)) {
    int32_t const cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd(cmdNum, &s.pm.cmd);
  } else {  // during demo, or Following in Spectator mode
    s.pm.cmd.forwardmove = scale * ((s.ps.stats[13] & PSF_USERINPUT_FORWARD)  / PSF_USERINPUT_FORWARD -
                                    (s.ps.stats[13] & PSF_USERINPUT_BACKWARD) / PSF_USERINPUT_BACKWARD);
    s.pm.cmd.rightmove   = scale * ((s.ps.stats[13] & PSF_USERINPUT_RIGHT)    / PSF_USERINPUT_RIGHT -
                                    (s.ps.stats[13] & PSF_USERINPUT_LEFT)     / PSF_USERINPUT_LEFT);
    s.pm.cmd.upmove      = scale * ((s.ps.stats[13] & PSF_USERINPUT_JUMP)     / PSF_USERINPUT_JUMP -
                                    (s.ps.stats[13] & PSF_USERINPUT_CROUCH)   / PSF_USERINPUT_CROUCH);
  }

  // clear all pmove local vars
  memset(&s.pml, 0, sizeof(s.pml));

  // save old velocity for crashlanding
  VectorCopy(s.ps.velocity, s.pml.previous_velocity);
  AngleVectors(s.ps.viewangles, s.pml.forward, s.pml.right, s.pml.up);

  if (s.pm.cmd.upmove < 10) {s.ps.pm_flags &= ~PMF_JUMP_HELD;} // not holding jump
  if (s.ps.pm_type >= PM_DEAD) {s.pm.cmd.forwardmove = 0; s.pm.cmd.rightmove = 0;  s.pm.cmd.upmove = 0;}

  // Use default key combination when no user input
  if (!s.pm.cmd.forwardmove && !s.pm.cmd.rightmove) {s.pm.cmd.forwardmove = scale;}

  // set mins, maxs, and viewheight
  hud_CheckDuck(&s.pm, &s.ps);
  // set watertype, and waterlevel
  hud_SetWaterLevel(&s.pm, &s.ps);
  // set groundentity
  hud_GroundTrace(&s.pm, &s.ps, &s.pml);

  if      (s.ps.powerups[PW_FLIGHT])           {return;}
  else if (s.ps.pm_flags & PMF_GRAPPLE_PULL)   {return;}
  else if (s.ps.pm_flags & PMF_TIME_WATERJUMP) {return;}
  else if (s.pm.waterlevel > 1)                {return;}
  else if (s.pml.walking)                      {hud_WalkMove();}
  else                                         {hud_AirMove();}

  CG_DrawAccel();
}

static void CG_DrawAccel(void) {
  float const yaw = atan2f(s.wishvel[1], s.wishvel[0]) - s.d_vel;

  VectorParse(accel_yh.string, s.graph_yh, 2);
  VectorParse(accel_rgbaNoAccel.string, s.graph_rgbaNoAccel, 4);
  VectorParse(accel_rgbaPartialAccel.string, s.graph_rgbaPartialAccel, 4);
  VectorParse(accel_rgbaFullAccel.string, s.graph_rgbaFullAccel, 4);
  VectorParse(accel_rgbaTurnZone.string, s.graph_rgbaTurnZone, 4);

  CG_FillAngleYaw(-s.dmin, +s.dmin, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaNoAccel);

  CG_FillAngleYaw(+s.dmin, +s.dopt, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);
  CG_FillAngleYaw(-s.dopt, -s.dmin, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaPartialAccel);

  CG_FillAngleYaw(+s.dopt, +s.dmax_cos, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);
  CG_FillAngleYaw(-s.dmax_cos, -s.dopt, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaFullAccel);

  CG_FillAngleYaw(+s.dmax_cos, +s.dmax, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);
  CG_FillAngleYaw(-s.dmax, -s.dmax_cos, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_rgbaTurnZone);
}

/*
==================
hud_Friction
Handles both ground friction and water friction
==================*/
// TODO: Write assert to assume 0 <= cT <= 1
static void hud_Friction(void) {
  // ignore slope movement
  float const speed = s.pml.walking ? VectorLength2(s.ps.velocity) : VectorLength(s.ps.velocity);
  if (speed < 1) {
    s.ps.velocity[0] = 0;
    s.ps.velocity[1] = 0; // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  // apply ground friction
  float drop = 0;
  if ( s.pm.waterlevel <= 1 && s.pml.walking && !(s.pml.groundTrace.surfaceFlags & SURF_SLICK)
       && !(s.ps.pm_flags & PMF_TIME_KNOCKBACK))
  {
    float const control = speed < pm_stopspeed ? pm_stopspeed : speed;
    drop += control * pm_friction * pm_frametime;
  }
  // apply water friction even if just wading
  if (s.pm.waterlevel) {
    drop += speed * pm_waterfriction * s.pm.waterlevel * pm_frametime;
  }
  // apply flying friction
  if (s.ps.powerups[PW_FLIGHT]) {
    drop += speed * pm_flightfriction * pm_frametime;
  }
  if (s.ps.pm_type == PM_SPECTATOR) {
    drop += speed * pm_spectatorfriction * pm_frametime;
  }

  // scale the velocity
  float newspeed = speed - drop;
  if (newspeed < 0) { newspeed = 0; }
  newspeed /= speed;
  for (uint8_t i = 0; i < 3; ++i) s.ps.velocity[i] *= newspeed;
}

static float dmin_update(state_t const* state) {
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

static float dopt_update(state_t const* state) {
  assert(state);
  float const num = state->wishspeed - state->a;
  return num >= state->vf ? 0 : acosf(num / state->vf);
}

static float dmax_cos_update(state_t const* state, float dopt) {
  assert(state);
  float const num       = sqrtf(state->v_squared - state->g_squared) - state->vf;
  float       dmax_cos = num >= state->a ? 0 : acosf(num / state->a);
  if (dmax_cos < dopt) {
    ASSERT_FLOAT_GE(state->v * state->vf - state->vf_squared, state->a * state->wishspeed - state->a_squared);
    dmax_cos = dopt;
  }
  return dmax_cos;
}

static float dmax_update(state_t const* state, float dmax_cos) {
  assert(state);
#ifndef NDEBUG
  if (state->a == 0) {ASSERT_EQ(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);}
  else               {ASSERT_LT(state->v_squared - state->vf_squared, 2 * state->a * state->wishspeed - state->a_squared);}
#endif

  float const num = state->v_squared - state->vf_squared - state->a_squared - state->g_squared;
  float const den = 2 * state->a * state->vf;
  if (num >= den)       {return 0;}
  else if (-num >= den) {return (float)M_PI;}

  float dmax = acosf(num / den);
  if (dmax < dmax_cos) {
    ASSERT_EQ(state->a, 0);
    dmax = dmax_cos;
  }
  return dmax;
}

static void update_d(float wishspeed, float accel, float slickGravity) {
  ASSERT_GE(slickGravity, 0);
  assert(slickGravity == 0 || s.pml.groundTrace.surfaceFlags & SURF_SLICK || s.ps.pm_flags & PMF_TIME_KNOCKBACK);

  state_t state;
  state.g_squared  = slickGravity * slickGravity;
  state.v_squared  = VectorLengthSquared2(s.pml.previous_velocity);
  state.vf_squared = VectorLengthSquared2(s.ps.velocity);
  state.wishspeed  = wishspeed;
  state.a          = accel * state.wishspeed * pm_frametime;
  state.a_squared  = state.a * state.a;
  if (!(accel_trueness.integer & ACCEL_GROUND) 
      || state.v_squared - state.vf_squared >= 2 * state.a * state.wishspeed - state.a_squared)
  {
    state.v_squared = state.vf_squared;
  }
  state.v  = sqrtf(state.v_squared);
  state.vf = sqrtf(state.vf_squared);

  ASSERT_LE(state.a * pm_frametime, 1);

  s.dmin     = dmin_update(&state);
  s.dopt     = dopt_update(&state);
  s.dmax_cos = dmax_cos_update(&state, s.dopt);
  s.dmax     = dmax_update(&state, s.dmax_cos);

  ASSERT_LE(s.dmin, s.dopt);
  ASSERT_LE(s.dopt, s.dmax_cos);
  ASSERT_LE(s.dmax_cos, s.dmax);

  s.d_vel = atan2f(s.ps.velocity[1], s.ps.velocity[0]);
}

/*
==============
hud_Accelerate
Handles user intended acceleration
==============*/
static void hud_Accelerate(float wishspeed, float accel) {
  update_d(wishspeed, accel, 0);
}
static void hud_SlickAccelerate(float wishspeed, float accel) {
  update_d(wishspeed, accel, s.ps.gravity * pm_frametime);
}

/*
===================
hud_AirMove
===================*/
static void hud_AirMove(void) {
  hud_Friction();

  float const scale = accel_trueness.integer & ACCEL_JUMPCROUCH ? PM_CmdScale(&s.ps, &s.pm.cmd)
                                                                : PM_AltCmdScale(&s.ps, &s.pm.cmd);

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2]   = 0;
  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  for (uint8_t i = 0; i < 2; ++i) {
    s.wishvel[i] = s.pm.cmd.forwardmove * s.pml.forward[i] + s.pm.cmd.rightmove * s.pml.right[i];
  }

  float const wishspeed = scale * VectorLength2(s.wishvel);

  if (accel_trueness.integer & ACCEL_CPM && s.ps.pm_flags & PMF_PROMODE) {
    if (s.pm.cmd.forwardmove == 0 && s.pm.cmd.rightmove != 0) {
      hud_Accelerate(wishspeed > cpm_airwishspeed ? cpm_airwishspeed : wishspeed, cpm_airstrafeaccelerate);
    } else {
      // Air control when s.pm.cmd.forwardmove != 0 && s.pm.cmd.rightmove == 0 only changes direction
      hud_Accelerate(wishspeed, pm_airaccelerate);
      // TODO: clean up
      if (s.dmax > (float)M_PI / 2) {
        float       v_squared  = VectorLengthSquared2(s.pml.previous_velocity);
        float const vf_squared = VectorLengthSquared2(s.ps.velocity);
        float const a          = cpm_airstopaccelerate * wishspeed * pm_frametime;
        if (v_squared - vf_squared >= 2 * a * wishspeed - a * a) v_squared = vf_squared;
        float const vf = sqrtf(vf_squared);
        {
          float const num = v_squared - vf_squared - a * a;
          float const den = 2 * a * vf;
          if (num >= den)       {s.dmax = 0;}
          else if (-num >= den) {s.dmax = (float)M_PI;}
          else                  {s.dmax = acosf(num / den);}
        }
        ASSERT_LE(s.dmax_cos, s.dmax);
      }
    }
  } else {
    hud_Accelerate(wishspeed, pm_airaccelerate);
  }

  // // we may have a ground plane that is very steep, even
  // // though we don't have a groundentity
  // // slide along the steep plane
  // if (s.pml->groundPlane) {
  //   PM_ClipVelocity(vf, s.pml->groundTrace.plane.normal, vf, OVERCLIP);
  // }

#if 0
  // ZOID:  If we are on the grapple, try stair-stepping
  // this allows a player to use the grapple to pull himself
  // over a ledge
  if (s.ps.pm_flags & PMF_GRAPPLE_PULL)
    PM_StepSlideMove(qtrue);
  else
    PM_SlideMove(qtrue);
#endif

  // PM_StepSlideMove(qtrue);
}

/*
===================
hud_WalkMove
===================*/
static void hud_WalkMove(void) {
  if (s.pm.waterlevel > 2 && DotProduct(s.pml.forward, s.pml.groundTrace.plane.normal) > 0) {
    // // begin swimming
    // PM_WaterMove();
    return;
  }

  if (hud_CheckJump(&s.pm, &s.ps, &s.pml)) {
    // jumped away
    if (s.pm.waterlevel > 1) {
      // PM_WaterMove();
    } else {
      hud_AirMove();
    }
    return;
  }

  hud_Friction();

  float const scale = accel_trueness.integer & ACCEL_JUMPCROUCH ? PM_CmdScale(&s.ps, &s.pm.cmd)
                                                              : PM_AltCmdScale(&s.ps, &s.pm.cmd);

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
  if (s.ps.pm_flags & PMF_DUCKED && wishspeed > s.ps.speed * pm_duckScale) {
    wishspeed = s.ps.speed * pm_duckScale;
  }

  // clamp the speed lower if wading or walking on the bottom
  if (s.pm.waterlevel) {
    float const waterScale = 1.f - (1.f - pm_swimScale) * s.pm.waterlevel / 3.f;
    if (wishspeed > s.ps.speed * waterScale) {
      wishspeed = s.ps.speed * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if (s.pml.groundTrace.surfaceFlags & SURF_SLICK || s.ps.pm_flags & PMF_TIME_KNOCKBACK) {
    // hud_Accelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.dmin, s.dopt, s.dmax_cos, s.dmax));
    hud_SlickAccelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.dmin, s.dopt, s.dmax_cos, s.dmax));
  } else {
    // don't reset the z velocity for slopes
    // s.ps.velocity[2] = 0;
    hud_Accelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_accelerate : pm_accelerate);
  }

  // // don't do anything if standing still
  // if (!s.ps.velocity[0] && !s.ps.velocity[1]) {return;}

  // PM_StepSlideMove(qfalse);
}
