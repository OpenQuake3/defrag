#include "pitch.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_utils.h"
#include "help.h"

#include <stdlib.h>

static vmCvar_t pitch;
static vmCvar_t pitch_xwh;
static vmCvar_t pitch_rgba;

static cvarTable_t pitch_cvartable[] = {
  { &pitch, "hud_pitch", "", CVAR_ARCHIVE_ND },
  { &pitch_xwh, "hud_pitch_xwh", "316 8 1", CVAR_ARCHIVE_ND },
  { &pitch_rgba, "hud_pitch_rgba", ".8 .8 .8 .8", CVAR_ARCHIVE_ND },
};

static help_t pitch_help[] = {
  {
    pitch_cvartable + 1,
    X | W | H,
    {
      "hud_pitch_xwh X X X",
    },
  },
  {
    pitch_cvartable + 2,
    RGBA,
    {
      "hud_pitch_rgba X X X X",
    },
  },
};

void hud_pitch_init(void)
{
  cvartable_init(pitch_cvartable, ARRAY_LEN(pitch_cvartable));
  hud_help_init(pitch_help, ARRAY_LEN(pitch_help));
}

void hud_pitch_update(void)
{
  cvartable_update(pitch_cvartable, ARRAY_LEN(pitch_cvartable));
}

typedef struct
{
  vec3_t graph_xwh;
  vec4_t graph_rgba;

  playerState_t ps;
} pitch_t;

static pitch_t s;

void hud_pitch_draw(void)
{
  if (pitch.string[0] == '\0') return;

  VectorParse(pitch_xwh.string, s.graph_xwh, 3);
  VectorParse(pitch_rgba.string, s.graph_rgba, 4);

  s.ps = *getPs();

  float const p = DEG2RAD(s.ps.viewangles[PITCH]);

  char const* str = pitch.string;
  char*       end;
  for (float angle = strtof(str, &end); str != end; angle = strtof(str, &end))
  {
    float const x = s.graph_xwh[0];
    float const w = s.graph_xwh[1];
    float const h = s.graph_xwh[2];
    CG_DrawLinePitch(DEG2RAD(angle), p, x, w, h, s.graph_rgba);
    str = end;
  }
}
