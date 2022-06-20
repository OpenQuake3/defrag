#include "cg_cvar.h"
#include "cg_local.h"
#include "q_assert.h"

/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================*/
static void CG_CalcVrect(void)
{
  cg.refdef.width = cgs.glconfig.vidWidth;

  cg.refdef.height = cgs.glconfig.vidHeight;

  cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width) / 2;
  cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height) / 2;
}

/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================*/
static void CG_CalcFov(void)
{
  float fov_x = hud_fov.value > 0 ? hud_fov.value : cvar_getValue("cg_fov");
  ASSERT_GT(fov_x, 0);

  // set it
  cg.refdef.fov_x = DEG2RAD(fov_x);
  cg.refdef.fov_y = atan2f((float)cg.refdef.height, cg.refdef.width / tanf(cg.refdef.fov_x / 2)) * 2;
}

/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============*/
static void CG_CalcViewValues(void)
{
  memset(&cg.refdef, 0, sizeof(cg.refdef));

  // calculate size of 3D view
  CG_CalcVrect();

  // field of view
  CG_CalcFov();
}

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================*/
void CG_DrawActiveFrame(int32_t serverTime, stereoFrame_t stereoView, qboolean demoPlayback)
{
  (void)stereoView;

  cg.time         = serverTime;
  cg.demoPlayback = demoPlayback;

  // update cvartable
  CG_UpdateCvars();

  // build cg.refdef
  CG_CalcViewValues();

  // finish up the rest of the refdef
  cg.refdef.time = cg.time;
}
