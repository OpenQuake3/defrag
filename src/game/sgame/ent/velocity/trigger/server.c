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


void trigger_velocity_touch (
    gentity_t* const self,
    gentity_t* const other,
    trace_t* const trace
  ) {
  if (!other->client) return;
  BG_trigger_velocity_touch(&other->client->ps, &self->s);
}


/**
 * @description
 * Defrag trigger_velocity (.5 .5 .5) ?
 * Must point at a target_position, which will be the direction of the push.
 * Will be client side predicted.
 */
void SP_trigger_velocity (
    gentity_t* const self
  ) {
  InitTrigger(self);

  // unlike other triggers, we need to send this one to the client
  self->r.svFlags &= ~SVF_NOCLIENT;

  // make sure the client precaches this sound
  G_SoundIndex("sound/world/jumppad.wav");

  self->s.eType      = ET_TRIGGER_VELOCITY;
  self->touch        = trigger_velocity_touch;
  self->think        = AimAtTarget;
  self->nextthink    = level.time + FRAMETIME;
  self->s.weapon     = self->spawnflags;
  self->s.angles2[0] = self->speed;        // XY
  self->s.angles2[1] = self->speed;        // XY
  self->s.angles2[2] = (float)self->count; // Z
  trap_LinkEntity(self);
}

