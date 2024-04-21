#include "cg_jump.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "help.h"

#include <stdlib.h>

static vmCvar_t jump;
static vmCvar_t jump_maxDelay;
static vmCvar_t jump_graph_xywh;
static vmCvar_t jump_graph_rgba;
static vmCvar_t jump_graph_rgbaOnGround;
static vmCvar_t jump_graph_rgbaPreJump;
static vmCvar_t jump_graph_rgbaPostJump;
static vmCvar_t jump_graph_outline_w;
static vmCvar_t jump_graph_outline_rgba;
static vmCvar_t jump_text_xh;
static vmCvar_t jump_text_rgba;

static cvarTable_t jump_cvartable[] = {
  { &jump, "hud_jump", "3", CVAR_ARCHIVE_ND },
  { &jump_maxDelay, "hud_jump_maxDelay", "360", CVAR_ARCHIVE_ND },
  { &jump_graph_xywh, "hud_jump_graph_xywh", "8 8 8 104", CVAR_ARCHIVE_ND },
  { &jump_graph_rgba, "hud_jump_graph_rgba", ".5 .5 .5 .5", CVAR_ARCHIVE_ND },
  { &jump_graph_rgbaOnGround, "hud_jump_graph_rgbaOnGround", "0 1 0 .75", CVAR_ARCHIVE_ND },
  { &jump_graph_rgbaPreJump, "hud_jump_graph_rgbaPreJump", "0 0 1 .75", CVAR_ARCHIVE_ND },
  { &jump_graph_rgbaPostJump, "hud_jump_graph_rgbaPostJump", "1 0 0 .75", CVAR_ARCHIVE_ND },
  { &jump_graph_outline_w, "hud_jump_graph_outline_w", "1", CVAR_ARCHIVE_ND },
  { &jump_graph_outline_rgba, "hud_jump_graph_outline_rgba", "1 1 1 .75", CVAR_ARCHIVE_ND },
  { &jump_text_xh, "hud_jump_text_xh", "6 12", CVAR_ARCHIVE_ND },
  { &jump_text_rgba, "hud_jump_text_rgba", "1 1 1 1", CVAR_ARCHIVE_ND },
};

static help_t jump_help[] = {
  {
    jump_cvartable + 2,
    X | Y | W | H,
    {
      "hud_jump_graph_xywh X X X X",
    },
  },
  {
    jump_cvartable + 3,
    RGBA,
    {
      "hud_jump_graph_rgba X X X X",
    },
  },
  {
    jump_cvartable + 4,
    RGBA,
    {
      "hud_jump_graph_rgbaOnGround X X X X",
    },
  },
  {
    jump_cvartable + 5,
    RGBA,
    {
      "hud_jump_graph_rgbaPreJump X X X X",
    },
  },
  {
    jump_cvartable + 6,
    RGBA,
    {
      "hud_jump_graph_rgbaPostJump X X X X",
    },
  },
  {
    jump_cvartable + 7,
    W,
    {
      "hud_jump_graph_outline_w X",
    },
  },
  {
    jump_cvartable + 8,
    RGBA,
    {
      "hud_jump_graph_outline_rgba X X X X",
    },
  },
  {
    jump_cvartable + 9,
    X | H,
    {
      "hud_jump_text_xh X X",
    },
  },
  {
    jump_cvartable + 10,
    RGBA,
    {
      "hud_jump_text_rgba X X X X",
    },
  },
};

void hud_jump_init(void)
{
  cvartable_init(jump_cvartable, ARRAY_LEN(jump_cvartable));
  hud_help_init(jump_help, ARRAY_LEN(jump_help));
}

void hud_jump_update(void)
{
  cvartable_update(jump_cvartable, ARRAY_LEN(jump_cvartable));
}

typedef enum
{
  AIR_NOJUMP,
  AIR_JUMP,
  GROUND_JUMP,
  GROUND_NOJUMP,
  AIR_JUMPNORELEASE
} state_t;

