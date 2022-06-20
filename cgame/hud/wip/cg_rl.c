#include "cg_rl.h"

#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "g_local.h"
#include "help.h"

#define MAX_RL_TIME 15000

static vmCvar_t target_draw;
static vmCvar_t target_shader;
static vmCvar_t target_size;
static vmCvar_t path_draw;
static vmCvar_t path_rgba;

static cvarTable_t rl_cvartable[] = {
  { &target_draw, "hud_rl_target_draw", "0", CVAR_ARCHIVE_ND },
  { &target_shader, "hud_rl_target_shader", "rlTraceMark", CVAR_ARCHIVE_ND },
  { &target_size, "hud_rl_target_size", "24", CVAR_ARCHIVE_ND },
  { &path_draw, "hud_rl_path_draw", "0", CVAR_ARCHIVE_ND },
  { &path_rgba, "hud_rl_path_rgba", "1 0 0 0", CVAR_ARCHIVE_ND },
};

static help_t rl_help[] = {
  {
    rl_cvartable + 4,
    RGBA,
    {
      "hud_rl_path_rgba X X X X",
    },
  },
};

typedef struct
{
  qhandle_t line_shader;
} rl_t;

static rl_t rl_;

void hud_rl_init(void)
{
  cvartable_init(rl_cvartable, ARRAY_LEN(rl_cvartable));
  hud_help_init(rl_help, ARRAY_LEN(rl_help));

  rl_.line_shader = trap_R_RegisterShader("railCore");
}

void hud_rl_update(void)
{
  cvartable_update(rl_cvartable, ARRAY_LEN(rl_cvartable));
}

void draw_rl(void)
{
  if (!target_draw.integer && !path_draw.integer) return;

  refEntity_t beam;
  trace_t     beam_trace;
  vec3_t      origin;
  vec3_t      dest;

  snapshot_t const* const    snap = getSnap();
  playerState_t const* const ps   = getPs();

  if (target_draw.integer && ps->weapon == WP_ROCKET_LAUNCHER)
  {
    gentity_t ent;
    BG_PlayerStateToEntityState(ps, &ent.s, qtrue);
    // use the snapped origin for linking so it matches client predicted versions
    VectorCopy(ent.s.pos.trBase, ent.r.currentOrigin);

    // execute client events
    // ClientEvents
    gentity_t m;
    FireWeapon(ps, &m, &ent);

    BG_EvaluateTrajectory(&m.s.pos, cg.time, origin);
    BG_EvaluateTrajectory(&m.s.pos, m.s.pos.trTime + MAX_RL_TIME, dest);
    trap_CM_BoxTrace(&beam_trace, origin, dest, NULL, NULL, 0, CONTENTS_SOLID);
    qhandle_t m_shader = trap_R_RegisterShader(target_shader.string);
    CG_ImpactMark(
      m_shader, beam_trace.endpos, beam_trace.plane.normal, 0, 1, 1, 1, 1, qfalse, target_size.value, qtrue);
  }

  // TODO: lerp trajectory stuff?
  for (int32_t i = 0; i < snap->numEntities; ++i)
  {
    entityState_t const entity = snap->entities[i];
    if (entity.eType == ET_MISSILE && entity.weapon == WP_ROCKET_LAUNCHER && entity.clientNum == ps->clientNum)
    {
      BG_EvaluateTrajectory(&entity.pos, cg.time, origin);
      BG_EvaluateTrajectory(&entity.pos, entity.pos.trTime + MAX_RL_TIME, dest);
      trap_CM_BoxTrace(&beam_trace, origin, dest, NULL, NULL, 0, CONTENTS_SOLID);
      if (path_draw.integer)
      {
        vec4_t color;
        VectorParse(path_rgba.string, color, 4);

        memset(&beam, 0, sizeof(beam));
        VectorCopy(origin, beam.oldorigin);
        VectorCopy(beam_trace.endpos, beam.origin);
        beam.reType       = RT_RAIL_CORE;
        beam.customShader = rl_.line_shader;
        AxisClear(beam.axis);
        beam.shaderRGBA[0] = (byte)(color[0] * 255);
        beam.shaderRGBA[1] = (byte)(color[1] * 255);
        beam.shaderRGBA[2] = (byte)(color[2] * 255);
        beam.shaderRGBA[3] = (byte)(color[3] * 255);
        trap_R_AddRefEntityToScene(&beam);
      }

      if (target_draw.integer)
      {
        qhandle_t m_shader = trap_R_RegisterShader(target_shader.string);
        CG_ImpactMark(
          m_shader, beam_trace.endpos, beam_trace.plane.normal, 0, 1, 1, 1, 1, qfalse, target_size.value, qtrue);
      }
    }
  }
}
