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
#ifndef sgame_ent_trigger_velocity_H
#define sgame_ent_trigger_velocity_H

#include "../../../../qcommon/q_shared.h"

typedef enum ent_velocity_Flags {
  /// If set, trigger will apply the horizontal speed in the player's horizontal direction of travel,
  /// otherwise it uses the target XY component.
  /// df.ent name: PLAYERDIR_XY
  ent_velocity_PlayerXY = 1 << 0,
  /// If set, trigger will add to the player's horizontal velocity,
  /// otherwise it sets the player's horizontal velociy.
  /// df.ent name: ADD_XY
  ent_velocity_AddXY = 1 << 1,
  /// If set, trigger will apply the vertical speed in the player's vertical direction of travel,
  /// otherwise it uses the target Z component.</flag>
  /// df.ent name: PLAYERDIR_Z
  ent_velocity_PlayerZ = 1 << 2,
  /// If set, trigger will add to the player's vertical velocity,
  /// otherwise it sets the player's vectical velociy.
  /// df.ent name: ADD_Z
  ent_velocity_AddZ = 1 << 3,
  /// If set, non-playerdir velocity pads will function in 2 directions based on the target specified.
  /// The chosen direction is based on the current direction of travel.
  /// Applies to horizontal direction.
  /// df.ent name: BIDIRECTIONAL_XY
  ent_velocity_BidirectionalXY = 1 << 4,
  /// If set, non-playerdir velocity pads will function in 2 directions based on the target specified.
  /// The chosen direction is based on the current direction of travel.
  /// Applies to vertical direction.
  /// df.ent name: BIDIRECTIONAL_Z
  ent_velocity_BidirectionalZ = 1 << 5,
  ///  If set, then a velocity pad that adds negative velocity will be clamped to 0,
  ///  if the resultant velocity would bounce the player in the opposite direction.
  /// df.ent name: CLAMP_NEGATIVE_ADDS
  ent_velocity_ClampNegative = 1 << 6,
} ent_velocity_Flags;

typedef struct ent_Velocity {
  ent_velocity_Flags flags;
  vec3_t speed;
  vec3_t direction;
} ent_Velocity;

#endif //sgame_ent_trigger_velocity_H

