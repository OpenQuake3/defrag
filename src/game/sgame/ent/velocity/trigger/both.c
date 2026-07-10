/*
  ==============================
  Written by:
    id software :            Quake III Arena
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
// @note This code would go at the bottom of bg_misc.c
#include "../../../g_local.h"
#include "./entity.h"


void BG_trigger_velocity_touch (
    playerState_t* const ps,
    entityState_t* const state
  ) {
  //__________________
  // spectators and flying characters don't use velocity pads
  if (ps->pm_type != PM_NORMAL) return;
  if (ps->powerups[PW_FLIGHT]) return;
  //__________________
  // if we didn't hit this same jumppad the previous frame
  // then don't play the event sound again if we are in a fat trigger
  if (ps->jumppad_ent != state->number) {
    vec3_t angles = {0};
    vectoangles(state->origin2, angles);
    float const p = fabs( AngleNormalize180( angles[PITCH] ) );
    BG_AddPredictableEventToPlayerstate(EV_JUMP_PAD, (p < 45), ps);
  }
  //__________________
  // remember hitting this jumppad this frame
  qboolean first_touch = (ps->jumppad_ent != state->number);
  ps->jumppad_ent      = state->number;
  ps->jumppad_frame    = ps->pmove_framecount;

  //__________________
  // Initialize the entity's data
  ent_Velocity ent = {0};
  ent.flags = state->weapon;
  VectorCopy(state->angles2, ent.speed);
  VectorCopy(state->origin2, ent.direction);
  //__________________
  // Early Fallback to JumpPad behavior if flags == 0
  if (ent.flags == 0) {
    VectorCopy(ent.direction, ps->velocity);
    return;
  }

  //__________________
  // Store the current velocity before the change
  vec3_t original = {0};
  VectorCopy(ps->velocity, original);
  // After
  vec_t x = 0;
  vec_t y = 0;
  vec_t z = 0;
  //__________________
  // XY axis
  if (ent.flags & ent_velocity_PlayerXY) {  // Use the player.dir.xy normalized and multiply it by the speed
    vec3_t dir = { original[0], original[1], 0 };
    VectorNormalize(dir);
    x = dir[0] * ent.speed[0];
    y = dir[1] * ent.speed[1];
  } else {  // Otherwise use the entity's direction.xy (speed already pre-computed by AimAtTarget)
    x = ent.direction[0];
    y = ent.direction[1];
    if (ent.flags & ent_velocity_BidirectionalXY) {
      vec3_t plus  = { original[0] + x, original[1] + y, 0 };
      vec3_t minus = { original[0] - x, original[1] - y, 0 };
      if (VectorLength(plus) < VectorLength(minus)) {
        x = -x;
        y = -y;
      }
    }
  }
  //__________________
  // Z axis
  if (ent.flags & ent_velocity_PlayerZ) {  // Use the player.dir.z normalized and multiply it by the speed
    vec3_t dir = { 0, 0, original[2] };
    VectorNormalize(dir);
    z = dir[2] * ent.speed[2];
  } else {  // Otherwise use the entity's direction.z (speed already pre-computed by AimAtTarget)
    z = ent.direction[2];
    if (ent.flags & ent_velocity_BidirectionalZ) {
      vec3_t plus  = { 0, 0, original[2] + z };
      vec3_t minus = { 0, 0, original[2] - z };
      if (VectorLength(plus) < VectorLength(minus)) {
        z = -z;
      }
    }
  }
  //__________________
  // Apply.XY
  if (ent.flags & ent_velocity_AddXY && first_touch) {  // Add only on the first frame we hit the trigger
    x = original[0] + x;
    y = original[1] + y;
    if (ent.flags & ent_velocity_ClampNegative) {  // Clamp when dot(xy, orig) is negative
      float const dot = (x * original[0] + y * original[1]);
      if (dot < 0) { x = 0; y = 0; }
    }
  }
  ps->velocity[0] = x;
  ps->velocity[1] = y;
  //__________________
  // Apply.Z
  if (ent.flags & ent_velocity_AddZ && first_touch) {
    z = original[2] + z;
    if (ent.flags & ent_velocity_ClampNegative) {
      float const dot = z * original[2];
      if (dot < 0) { z = 0; }
    }
  }
  ps->velocity[2] = z;
}

