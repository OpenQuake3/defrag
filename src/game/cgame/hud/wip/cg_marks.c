#include "cg_local.h"
#include "q_assert.h"

/*
=================
CG_ImpactMark

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================*/
#define MAX_MARK_FRAGMENTS 128
#define MAX_MARK_POINTS    384

void CG_ImpactMark(
  qhandle_t    markShader,
  vec3_t const origin,
  vec3_t const dir,
  float        orientation,
  float        red,
  float        green,
  float        blue,
  float        alpha,
  qboolean     alphaFade,
  float        radius,
  qboolean     temporary)
{
  (void)alphaFade;
  vec3_t         axis[3];
  float          texCoordScale;
  vec3_t         originalPoints[4];
  byte           colors[4];
  int32_t        i, j;
  int32_t        numFragments;
  markFragment_t markFragments[MAX_MARK_FRAGMENTS], *mf;
  vec3_t         markPoints[MAX_MARK_POINTS];
  vec3_t         projection;

  ASSERT_TRUE(temporary);

  // if (!cg_addMarks.integer)
  // {
  //   return;
  // }

  if (radius <= 0)
  {
    trap_Error("CG_ImpactMark called with <= 0 radius");
  }

  // if ( markTotal >= MAX_MARK_POLYS ) {
  //  return;
  //}

  // create the texture axis
  VectorNormalize2(dir, axis[0]);
  PerpendicularVector(axis[1], axis[0]);
  RotatePointAroundVector(axis[2], axis[0], axis[1], orientation);
  CrossProduct(axis[0], axis[2], axis[1]);

  texCoordScale = .5f * 1.f / radius;

  // create the full polygon
  for (i = 0; i < 3; i++)
  {
    originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
    originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
    originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
    originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
  }

  // get the fragments
  VectorScale(dir, -20, projection);
  numFragments = trap_CM_MarkFragments(
    4, (void*)originalPoints, projection, MAX_MARK_POINTS, markPoints[0], MAX_MARK_FRAGMENTS, markFragments);

  colors[0] = (byte)(red * 255);
  colors[1] = (byte)(green * 255);
  colors[2] = (byte)(blue * 255);
  colors[3] = (byte)(alpha * 255);

  for (i = 0, mf = markFragments; i < numFragments; i++, mf++)
  {
    polyVert_t* v;
    polyVert_t  verts[MAX_VERTS_ON_POLY];
    // markPoly_t* mark;

    // we have an upper limit on the complexity of polygons
    // that we store persistantly
    if (mf->numPoints > MAX_VERTS_ON_POLY)
    {
      mf->numPoints = MAX_VERTS_ON_POLY;
    }
    for (j = 0, v = verts; j < mf->numPoints; j++, v++)
    {
      vec3_t delta;

      VectorCopy(markPoints[mf->firstPoint + j], v->xyz);

      VectorSubtract(v->xyz, origin, delta);
      v->st[0] = .5f + DotProduct(delta, axis[1]) * texCoordScale;
      v->st[1] = .5f + DotProduct(delta, axis[2]) * texCoordScale;
      memcpy(v->modulate, colors, sizeof(v->modulate));
    }

    // if it is a temporary (shadow) mark, add it immediately and forget about it
    if (temporary)
    {
      trap_R_AddPolyToScene(markShader, mf->numPoints, verts);
      continue;
    }

    // // otherwise save it persistantly
    // mark                = CG_AllocMark();
    // mark->time          = cg.time;
    // mark->alphaFade     = alphaFade;
    // mark->markShader    = markShader;
    // mark->poly.numVerts = mf->numPoints;
    // mark->color[0]      = red;
    // mark->color[1]      = green;
    // mark->color[2]      = blue;
    // mark->color[3]      = alpha;
    // memcpy(mark->verts, verts, mf->numPoints * sizeof(verts[0]));
    // markTotal++;
  }
}