typedef struct
{
  // timestamps for computation
  uint32_t t_jumpPreGround;
  uint32_t t_groundTouch;

  // state machine
  state_t lastState;

  // draw data
  int32_t postDelay;
  int32_t preDelay;
  int32_t fullDelay;

  vec4_t graph_xywh;
  vec2_t text_xh;

  vec4_t graph_rgba;
  vec4_t graph_rgbaPostJump;
  vec4_t graph_rgbaOnGround;
  vec4_t graph_rgbaPreJump;
  vec4_t graph_outline_rgba;
  vec4_t text_rgba;
} jump_t;

static jump_t jump_;

static void hud_jump_update_state(void)
{
  /*
   * To draw this hud we have to make a little state machine
   *
   * AIR_NOJUMP:        The player is midair, not holding the jump button
   * AIR_JUMP:          The player is midair, holding jump button
   * GROUND_JUMP:       The player is on the ground, holding jump button
   * GROUND_NOJUMP:     The player is on the ground, not holding jump button
   * AIR_JUMPNORELEASE: The player is midair, without releasing the jump button
   */

  uint32_t const             now     = getSnap()->serverTime;
  playerState_t const* const ps      = getPs();
  int8_t const               inAir   = ps->groundEntityNum == ENTITYNUM_NONE;
  int8_t const               jumping = (ps->stats[13] & PSF_USERINPUT_JUMP) / PSF_USERINPUT_JUMP;

  // Determine current state
  state_t state;
  switch (jump_.lastState)
  {
  case AIR_JUMP:
  case AIR_NOJUMP:
    if (inAir)
    {
      state = jumping ? AIR_JUMP : AIR_NOJUMP;
    }
    else
    {
      state = jumping ? GROUND_JUMP : GROUND_NOJUMP;
    }
    break;

  // Edge case at end of cycle
  case GROUND_NOJUMP:
  case GROUND_JUMP:
  case AIR_JUMPNORELEASE:
    if (inAir)
    {
      state = jumping ? AIR_JUMPNORELEASE : AIR_NOJUMP;
    }
    else
    {
      state = jumping ? GROUND_JUMP : GROUND_NOJUMP;
    }
    break;

  default:
    state = GROUND_NOJUMP;
    break;
  }

  // Act on current state
  switch (state)
  {
  case AIR_NOJUMP: // We spend the most time in this state that is why here we show the last jump stats
    if (jump_.lastState == GROUND_NOJUMP)
    {
      jump_.preDelay  = jump_.t_jumpPreGround - now;
      jump_.postDelay = 0;
      jump_.fullDelay = 0;
    }
    else if (jump_.lastState == AIR_JUMP)
    {
      jump_.postDelay = 0;
      jump_.fullDelay = jump_.preDelay;
    }
    else if (jump_.lastState == AIR_JUMPNORELEASE)
    {
      jump_.fullDelay = jump_.postDelay;
      if (jump_.preDelay > 0)
      {
        jump_.fullDelay += jump_.preDelay;
      }
    }
    break;

  case AIR_JUMP:
    if (jump_.lastState == AIR_NOJUMP) jump_.t_jumpPreGround = now;
    jump_.preDelay = now - jump_.t_jumpPreGround; // ms
    break;

  case GROUND_JUMP:
    jump_.t_groundTouch = now;
    break;

  case GROUND_NOJUMP:
    if (jump_.lastState == AIR_JUMP || jump_.lastState == GROUND_JUMP)
    {
      jump_.postDelay = 0;
      jump_.fullDelay = jump_.preDelay;
    }
    else if (jump_.lastState == AIR_NOJUMP)
    {
      jump_.t_jumpPreGround = now; // groundtime
    }
    jump_.preDelay      = jump_.t_jumpPreGround - now;
    jump_.t_groundTouch = now;
    break;

  case AIR_JUMPNORELEASE:
    if (jump_.lastState == GROUND_NOJUMP)
    {
      jump_.preDelay = jump_.t_jumpPreGround - now;
    }
    jump_.postDelay = now - jump_.t_groundTouch; // ms
    break;

  default:
    break;
  }

  if (jump_.preDelay > jump_maxDelay.integer) jump_.preDelay = jump_maxDelay.integer;
  if (jump_.preDelay < -jump_maxDelay.integer) jump_.preDelay = -jump_maxDelay.integer;
  if (jump_.postDelay > jump_maxDelay.integer) jump_.postDelay = jump_maxDelay.integer;

  // if (state != lastState)
  //  g_syscall( CG_PRINT, vaf("%u %u\n", state, lastState));
  jump_.lastState = state;
}

