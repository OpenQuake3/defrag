static void hud_PmoveSingle(void);
static void hud_AirMove(void);
static void hud_WalkMove(void);


// void old_hud_accel_draw(void) {
//   if (!accel.integer) {return;}

//   s.ps = *getPs();
//   s.pm.tracemask = s.ps.pm_type == PM_DEAD ? MASK_PLAYERSOLID & ~CONTENTS_BODY : MASK_PLAYERSOLID;
//   if (VectorLengthSquared2(s.ps.velocity) >= accel_min_speed.value * accel_min_speed.value) {hud_PmoveSingle();}
// }

static void CG_DrawAccel(void);
static void hud_PmoveSingle(void) {
  // int8_t const scale = s.ps.stats[13] & PSF_USERINPUT_WALK ? 64 : 127;
  // if (!cg.demoPlayback && !(s.ps.pm_flags & PMF_FOLLOW)) {
    // int32_t const cmdNum = trap_GetCurrentCmdNumber();
    // trap_GetUserCmd(cmdNum, &s.pm.cmd);
  // } else {  // during demo, or Following in Spectator mode
  //   s.pm.cmd.forwardmove = scale * ((s.ps.stats[13] & PSF_USERINPUT_FORWARD)  / PSF_USERINPUT_FORWARD -
  //                                   (s.ps.stats[13] & PSF_USERINPUT_BACKWARD) / PSF_USERINPUT_BACKWARD);
  //   s.pm.cmd.rightmove   = scale * ((s.ps.stats[13] & PSF_USERINPUT_RIGHT)    / PSF_USERINPUT_RIGHT -
  //                                   (s.ps.stats[13] & PSF_USERINPUT_LEFT)     / PSF_USERINPUT_LEFT);
  //   s.pm.cmd.upmove      = scale * ((s.ps.stats[13] & PSF_USERINPUT_JUMP)     / PSF_USERINPUT_JUMP -
  //                                   (s.ps.stats[13] & PSF_USERINPUT_CROUCH)   / PSF_USERINPUT_CROUCH);
  // }

  // clear all pmove local vars
  // memset(&s.pml, 0, sizeof(s.pml));
  // save old velocity for crashlanding
  // VectorCopy(s.ps.velocity, s.pml.previous_velocity);
  // Update viewangles
  // AngleVectors(s.ps.viewangles, s.pml.forward, s.pml.right, s.pml.up);
  // Jump held or dead
  // if (s.pm.cmd.upmove < 10) {s.ps.pm_flags &= ~PMF_JUMP_HELD;} // not holding jump
  // if (s.ps.pm_type >= PM_DEAD) {s.pm.cmd.forwardmove = 0; s.pm.cmd.rightmove = 0;  s.pm.cmd.upmove = 0;}
  // Use default key combination when no user input
  if (!s.pm.cmd.forwardmove && !s.pm.cmd.rightmove) {s.pm.cmd.forwardmove = scale;}
  //::::::::::::::::::::::::::::::::::::
  // set mins, maxs, and viewheight
  // hud_CheckDuck(&s.pm, &s.ps);
  //::::::::::::::::::::::::::::::::::::
  // set watertype, and waterlevel
  // hud_SetWaterLevel(&s.pm, &s.ps);
  //::::::::::::::::::::::::::::::::::::
  // set groundentity
  // hud_GroundTrace(&s.pm, &s.ps, &s.pml);
  //::::::::::::::::::::::::::::::::::::
  // Skip cases
  // if      (s.ps.powerups[PW_FLIGHT])           {return;}
  // else if (s.ps.pm_flags & PMF_GRAPPLE_PULL)   {return;}
  // else if (s.ps.pm_flags & PMF_TIME_WATERJUMP) {return;}
  // else if (s.pm.waterlevel > 1)                {return;}
  // Get data
  else if (s.pml.walking) {hud_WalkMove();}
  // else                    {hud_AirMove();}
  // Draw it
  // CG_DrawAccel();
}

// CG_DrawAccel

//:::::::::::::::
// hud_Friction
//   Same as PM_ with some refactor to logic
//   TODO: Write assert to assume 0 <= cT <= 1
//:::::::::::::::

// slowDir_update
// fastDir_update
// stopDir_update
// turnDir_update
// dir_update

//:::::::::::::::
// hud_Accelerate
//   Handles user intended acceleration
//:::::::::::::::
// static void hud_Accelerate(float wishspeed, float accel) {
//   dir_update(wishspeed, accel, 0);
// }
static void hud_SlickAccelerate(float wishspeed, float accel) {
  dir_update(wishspeed, accel, s.ps.gravity * pm_frametime);
}

