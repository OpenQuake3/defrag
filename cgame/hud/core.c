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

#include "local.h"  // hud/local.h
#include "../../qcommon/q_assert.h"
// #include "cg_snap.h"
// #include "cg_jump.h"
// #include "pitch.h"
// #include "cg_timer.h"
// #include "compass.h"
// #include "bbox.h"
// #include "cg_ammo.h"
// #include "cg_entity.h"
// #include "cg_gl.h"
// #include "cg_rl.h"

static vmCvar_t hud;
vmCvar_t hud_fov;
vmCvar_t hud_projection;

static cvarTable_t hud_cvartable[] = {
  { &hud, "hud_enable", "1", CVAR_ARCHIVE_ND },
  { &hud_fov, "hud_fov", "0", CVAR_ARCHIVE_ND },
  { &hud_projection, "hud_projection", "0", CVAR_ARCHIVE_ND },
};

void hud_init(void) {
  cvartable_init(hud_cvartable, ARRAY_LEN(hud_cvartable));

  hud_accel_init();
  // hud_snap_init();
  // hud_jump_init();
  // hud_timer_init();
  // hud_pitch_init();
  // hud_compass_init();
  // hud_bbox_init();
  // hud_ammo_init();
  // hud_entityStates_init();
  // hud_gl_init();
  // hud_rl_init();
}

void hud_term(void) {
  // hud_help_term();
}

void hud_update(void) {
  cvartable_update(hud_cvartable, ARRAY_LEN(hud_cvartable));
  if (!hud.integer) {return;}

  hud_accel_update();
  // hud_snap_update();
  // hud_jump_update();
  // hud_timer_update();
  // hud_pitch_update();
  // hud_compass_update();
  // hud_bbox_update();
  // hud_ammo_update();
  // hud_entityStates_update();
  // hud_gl_update();
  // hud_rl_update();
}

// This was called inside cg_vm.c in proxymod
// We are not proxying anything, so this goes directly to cgame instead (cg_draw.c)
void hud_draw(void) {
  if (!trap_CM_NumInlineModels()) {return;} // Check if we have models, otherwise CM_ClipHandleToModel will fail
  if (!hud.integer) {return;}  // If hud is disabled, don't draw it

  hud_accel_draw();
  // hud_snap_draw();
  // hud_compass_draw();
  // hud_pitch_draw();

  // hud_ammo_draw();
  // hud_jump_draw();
  // hud_timer_draw();
}

// hud help
void hud_help_init(help_t const* help, size_t size) {
  ASSERT_LT(helpTableIdx, ARRAY_LEN(helpTable));
  #ifndef NDEBUG
  for (size_t i = 0; i < size; ++i)  {
    size_t const len = strlen(help[i].cvarTable->cvarName);
    assert(strlen(help[i].message[0]) > len);
    assert(!Q_strncmp(help[i].message[0], help[i].cvarTable->cvarName, (int)len));
    assert(help[i].message[0][len] == ' ');
  }
  #endif
  helpTable[helpTableIdx].help = help;
  helpTable[helpTableIdx].size = size;
  ++helpTableIdx;
}
void hud_help_term(void) { helpTableIdx = 0; }

