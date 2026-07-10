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
#ifndef sgame_ent_velocity_target_H
#define sgame_ent_velocity_target_H

typedef enum ent_velocity_target_Flags {
  ent_velocity_Percentage = 1 << 0,
  ent_velocity_Add        = 1 << 1,
  ent_velocity_PosX       = 1 << 2,
  ent_velocity_NegX       = 1 << 3,
  ent_velocity_PosY       = 1 << 4,
  ent_velocity_NegY       = 1 << 5,
  ent_velocity_PosZ       = 1 << 6,
  ent_velocity_NegZ       = 1 << 7,
  ent_velocity_Launcher   = 1 << 8,
} ent_velocity_target_Flags;

typedef struct ent_velocity_Target {
  ent_velocity_target_Flags flags;
  float speed;
  vec3_t sign;
  qboolean use_x;
  qboolean use_y;
  qboolean use_z;
} ent_velocity_Target;


#endif // sgame_ent_velocity_target_H

