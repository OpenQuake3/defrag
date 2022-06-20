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
#include "cg_utils.h"

#include "cg_cvar.h"
#include "cg_local.h"
#include "cg_vm.h"
#include "defrag.h"

snapshot_t const* getSnap(void)
{
  static snapshot_t snapshot;
  int32_t           curSnapNum;
  int32_t           servertime;
  trap_GetCurrentSnapshotNumber(&curSnapNum, &servertime);
  trap_GetSnapshot(curSnapNum, &snapshot);
  return &snapshot;
}

playerState_t const* getPs(void)
{
  if (cvar_getInteger("g_synchronousClients")) return &getSnap()->ps;
  return (playerState_t const*)VM_ArgPtr(defrag()->pps_offset);
}
