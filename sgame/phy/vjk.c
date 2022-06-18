#include "osdf.h"

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
  phy_jump_velocity = 225; // default 270
}

void q3a_vjk(pmove_t *pmove) {
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
  q3a_Weapon();
  // torso animation
  PM_TorsoAnimation();
  // footstep events / legs animations
  PM_Footsteps();
  // entering / leaving water splashes
  PM_WaterEvents();
  // snap some parts of playerstate to save network bandwidth
  trap_SnapVector(pm->ps->velocity);
}

