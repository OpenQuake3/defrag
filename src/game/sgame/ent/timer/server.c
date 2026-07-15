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
#include "../../g_local.h"
#include "./entity.h"


void Use_target_startTimer (
    gentity_t* const self,
    gentity_t* const other,
    gentity_t* const activator
  ) {
  int const clNum = activator - g_entities;
  gclient_t* const cl = activator->client;

  // Skip cases
  if (!cl) return;                            // Activator is not a client
  if (cl->ps.pm_type != PM_NORMAL) return;    // Client is not in normal movement mode
  if (cl->ps.stats[STAT_HEALTH] <= 0) return; // Client is not alive

  // Hitting start trigger
  cl->timer_start = cl->ps.commandTime;   // Set start as commandTime (aka servertime)
  // Com_Printf("timerStart:: ps.commandTime= %i || timer_start= %i\n", cl->ps.commandTime, cl->timer_start);
  // Notify client: New timer started at timer_start
  trap_SendServerCommand(clNum, va("timerStart %i", cl->timer_start));
}

void SP_target_startTimer (gentity_t* const self) {
  self->use = Use_target_startTimer;
}


void Use_target_stopTimer (
    gentity_t* const self,
    gentity_t* const other,
    gentity_t* const activator
  ) {
  int const clNum     = activator - g_entities;
  gclient_t* const cl = activator->client;

  // Skip cases
  if (!cl) return;                             // Activator is not a client
  if (cl->ps.pm_type != PM_NORMAL) return;     // Client is not in normal movement mode
  if (cl->ps.stats[STAT_HEALTH] <= 0) return;  // Client is not alive

  // Hitting end trigger
  cl->timer_end = cl->ps.commandTime;  // Set end time as commandTime (aka servertime)
  // Notify: New time as servertime
  trap_SendServerCommand(clNum, va("timerEnd %i", cl->timer_end));
}

void SP_target_stopTimer (gentity_t* const self) {
  self->use = Use_target_stopTimer;
}