//:::::::::::::::
// hud_AirMove
//:::::::::::::::
static void hud_AirMove(void) {
//  hud_Friction();
  // Skips storing fmove, smove, cmd
  // New trueness check for scale
  float const scale = accel_trueness.integer & ACCEL_JUMPCROUCH ? PM_CmdScale   (&s.ps, &s.pm.cmd)
                                                                : PM_AltCmdScale(&s.ps, &s.pm.cmd);
  // Skips setMovementDir

  // project moves down to flat plane
//  s.pml.forward[2] = 0;
//  s.pml.right[2]   = 0;
//  VectorNormalize(s.pml.forward);
//  VectorNormalize(s.pml.right);

//  for (uint8_t i = 0; i < 2; ++i) {
//    s.wishvel[i] = s.pm.cmd.forwardmove * s.pml.forward[i] + s.pm.cmd.rightmove * s.pml.right[i];
//  }
  // skips zeroing Z wishvel. ignores it instead
  // float const wishspeed = scale * VectorLength2D(s.wishvel);

  // If show true CPM air control zones, and CPM is active
  if (accel_trueness.integer & ACCEL_CPM && s.ps.pm_flags & PMF_PROMODE) {
    // A/D accel 
    if (s.pm.cmd.forwardmove == 0 && s.pm.cmd.rightmove != 0) {
      hud_Accelerate(wishspeed > cpm_airwishspeed ? cpm_airwishspeed : wishspeed, cpm_airstrafeaccelerate);
    // Diagonal or W only
    } else {
      // Air control when s.pm.cmd.forwardmove != 0 && s.pm.cmd.rightmove == 0 only changes direction
      hud_Accelerate(wishspeed, pm_airaccelerate);
      // TODO: clean up
      if (s.turnDir > (float)M_PI / 2) {
        float       v_squared  = VectorLengthSquared2D(s.pml.previous_velocity);
        float const vf_squared = VectorLengthSquared2D(s.ps.velocity);
        float const a          = cpm_airstopaccelerate * wishspeed * pm_frametime;
        if (v_squared - vf_squared >= 2 * a * wishspeed - a * a) { v_squared = vf_squared; }
        float const vf = sqrtf(vf_squared);
        {
          float const num = v_squared - vf_squared - a * a;
          float const den = 2 * a * vf;
          if      ( num >= den) {s.turnDir = 0;}
          else if (-num >= den) {s.turnDir = (float)M_PI;}
          else                  {s.turnDir = acosf(num / den);}
        }
        ASSERT_LE(s.stopDir, s.turnDir);
      }
    }
  } else {
    hud_Accelerate(wishspeed, pm_airaccelerate);
  }

  // // we may have a ground plane that is very steep, even
  // // though we don't have a groundentity
  // // slide along the steep plane
  // if (s.pml->groundPlane) {
  //   PM_ClipVelocity(vf, s.pml->groundTrace.plane.normal, vf, OVERCLIP);
  // }

    // PM_StepSlideMove(qtrue);
}

/*
===================
hud_WalkMove
===================*/
float PM_CmdScale(playerState_t const* ps, usercmd_t const* cmd) {
  int32_t max = abs(cmd->forwardmove);
  if (abs(cmd->rightmove) > max) { max = abs(cmd->rightmove); }
  if (abs(cmd->upmove) > max)    { max = abs(cmd->upmove); }
  if (!max)                      { return 0; }
  const float total = sqrtf((float)(cmd->forwardmove * cmd->forwardmove + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove));
  return (float)ps->speed * max / (127.f * total);
}
/* PM_CmdScale without upmove */
float PM_AltCmdScale(playerState_t const* ps, usercmd_t const* cmd) {
  int32_t max = abs(cmd->forwardmove);
  if (abs(cmd->rightmove) > max) { max = abs(cmd->rightmove); }
  // Skips upmove
  if (!max)                      { return 0; }
  const float total = sqrtf((float)(cmd->forwardmove * cmd->forwardmove + cmd->rightmove * cmd->rightmove));
  return (float)ps->speed * max / (127.f * total);
}

