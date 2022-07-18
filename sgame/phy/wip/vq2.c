
void vq2_init(){
  phy_stopspeed = pm_stopspeed;
  phy_crouch_scale = pm_duckScale;
  // Acceleration
  phy_ground_accel = pm_accelerate;
  phy_air_accel = pm_airaccelerate;
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
  phy_jump_auto = qfalse;
  phy_jump_velocity = JUMP_VELOCITY;
  phy_step_maxvel = phy_jump_velocity;
}


