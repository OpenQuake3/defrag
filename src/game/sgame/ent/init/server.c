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
// @note This code would go at the bottom of  g_target.c
#include "../../g_local.h"
#include "./entity.h"

void Use_target_init(gentity_t* const ent, gentity_t* const other, gentity_t* const activator) {
	if (!activator || !activator->client) return;

	gclient_t* const     client = activator->client;
	playerState_t* const ps     = &client->ps;

	// Com_Printf("target_init: spawnflags=%d, MachinegunRemove=%d, GauntletRemove=%d\n",
	//   ent->spawnflags,
	//   (ent->spawnflags & ent_init_MachinegunRemove) ? 1 : 0,
	//   (ent->spawnflags & ent_init_GauntletRemove  ) ? 1 : 0);

	// Armor
	if (!(ent->spawnflags & ent_init_ArmorKeep)) {
		ps->stats[STAT_ARMOR] = 0;
	}

	if (!(ent->spawnflags & ent_init_HealthKeep)) {
		// Com_Printf("DEBUG: Resetting health from %d to %d\n",
		//     ps->stats[STAT_HEALTH], ps->stats[STAT_MAX_HEALTH] + 25);
		ps->stats[STAT_HEALTH] = ps->stats[STAT_MAX_HEALTH] + 25;
		activator->health      = ps->stats[STAT_HEALTH];
	}

	// Weapons (bitmask in STAT_WEAPONS)
	if (!(ent->spawnflags & (ent_init_WeaponsKeep | ent_init_MachinegunRemove | ent_init_GauntletRemove))) {
		ps->stats[STAT_WEAPONS] = 0;
	}

	// Selectively remove machinegun ammo if flagged
	if (ent->spawnflags & ent_init_MachinegunRemove) {
		ps->ammo[WP_MACHINEGUN] = 0;
		ps->stats[STAT_WEAPONS] &= ~(1 << WP_MACHINEGUN);
	}

	if (ent->spawnflags & ent_init_GauntletRemove) {
		ps->ammo[WP_GAUNTLET] = 0;
		ps->stats[STAT_WEAPONS] &= ~(1 << WP_GAUNTLET);
		// Com_Printf("DEBUG: Removed gauntlet. STAT_WEAPONS now = %d\n", ps->stats[STAT_WEAPONS]);
	}

	// Reset current weapon only if we're actually removing chaingun
	if ((ent->spawnflags & ent_init_GauntletRemove) && ps->weapon == WP_GAUNTLET) {
		ps->weapon = WP_NONE;
	}

	// Powerups
	if (!(ent->spawnflags & ent_init_KeepPowerups)) {
		for (int i = 0; i < MAX_POWERUPS; ++i) {
			ps->powerups[i] = 0;
		}
	}

	// Holdables
	if (!(ent->spawnflags & ent_init_HoldableKeep)) {
		ps->stats[STAT_HOLDABLE_ITEM] = 0;
	}

	// Ensure player has gauntlet (but not if we just removed it)
	if (!(ent->spawnflags & ent_init_GauntletRemove) && !(ps->stats[STAT_WEAPONS] & (1 << WP_GAUNTLET))) {
		ps->stats[STAT_WEAPONS] |= (1 << WP_GAUNTLET);
		ps->ammo[WP_GAUNTLET] = 200;
		ps->weapon            = WP_GAUNTLET;
	}
}

void SP_target_init(gentity_t* const ent) {
	ent->use       = Use_target_init;
	ent->classname = "target_init";
	trap_LinkEntity(ent);
}