void hud_jump_draw(void)
{
  if (!jump.integer) return;

  hud_jump_update_state();

  VectorParse(jump_text_rgba.string, jump_.text_rgba, 4);
  VectorParse(jump_graph_rgba.string, jump_.graph_rgba, 4);
  VectorParse(jump_graph_rgbaOnGround.string, jump_.graph_rgbaOnGround, 4);
  VectorParse(jump_graph_rgbaPreJump.string, jump_.graph_rgbaPreJump, 4);
  VectorParse(jump_graph_rgbaPostJump.string, jump_.graph_rgbaPostJump, 4);
  VectorParse(jump_graph_outline_rgba.string, jump_.graph_outline_rgba, 4);

  VectorParse(jump_graph_xywh.string, jump_.graph_xywh, 4);
  float const graph_hh = jump_.graph_xywh[3] / 2.f; // half height
  float const graph_m  = jump_.graph_xywh[1] + graph_hh;

  float const upHeight   = ((float)jump_.postDelay / (float)jump_maxDelay.integer) * graph_hh;
  float const downHeight = ((float)abs(jump_.preDelay) / (float)jump_maxDelay.integer) * graph_hh;

  if (jump.integer & 1)
  {
    CG_FillRect(jump_.graph_xywh[0], jump_.graph_xywh[1], jump_.graph_xywh[2], jump_.graph_xywh[3], jump_.graph_rgba);
    CG_FillRect(
      jump_.graph_xywh[0],
      graph_m,
      jump_.graph_xywh[2],
      downHeight,
      jump_.preDelay < 0 ? jump_.graph_rgbaOnGround : jump_.graph_rgbaPreJump);
    CG_FillRect(jump_.graph_xywh[0], graph_m - upHeight, jump_.graph_xywh[2], upHeight, jump_.graph_rgbaPostJump);
    CG_DrawRect(
      jump_.graph_xywh[0],
      jump_.graph_xywh[1],
      jump_.graph_xywh[2],
      jump_.graph_xywh[3],
      jump_graph_outline_w.value,
      jump_.graph_outline_rgba);
  }
  if (jump.integer & 2)
  {
    VectorParse(jump_text_xh.string, jump_.text_xh, 2);
    qboolean const alignRight = jump_.graph_xywh[0] + jump_.graph_xywh[2] / 2.f > cgs.screenWidth / 2;
    CG_DrawText(
      alignRight ? jump_.graph_xywh[0] - jump_.text_xh[0]
                 : jump_.graph_xywh[0] + jump_.graph_xywh[2] + jump_.text_xh[0],
      graph_m - 1.5f * jump_.text_xh[1],
      jump_.text_xh[1],
      vaf("%i", jump_.postDelay),
      jump_.text_rgba,
      alignRight,
      qtrue /*shadow*/);
    CG_DrawText(
      alignRight ? jump_.graph_xywh[0] - jump_.text_xh[0]
                 : jump_.graph_xywh[0] + jump_.graph_xywh[2] + jump_.text_xh[0],
      graph_m - .5f * jump_.text_xh[1],
      jump_.text_xh[1],
      vaf("%i", jump_.fullDelay),
      jump_.text_rgba,
      alignRight,
      qtrue /*shadow*/);
    CG_DrawText(
      alignRight ? jump_.graph_xywh[0] - jump_.text_xh[0]
                 : jump_.graph_xywh[0] + jump_.graph_xywh[2] + jump_.text_xh[0],
      graph_m + .5f * jump_.text_xh[1],
      jump_.text_xh[1],
      vaf("%i", jump_.preDelay),
      jump_.text_rgba,
      alignRight,
      qtrue /*shadow*/);
  }
}