void pmd_updateAccel(pmoveData_t* pmd) {
  const float scaletr = core_CmdScale(&pm->cmd, accel_trueness.integer & ACCEL_JUMPCROUCH)

  // wishspeed = scaletr * VectorLength2D(s.wishvel);
  // if (s.ps.pm_flags & PMF_DUCKED && wishspeed > s.ps.speed * pm_duckScale) {
  //   wishspeed = s.ps.speed * pm_duckScale;  }
  // // clamp the speed lower if wading or walking on the bottom
  // if (s.pm.waterlevel) {
  //   float const waterScale = 1.f - (1.f - pm_swimScale) * s.pm.waterlevel / 3.f;
  //   if (wishspeed > s.ps.speed * waterScale) {
  //     wishspeed = s.ps.speed * waterScale;
  //   }
  // }

  float gravity = (pml.groundTrace.surfaceFlags & SURF_SLICK || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ? pm->ps->gravity * pm_frametime : 0;
  dir_update(wishspeed, accel, gravity);
  // when a player gets hit, they temporarily lose full control, which allows them to be moved a bit
  // if (s.pml.groundTrace.surfaceFlags & SURF_SLICK || s.ps.pm_flags & PMF_TIME_KNOCKBACK) {
    // hud_Accelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.slowDir, s.fastDir, s.stopDir, s.turnDir));
    // hud_SlickAccelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
        // dir_update(wishspeed, accel, s.ps.gravity * pm_frametime);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.slowDir, s.fastDir, s.stopDir, s.turnDir));
  // } else {
    // hud_Accelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_accelerate : pm_accelerate);
        // dir_update(wishspeed, accel, 0);
  }
}
static void hud_WalkMove(void) {
  // if (s.pm.waterlevel > 2 && DotProduct(s.pml.forward, s.pml.groundTrace.plane.normal) > 0) {
  //   // // begin swimming
  //   // PM_WaterMove();
  //   return;  // skip if swimming
  // }

  // if (hud_CheckJump(&s.pm, &s.ps, &s.pml)) {
  //   // jumped away
  //   if (s.pm.waterlevel > 1) {
  //     // PM_WaterMove();  // skip if swimming up
  //   } else {
  //     hud_AirMove();  // update dir with airmove data
  //   }
  //   return;
  // }

  // hud_Friction();

  // skips fmove, smove and cmd
  // Scale cmd based on trueness
  float const scale = accel_trueness.integer & ACCEL_JUMPCROUCH ? PM_CmdScale(&s.ps, &s.pm.cmd)
                                                                : PM_AltCmdScale(&s.ps, &s.pm.cmd);
  // Skips setmovedir
  // project moves down to flat plane
  // s.pml.forward[2] = 0;
  // s.pml.right[2]   = 0;

  // TODO: only flat ground correct now
  // // project the forward and right directions onto the ground plane
  // PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
  // PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
  //
  // VectorNormalize(s.pml.forward);
  // VectorNormalize(s.pml.right);

  // for (uint8_t i = 0; i < 2; ++i) {
  //   s.wishvel[i] = s.pm.cmd.forwardmove * s.pml.forward[i] + s.pm.cmd.rightmove * s.pml.right[i];
  // }

  float wishspeed = scale * VectorLength2D(s.wishvel);

  // clamp the speed lower if ducking
  if (s.ps.pm_flags & PMF_DUCKED && wishspeed > s.ps.speed * pm_duckScale) {
    wishspeed = s.ps.speed * pm_duckScale;
  }

  // clamp the speed lower if wading or walking on the bottom
  if (s.pm.waterlevel) {
    float const waterScale = 1.f - (1.f - pm_swimScale) * s.pm.waterlevel / 3.f;
    if (wishspeed > s.ps.speed * waterScale) {
      wishspeed = s.ps.speed * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if (s.pml.groundTrace.surfaceFlags & SURF_SLICK || s.ps.pm_flags & PMF_TIME_KNOCKBACK) {
    // hud_Accelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.slowDir, s.fastDir, s.stopDir, s.turnDir));
    hud_SlickAccelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_slickaccelerate : pm_slickaccelerate);
    // g_syscall(CG_PRINT, vaf("a: %1.3f %1.3f %1.3f %1.3f\n", s.slowDir, s.fastDir, s.stopDir, s.turnDir));
  } else {
    // don't reset the z velocity for slopes
    // s.ps.velocity[2] = 0;
    hud_Accelerate(wishspeed, s.ps.pm_flags & PMF_PROMODE ? cpm_accelerate : pm_accelerate);
  }

  // // don't do anything if standing still
  // if (!s.ps.velocity[0] && !s.ps.velocity[1]) {return;}

  // PM_StepSlideMove(qfalse);
}
