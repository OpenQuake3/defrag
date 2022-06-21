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


#include "local.h"
#include "q3a.h"

void vjk_init(){
  phy_stopspeed = pm_stopspeed;
  phy_crouch_scale = pm_duckScale;
  // Acceleration
  phy_ground_accel = 12;
  phy_air_accel = 4;  // and   phy_speed = 250 
  phy_fly_accel = pm_flyaccelerate;
  // Friction
  phy_friction = pm_friction;
  phy_fly_friction = pm_flightfriction;
  phy_spectator_friction = pm_spectatorfriction;
  // Water
  phy_water_accel = pm_wateraccelerate;
  phy_water_scale = pm_swimScale;
  phy_water_friction = pm_waterfriction;
  // New
  phy_slidemove_type = Q3A;
  phy_snapvelocity = qtrue;
  phy_input_scalefix = qfalse;
  phy_aircontrol = qfalse;
  phy_jump_type = VQ3;
  phy_jump_auto = qtrue;
  phy_jump_velocity = JUMP_VELOCITY; // vjk = 225. vq3 default = JUMP_VELOCITY = 270
}

void vjk_move(pmove_t *pmove) {
  // set mins, maxs, and viewheight
  PM_CheckDuck();
  // set groundentity
  PM_GroundTrace();

  // do deadmove  :moved to top
  // drop timers
  PM_DropTimers();

  if (pm->ps->powerups[PW_FLIGHT]) {
    PM_FlyMove(); // flight powerup doesn't allow jump and has different friction
  } else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL) {
    PM_GrappleMove();
    PM_AirMove(); // We can wiggle a bit
  } else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
    PM_WaterJumpMove();
  } else if (pm->waterlevel > 1) {
    PM_WaterMove(); // swimming
  } else if (pml.walking) {
    q3a_WalkMove(); // walking on ground
  } else {
    q3a_AirMove(); // airborne
  }
  // animations
  PM_Animate();
  // set groundentity, watertype, and waterlevel
  PM_GroundTrace();
  PM_SetWaterLevel();
  // weapons
  core_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // snap some parts of playerstate to save network bandwidth
  trap_SnapVector(pm->ps->velocity);
}

