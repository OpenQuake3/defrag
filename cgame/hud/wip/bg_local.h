#ifndef BG_LOCAL_H
#define BG_LOCAL_H

#include "q_shared.h"

#define MIN_WALK_NORMAL .7f // can't walk on very steep slopes

#define STEPSIZE 18

#define JUMP_VELOCITY 270

#define OVERCLIP 1.001f

#define pm_frametime .008f

// movement parameters
#define pm_stopspeed 100.f
#define pm_duckScale .25f
#define pm_swimScale .50f

#define pm_accelerate      10.f
#define pm_airaccelerate   1.f
#define pm_slickaccelerate 1.f
#define pm_wateraccelerate 4.f
#define pm_flyaccelerate   8.f

#define cpm_accelerate      15.f
#define cpm_slickaccelerate 15.f

#define cpm_airstopaccelerate   2.5f
#define cpm_airstrafeaccelerate 70.f
#define cpm_airwishspeed        30.f

#define pm_friction          6.f
#define pm_waterfriction     1.f
#define pm_flightfriction    3.f
#define pm_spectatorfriction 5.f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct
{
  vec3_t forward, right, up;
  float  frametime;

  int32_t msec;

  qboolean walking;
  qboolean groundPlane;
  trace_t  groundTrace;

  float impactSpeed;

  vec3_t  previous_origin;
  vec3_t  previous_velocity;
  int32_t previous_waterlevel;
} pml_t;

#endif // BG_LOCAL_H
