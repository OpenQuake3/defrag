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
// @note This code would go at the bottom of  g_trigger.c
#include "../../../g_local.h"
#include "./entity.h"


static qboolean use_axis (
    ent_velocity_target_Flags const flags,
    int const pos,
    int const neg
  ) {
  qboolean has_pos = (flags & pos);
  qboolean has_neg = (flags & neg);
  return (has_pos || has_neg) && !(has_pos && has_neg);
}

// @WARN: Entity Spec Ambiguities
// - Whether "split" means divide among axes (if yes, how), or to give full speed to both
// - How does PERCENTAGE + LAUNCHER work ?? (eg: percentage of zero when stationary?)
// - How does non-LAUNCHER interact with axis flags when the direction comes from player's current velocity.
void Use_target_velocity (
    gentity_t* const self,
    gentity_t* const other,
    gentity_t* const activator
  ) {
  if (!activator->client                          ) return;
  if ( activator->client->ps.pm_type != PM_NORMAL ) return;
  if ( activator->client->ps.powerups[PW_FLIGHT]  ) return;
  //__________________
  // play fly sound every 1.5 seconds
  if (activator->fly_sound_debounce_time < level.time) {
    activator->fly_sound_debounce_time = level.time + 1500;
    G_Sound( activator, CHAN_AUTO, self->noise_index );
  }
  playerState_t* const ps = &activator->client->ps;
  //__________________
  // Initialize the values
  ent_velocity_Target ent = {0};
  ent.flags   = self->spawnflags;
  ent.speed   = self->speed;
  ent.sign[0] = ent.flags & ent_velocity_PosX ? 1.0f : -1.0f;
  ent.sign[1] = ent.flags & ent_velocity_PosY ? 1.0f : -1.0f;
  ent.sign[2] = ent.flags & ent_velocity_PosZ ? 1.0f : -1.0f;
  ent.use_x   = use_axis(ent.flags, ent_velocity_PosX, ent_velocity_NegX);
  ent.use_y   = use_axis(ent.flags, ent_velocity_PosY, ent_velocity_NegY);
  ent.use_z   = use_axis(ent.flags, ent_velocity_PosZ, ent_velocity_NegZ);

  float speed = (ent.flags & ent_velocity_Percentage)
    ? VectorLength(ps->velocity) * (ent.speed / 100.0f)
    : ent.speed;
  vec3_t velocity = {0};
  //__________________
  // Apply velocity
  if (ent.flags & ent_velocity_Launcher) {
    VectorCopy(ent.sign, velocity);
  } else {
    if (VectorLength(ps->velocity) < 0.001f) return;
    VectorCopy(ps->velocity, velocity);
    VectorNormalize(velocity);
  }
  if (ent.flags & ent_velocity_Add) {
    if (ent.use_x) ps->velocity[0] += velocity[0] * speed;
    if (ent.use_y) ps->velocity[1] += velocity[1] * speed;
    if (ent.use_z) ps->velocity[2] += velocity[2] * speed;
  } else {
    if (ent.use_x) ps->velocity[0]  = velocity[0] * speed;
    if (ent.use_y) ps->velocity[1]  = velocity[1] * speed;
    if (ent.use_z) ps->velocity[2]  = velocity[2] * speed;
  }
}


/**
 * @description
 * Defrag target_speed  TODO: Fields as seen in .map
 * "speed" defaults to 100
 */
void SP_target_velocity (
    gentity_t* const self
  ) {
  if (!self->speed) self->speed = 100;
  self->noise_index = G_SoundIndex("sound/misc/windfly.wav" );
  self->use = Use_target_velocity;
}

