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


void Use_target_velocity (
    gentity_t* const self,
    gentity_t* const other,
    gentity_t* const activator
  ) {
  if (!activator->client                          ) return;
  if ( activator->client->ps.pm_type != PM_NORMAL ) return;
  if ( activator->client->ps.powerups[PW_FLIGHT]  ) return;

  VectorCopy(self->s.origin2, activator->client->ps.velocity);

  // play fly sound every 1.5 seconds
  if (activator->fly_sound_debounce_time < level.time) {
    activator->fly_sound_debounce_time = level.time + 1500;
    G_Sound( activator, CHAN_AUTO, self->noise_index );
  }
}


/**
 * @description
 * Defrag target_push (.5 .5 .5) (-8 -8 -8) (8 8 8) bouncepad
 * Pushes the activator in the direction.of angle, or towards a target apex.
 * "speed" defaults to 1000
 * if "bouncepad", play bounce noise instead of windfly
 */
void SP_target_velocity (
    gentity_t* const self
  ) {
  if (!self->speed) self->speed = 1000;
  G_SetMovedir(self->s.angles, self->s.origin2);
  VectorScale(self->s.origin2, self->speed, self->s.origin2);

  self->noise_index = (self->spawnflags & 1)
    ? G_SoundIndex("sound/world/jumppad.wav")
    : G_SoundIndex("sound/misc/windfly.wav" );

  if (self->target) {
    VectorCopy( self->s.origin, self->r.absmin );
    VectorCopy( self->s.origin, self->r.absmax );
    self->think     = AimAtTarget;
    self->nextthink = level.time + FRAMETIME;
  }
  self->use = Use_target_velocity;
}

