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
#ifndef sgame_ent_init_H
#define sgame_ent_init_H

typedef enum ent_init_Flag {
  ent_init_ArmorKeep        = 1 << 0,
  ent_init_HealthKeep       = 1 << 1,
  ent_init_WeaponsKeep      = 1 << 2,
  ent_init_KeepPowerups     = 1 << 3,
  ent_init_HoldableKeep     = 1 << 4,
  ent_init_MachinegunRemove = 1 << 5,
  ent_init_GauntletRemove   = 1 << 6,
} ent_init_Flag;

#endif // sgame_ent_init_H

