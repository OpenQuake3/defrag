#include "compass.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"
#include "help.h"

static vmCvar_t compass;
static vmCvar_t compass_yh;
static vmCvar_t compass_quadrant_rgbas;
static vmCvar_t compass_ticks_rgba;
static vmCvar_t compass_arrow_rgbas;

static cvarTable_t compass_cvartable[] = {
  { &compass, "hud_compass", "0b000", CVAR_ARCHIVE_ND },
  { &compass_yh, "hud_compass_yh", "188 8", CVAR_ARCHIVE_ND },
  { &compass_quadrant_rgbas,
    "hud_compass_quadrant_rgbas",
    "1 1 0 .25 / 0 1 0 .25 / 0 0 1 .25 / 1 0 1 .25",
    CVAR_ARCHIVE_ND },
  { &compass_ticks_rgba, "hud_compass_ticks_rgba", "1 1 1 1", CVAR_ARCHIVE_ND },
  { &compass_arrow_rgbas, "hud_compass_arrow_rgbas", "1 1 1 1 / 1 .5 0 1", CVAR_ARCHIVE_ND },
};

static help_t compass_help[] = {
  { compass_cvartable + 0,
    BINARY_LITERAL,
    {
      "hud_compass 0bXXX",
      "              |||",
      "              ||+- draw quadrants",
      "              |+-- draw ticks",
      "              +--- draw arrow",
    } },
#define QUADRANTS 1
#define TICKS     2
#define ARROW     4
  {
    compass_cvartable + 1,
    Y | H,
    {
      "hud_compass_yh X X",
    },
  },
  {
    compass_cvartable + 2,
    RGBAS,
    {
      "hud_compass_quadrant_rgbas X X X X / X X X X / X X X X / X X X X",
    },
  },
  {
    compass_cvartable + 3,
    RGBA,
    {
      "hud_compass_ticks_rgba X X X X",
    },
  },
  {
    compass_cvartable + 4,
    RGBAS,
    {
      "hud_compass_arrow_rgbas X X X X / X X X X",
    },
  },
};

void hud_compass_init(void)
{
  cvartable_init(compass_cvartable, ARRAY_LEN(compass_cvartable));
  hud_help_init(compass_help, ARRAY_LEN(compass_help));
}

void hud_compass_update(void)
{
  cvartable_update(compass_cvartable, ARRAY_LEN(compass_cvartable));
  compass.integer = cvar_getInteger("hud_compass");
}

typedef struct
{
  vec2_t graph_yh;
  vec4_t graph_quadrant_rgbas[4];
  vec4_t graph_arrow_rgba[2];
  vec4_t graph_ticks_rgba;

  playerState_t ps;
} compass_t;

static compass_t s;

void hud_compass_draw(void)
{
  if (!compass.integer) return;

  VectorParse(compass_yh.string, s.graph_yh, 2);

  s.ps = *getPs();

  float const yaw = DEG2RAD(s.ps.viewangles[YAW]);

  if (compass.integer & QUADRANTS)
  {
    VectorParse4(compass_quadrant_rgbas.string, s.graph_quadrant_rgbas, 4);
    CG_FillAngleYaw(0, (float)M_PI / 2, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[0]);
    CG_FillAngleYaw((float)M_PI / 2, (float)M_PI, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[1]);
    CG_FillAngleYaw(-(float)M_PI / 2, -(float)M_PI, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[2]);
    CG_FillAngleYaw(0, -(float)M_PI / 2, yaw, s.graph_yh[0], s.graph_yh[1], s.graph_quadrant_rgbas[3]);
  }

  if (compass.integer & TICKS)
  {
    VectorParse(compass_ticks_rgba.string, s.graph_ticks_rgba, 4);
    {
      float const y = s.graph_yh[0] + s.graph_yh[1] / 2;
      float const w = 1;
      float const h = s.graph_yh[1] / 2;
      CG_DrawLineYaw(0, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw((float)M_PI / 2, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw((float)M_PI, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(-(float)M_PI / 2, yaw, y, w, h, s.graph_ticks_rgba);
    }
    {
      float const y = s.graph_yh[0] + 3 * s.graph_yh[1] / 4;
      float const w = 1;
      float const h = s.graph_yh[1] / 4;
      CG_DrawLineYaw((float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(3 * (float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(-(float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
      CG_DrawLineYaw(-3 * (float)M_PI / 4, yaw, y, w, h, s.graph_ticks_rgba);
    }
  }

  if (compass.integer & ARROW && (s.ps.velocity[0] != 0 || s.ps.velocity[1] != 0))
  {
    VectorParse4(compass_arrow_rgbas.string, s.graph_arrow_rgba, 2);
    vec4_t* color = &s.graph_arrow_rgba[0];
    if (s.ps.velocity[0] == 0 || s.ps.velocity[1] == 0)
    {
      color = &s.graph_arrow_rgba[1];
    }

    float const v_dir = atan2f(s.ps.velocity[1], s.ps.velocity[0]);

    float const y = s.graph_yh[0] + s.graph_yh[1];
    float const w = s.graph_yh[1];
    float const h = s.graph_yh[1] / 2;

    uint8_t const ch_up   = 135; // Arrow pointing up.
    uint8_t const ch_down = 134; // Arrow pointing down.

    CG_DrawCharYaw(v_dir, yaw, y, w, h, ch_up, *color);
    CG_DrawCharYaw(v_dir, yaw - (float)M_PI, y, w, h, ch_down, *color);
  }
}
