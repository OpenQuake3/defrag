#include "bbox.h"

#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "help.h"

static vmCvar_t bbox;
static vmCvar_t bbox_rgba;

static cvarTable_t bbox_cvartable[] = {
  { &bbox, "hud_bbox", "0", CVAR_ARCHIVE_ND },
  { &bbox_rgba, "hud_bbox_rgba", ".9 .5 .7 .7", CVAR_ARCHIVE_ND },
};

static help_t bbox_help[] = {
  {
    bbox_cvartable + 1,
    RGBA,
    {
      "hud_bbox_rgba X X X X",
    },
  },
};

typedef struct
{
  vec4_t               bbox_rgba;
  playerState_t const* ps;

  // qhandle_t bboxShader;
  qhandle_t bboxShader_nocull;

  vec3_t mins;
  vec3_t maxs;

  float extx;
  float exty;
  float extz;

  polyVert_t verts[4];
} bbox_t;

static bbox_t s;

void hud_bbox_init(void)
{
  cvartable_init(bbox_cvartable, ARRAY_LEN(bbox_cvartable));
  hud_help_init(bbox_help, ARRAY_LEN(bbox_help));

  // get the shader handles
  // s.bboxShader        = trap_R_RegisterShader("bbox");
  s.bboxShader_nocull = trap_R_RegisterShader("bbox_nocull");

  s.mins[0] = s.mins[1] = -15;
  s.mins[2]             = -24;
  s.maxs[0] = s.maxs[1] = 15;

  // get the extents (size)
  s.extx = s.maxs[0] - s.mins[0];
  s.exty = s.maxs[1] - s.mins[1];

  // set the polygon's texture coordinates
  s.verts[0].st[0] = 0;
  s.verts[0].st[1] = 0;
  s.verts[1].st[0] = 0;
  s.verts[1].st[1] = 1;
  s.verts[2].st[0] = 1;
  s.verts[2].st[1] = 1;
  s.verts[3].st[0] = 1;
  s.verts[3].st[1] = 0;
}

void hud_bbox_update(void)
{
  cvartable_update(bbox_cvartable, ARRAY_LEN(bbox_cvartable));
}

static void CG_AddBoundingBox(void);

void draw_bbox(void)
{
  if (!bbox.integer) return;

  VectorParse(bbox_rgba.string, s.bbox_rgba, 4);

  s.ps = getPs();

  CG_AddBoundingBox();
}

static void CG_AddBoundingBox(void)
{
  int    i;
  vec3_t corners[8];
  //             4 ----- 7
  //   z        /|      /|
  //   | _ y   5 ----- 6 |
  //  /        | |     | |
  // x         | 0 ----| 3
  //           |/      |/
  //           1 ----- 2

  // TODO
  // don't draw it in third-person
  // if (!cg_thirdPerson.integer)
  //   return;

  // don't draw it for dead players
  if (s.ps->pm_type >= PM_SPECTATOR) return;

  // if they don't exist, forget it
  if (/*!s.bboxShader || */!s.bboxShader_nocull) return;

  if (s.ps->pm_flags & PMF_DUCKED)
  {
    s.maxs[2] = 16;
  }
  else
  {
    s.maxs[2] = 32;
  }

  // get the extents (size)
  s.extz = s.maxs[2] - s.mins[2];

  // set the polygon's vertex colors
  for (i = 0; i < 4; ++i)
  {
    s.verts[i].modulate[0] = (byte)(s.bbox_rgba[0] * 255);
    s.verts[i].modulate[1] = (byte)(s.bbox_rgba[1] * 255);
    s.verts[i].modulate[2] = (byte)(s.bbox_rgba[2] * 255);
    s.verts[i].modulate[3] = (byte)(s.bbox_rgba[3] * 255);
  }

  VectorAdd(s.ps->origin, s.mins, corners[0]);

  VectorCopy(corners[0], corners[1]);
  corners[1][0] += s.extx;

  VectorCopy(corners[1], corners[2]);
  corners[2][1] += s.exty;

  VectorCopy(corners[2], corners[3]);
  corners[3][0] -= s.extx;

  // bottom
  VectorCopy(corners[0], s.verts[0].xyz);
  VectorCopy(corners[1], s.verts[1].xyz);
  VectorCopy(corners[2], s.verts[2].xyz);
  VectorCopy(corners[3], s.verts[3].xyz);
  trap_R_AddPolyToScene(s.bboxShader_nocull, 4, s.verts);

  if (bbox.integer == 1)
  {
    for (i = 0; i < 4; ++i)
    {
      VectorCopy(corners[i], corners[i + 4]);
      corners[i + 4][2] += s.extz;
    }

    // top
    VectorCopy(corners[7], s.verts[0].xyz);
    VectorCopy(corners[6], s.verts[1].xyz);
    VectorCopy(corners[5], s.verts[2].xyz);
    VectorCopy(corners[4], s.verts[3].xyz);
    trap_R_AddPolyToScene(s.bboxShader_nocull, 4, s.verts);

    // top side
    VectorCopy(corners[3], s.verts[0].xyz);
    VectorCopy(corners[2], s.verts[1].xyz);
    VectorCopy(corners[6], s.verts[2].xyz);
    VectorCopy(corners[7], s.verts[3].xyz);
    trap_R_AddPolyToScene(s.bboxShader_nocull, 4, s.verts);

    // left side
    VectorCopy(corners[2], s.verts[0].xyz);
    VectorCopy(corners[1], s.verts[1].xyz);
    VectorCopy(corners[5], s.verts[2].xyz);
    VectorCopy(corners[6], s.verts[3].xyz);
    trap_R_AddPolyToScene(s.bboxShader_nocull, 4, s.verts);

    // right side
    VectorCopy(corners[0], s.verts[0].xyz);
    VectorCopy(corners[3], s.verts[1].xyz);
    VectorCopy(corners[7], s.verts[2].xyz);
    VectorCopy(corners[4], s.verts[3].xyz);
    trap_R_AddPolyToScene(s.bboxShader_nocull, 4, s.verts);

    // bottom side
    VectorCopy(corners[1], s.verts[0].xyz);
    VectorCopy(corners[0], s.verts[1].xyz);
    VectorCopy(corners[4], s.verts[2].xyz);
    VectorCopy(corners[5], s.verts[3].xyz);
    trap_R_AddPolyToScene(s.bboxShader_nocull, 4, s.verts);
  }
}
