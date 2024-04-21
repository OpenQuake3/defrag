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

// input:  origin, velocity, bounds, groundPlane, trace function
// output: origin, velocity, impacts, stairup boolean

//==================
// SlideMove
//   Returns qtrue if the velocity was clipped in some way
//==================
#define MAX_CLIP_PLANES 5
// Forward declare
static qboolean q3a_SlideMove(qboolean gravity);
static qboolean new_SlideMove(qboolean gravity);
// Select type
qboolean core_SlideMove(qboolean gravity) {
  switch (pm->movetype){
    case VQ3: return q3a_SlideMove(gravity);
    case CPM: return q3a_SlideMove(gravity);
    case VQ1: return q3a_SlideMove(gravity);
    case VQ2: return new_SlideMove(gravity);
    case VQ4: return new_SlideMove(gravity);
    case VJK: return q3a_SlideMove(gravity);
    case CQ3: return q3a_SlideMove(gravity);
  }
}
// Unmodded Q3A version
static qboolean q3a_SlideMove( qboolean gravity ) {
  int     bumpcount, numbumps;
  vec3_t  dir;
  float   d;
  int     numplanes;
  vec3_t  planes[MAX_CLIP_PLANES];
  vec3_t  primal_velocity;
  vec3_t  clipVelocity;
  int     i, j, k;
  trace_t trace;
  vec3_t  end;
  float   time_left;
  float   into;
  vec3_t  endVelocity;
  vec3_t  endClipVelocity;
  
  numbumps = 4;

  VectorCopy (pm->ps->velocity, primal_velocity);

  if ( gravity ) {
    VectorCopy( pm->ps->velocity, endVelocity );
    endVelocity[2] -= pm->ps->gravity * pml.frametime;
    pm->ps->velocity[2] = ( pm->ps->velocity[2] + endVelocity[2] ) * 0.5;
    primal_velocity[2] = endVelocity[2];
    if ( pml.groundPlane ) {
      // slide along the ground plane
      VectorReflect (pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, overbounce_scale);
    }
  }

  time_left = pml.frametime;

  // never turn against the ground plane
  if ( pml.groundPlane ) {
    numplanes = 1;
    VectorCopy( pml.groundTrace.plane.normal, planes[0] );
  } else {
    numplanes = 0;
  }

  // never turn against original velocity
  VectorNormalize2( pm->ps->velocity, planes[numplanes] );
  numplanes++;

  for ( bumpcount=0 ; bumpcount < numbumps ; bumpcount++ ) {
    // calculate position we are trying to move to
    VectorMA( pm->ps->origin, time_left, pm->ps->velocity, end );
    // see if we can make it there
    pm->trace ( &trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);

    // entity is completely trapped in another solid
    if (trace.allsolid) {
      pm->ps->velocity[2] = 0;  // don't build up falling damage, but allow sideways acceleration
      return qtrue;
    }

    // actually covered some distance
    if (trace.fraction > 0) { VectorCopy (trace.endpos, pm->ps->origin); }
    // moved the entire distance
    if (trace.fraction == 1) { break; }
    // save entity for contact
    PM_AddTouchEnt( trace.entityNum );

    time_left -= time_left * trace.fraction;

    if (numplanes >= MAX_CLIP_PLANES) {
      // this shouldn't really happen
      VectorClear( pm->ps->velocity );
      return qtrue;
    }

    //
    // if this is the same plane we hit before, nudge velocity
    // out along it, which fixes some epsilon issues with
    // non-axial planes
    //
    for ( i = 0 ; i < numplanes ; i++ ) {
      if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99 ) {
        VectorAdd( trace.plane.normal, pm->ps->velocity, pm->ps->velocity );
        break;
      }
    }
    if ( i < numplanes ) { continue; }
    VectorCopy (trace.plane.normal, planes[numplanes]);
    numplanes++;
    //
    // modify velocity so it parallels all of the clip planes
    //
    // find a plane that it enters
    for ( i = 0 ; i < numplanes ; i++ ) {
      into = DotProduct( pm->ps->velocity, planes[i] );
      if ( into >= 0.1 ) { continue;}  // move doesn't interact with the plane
      // see how hard we are hitting things
      if ( -into > pml.impactSpeed ) {pml.impactSpeed = -into;}
      // slide along the plane
      VectorReflect (pm->ps->velocity, planes[i], clipVelocity, overbounce_scale);
      // slide along the plane
      VectorReflect(endVelocity, planes[i], endClipVelocity, overbounce_scale); // IoQuake3 Wrapped this behind a gravity check. This version is default q3a

      // see if there is a second plane that the new move enters
      for ( j = 0 ; j < numplanes ; j++ ) {
        if ( j == i ) {continue;}
        if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) {continue;}  // move doesn't interact with the plane
        // try clipping the move to the plane
        VectorReflect(clipVelocity, planes[j], clipVelocity, overbounce_scale);
        VectorReflect(endClipVelocity, planes[j], endClipVelocity, overbounce_scale); //IoQuake3 wrapped this inside a gravity check. This version is default q3a-1.32
        // see if it goes back into the first clip plane
        if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) {continue;}
        // slide the original velocity along the crease
        CrossProduct (planes[i], planes[j], dir);
        VectorNormalize( dir );
        d = DotProduct( dir, pm->ps->velocity );
        VectorScale( dir, d, clipVelocity );

        if ( gravity ) {
          CrossProduct (planes[i], planes[j], dir);
          VectorNormalize( dir );
          d = DotProduct( dir, endVelocity );
          VectorScale( dir, d, endClipVelocity );
        }

        // see if there is a third plane the the new move enters
        for ( k = 0 ; k < numplanes ; k++ ) {
          if ( k == i || k == j ) {continue;}
          if ( DotProduct( clipVelocity, planes[k] ) >= 0.1 ) {continue;}   // move doesn't interact with the plane
          // stop dead at a tripple plane interaction
          VectorClear( pm->ps->velocity );
          return qtrue;
        }
      }
      // if we have fixed all interactions, try another move
      VectorCopy( clipVelocity, pm->ps->velocity );  
      VectorCopy( endClipVelocity, endVelocity ); //IoQuake3 wrapped this inside a gravity check. This version is default q3a-1.32
      break;
    }
  }

  if (gravity) { VectorCopy(endVelocity, pm->ps->velocity); }
  // don't change velocity if in a timer (aka do skimming)
  if (pm->ps->pm_time) { VectorCopy(primal_velocity, pm->ps->velocity); }
  return ( bumpcount != 0 );
}
// New slidemove tech. Adds:
//   Optional skimming
//   Rampslides
static qboolean new_SlideMove( qboolean gravity ) {
  int      bumpcount, numbumps;
  vec3_t   dir;
  float    d;
  int      numplanes;
  vec3_t   planes[MAX_CLIP_PLANES];
  vec3_t   primal_velocity;
  vec3_t   clipVelocity;
  int      i, j, k;
  trace_t  trace;
  vec3_t   end;
  float    time_left;
  float    into;
  vec3_t   endVelocity;
  vec3_t   endClipVelocity;
  
  numbumps = 4;

  VectorCopy (pm->ps->velocity, primal_velocity);

  if ( gravity ) {
    VectorCopy( pm->ps->velocity, endVelocity );
    endVelocity[2] -= pm->ps->gravity * pml.frametime;
    pm->ps->velocity[2] = ( pm->ps->velocity[2] + endVelocity[2] ) * 0.5;
    primal_velocity[2] = endVelocity[2];
    // slide along the ground plane
    if (pml.groundPlane) { VectorReflect(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, overbounce_scale); }
  }

  time_left = pml.frametime;

  // never turn against the ground plane
  if ( pml.groundPlane ) { numplanes = 1; VectorCopy( pml.groundTrace.plane.normal, planes[0] ); }
  else                   { numplanes = 0; }
  // never turn against original velocity
  VectorNormalize2( pm->ps->velocity, planes[numplanes] );
  numplanes++;

  for ( bumpcount=0 ; bumpcount < numbumps ; bumpcount++ ) {
    // calculate position we are trying to move to
    VectorMA( pm->ps->origin, time_left, pm->ps->velocity, end );
    // see if we can make it there
    pm->trace ( &trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, pm->tracemask);
    // entity is completely trapped in another solid
    if (trace.allsolid) {
      pm->ps->velocity[2] = 0;  // don't build up falling damage, but allow sideways acceleration
      return qtrue;
    }
    // actually covered some distance
    if (trace.fraction > 0) { VectorCopy (trace.endpos, pm->ps->origin); }
    // moved the entire distance
    if (trace.fraction == 1) { break; }
    // save entity for contact
    PM_AddTouchEnt( trace.entityNum );

    time_left -= time_left * trace.fraction;

    // this shouldn't really happen
    if (numplanes >= MAX_CLIP_PLANES) { VectorClear( pm->ps->velocity ); return qtrue; }

    // if this is the same plane we hit before,
    // nudge velocity out along it. fixes epsilon issues with non-axial planes
    for ( i = 0 ; i < numplanes ; i++ ) {
      if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99 ) {
        VectorAdd( trace.plane.normal, pm->ps->velocity, pm->ps->velocity );
        break;
      }
    }
    if ( i < numplanes ) { continue; }
    VectorCopy (trace.plane.normal, planes[numplanes]);
    numplanes++;
    //
    // modify velocity so it parallels all of the clip planes
    //
    // find a plane that it enters
    for ( i = 0 ; i < numplanes ; i++ ) {
      into = DotProduct( pm->ps->velocity, planes[i] );
      if ( into >= 0.1 ) { continue;}  // move doesn't interact with the plane
      // see how hard we are hitting things
      if ( -into > pml.impactSpeed ) {pml.impactSpeed = -into;}
      // slide along the plane
      VectorReflect (pm->ps->velocity, planes[i], clipVelocity, overbounce_scale );
      // slide along the plane
      VectorReflect (endVelocity, planes[i], endClipVelocity, overbounce_scale ); // IoQuake3 Wrapped this behind a gravity check. This version is default q3a

      // see if there is a second plane that the new move enters
      for ( j = 0 ; j < numplanes ; j++ ) {
        if ( j == i ) {continue;}
        if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) {continue;}  // move doesn't interact with the plane
        // try clipping the move to the plane
        VectorReflect( clipVelocity, planes[j], clipVelocity, overbounce_scale );
        VectorReflect( endClipVelocity, planes[j], endClipVelocity, overbounce_scale ); //IoQuake3 wrapped this inside a gravity check. This version is default q3a-1.32
        // see if it goes back into the first clip plane
        if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) {continue;}
        // slide the original velocity along the crease
        CrossProduct (planes[i], planes[j], dir);
        VectorNormalize( dir );
        d = DotProduct( dir, pm->ps->velocity );
        VectorScale( dir, d, clipVelocity );

        if ( gravity ) {
          CrossProduct (planes[i], planes[j], dir);
          VectorNormalize( dir );
          d = DotProduct( dir, endVelocity );
          VectorScale( dir, d, endClipVelocity );
        }

        // see if there is a third plane the the new move enters
        for ( k = 0 ; k < numplanes ; k++ ) {
          if ( k == i || k == j ) {continue;}
          if ( DotProduct( clipVelocity, planes[k] ) >= 0.1 ) {continue;}   // move doesn't interact with the plane
          // stop dead at a tripple plane interaction
          VectorClear( pm->ps->velocity );
          return qtrue;
        }
      }
      // if we have fixed all interactions, try another move
      VectorCopy( clipVelocity, pm->ps->velocity );  
      VectorCopy( endClipVelocity, endVelocity ); //IoQuake3 wrapped this inside a gravity check. This version is default q3a-1.32
      break;
    }
  }

  if (gravity) { VectorCopy(endVelocity, pm->ps->velocity); }
  // don't change velocity if in a timer
  if (pm->ps->pm_time && phy_skim_enable) { VectorCopy(primal_velocity, pm->ps->velocity); }  // Do skimming, if enabled
  return ( bumpcount != 0 );
}
//==================
// StepSlideMove
//   Handles stepmove behavior
//==================
// Forward declare
static void q3a_StepMove(qboolean gravity);
static void new_StepMove(qboolean gravity);
// Select type
void core_StepSlideMove(qboolean gravity) {
  switch (pm->movetype){
    case VQ3: q3a_StepMove(gravity); break;
    case CPM: q3a_StepMove(gravity); break;
    case VQ1: q3a_StepMove(gravity); break;
    case VQ2: new_StepMove(gravity); break;
    case VQ4: new_StepMove(gravity); break;
    case VJK: q3a_StepMove(gravity); break;
    case CQ3: q3a_StepMove(gravity); break;
  }
}
// Unmodded Q3A version
static void q3a_StepMove( qboolean gravity ) {
  vec3_t    start_o, start_v;
  // vec3_t    down_o, down_v;
  trace_t   trace;
  // float     down_dist, up_dist;
  // vec3_t    delta, delta2;
  vec3_t    up, down;
  float     stepSize;
  float     delta;
  qboolean  timerActive, cantDoubleJump, isSteepRamp;
  int       max_jumpvel;

  VectorCopy (pm->ps->origin, start_o);
  VectorCopy (pm->ps->velocity, start_v);

  if ( core_SlideMove( gravity ) == 0 ) {return;}   // we got exactly where we wanted to go first try 
  VectorCopy(start_o, down);
  down[2] -= STEPSIZE;
  pm->trace (&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
  // Step up
  max_jumpvel    = phy_jump_velocity + phy_jump_dj_velocity;
  timerActive    = ( pm->cmd.serverTime - pm->ps->stats[STAT_TIME_LASTJUMP] < phy_jump_timebuffer ) ? qtrue : qfalse;
  cantDoubleJump = ( pm->movetype == VQ3 || !timerActive || pm->ps->velocity[2] > max_jumpvel ) ? qtrue : qfalse;
  VectorSet(up, 0, 0, 1);
  isSteepRamp    = DotProduct(trace.plane.normal, up) < MIN_WALK_NORMAL ? qtrue:qfalse;
  // never step up when:
  //   Step-down trace moved all the way down, (or) we are in a steepramp
  //   (and) still have up velocity
  //   (and) You can't doublejump (vq3 or dj-timer is not active)
  //   (and) Vertical speed is bigger than the maximum possible dj speed (prevent stairs-climb crazyness) (included in cantDoubleJump)
  if (((trace.fraction == 1.0 || isSteepRamp) 
      && pm->ps->velocity[2] > 0)
      && cantDoubleJump)
    { return; }

  //VectorCopy (pm->ps->origin, down_o);
  //VectorCopy (pm->ps->velocity, down_v);

  VectorCopy (start_o, up);
  up[2] += STEPSIZE;
  // test the player position if they were a stepheight higher
  pm->trace (&trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
  if ( trace.allsolid ) {
    if ( pm->debugLevel ) { Com_Printf("%i:bend can't step\n", c_pmove); }
    VectorClear(pm->ps->velocity); // Wallbug fix
    return;   // can't step up
  }

  stepSize = trace.endpos[2] - start_o[2];
  // try slidemove from this position
  VectorCopy (trace.endpos, pm->ps->origin);
  VectorCopy (start_v, pm->ps->velocity);

  core_SlideMove( gravity );

  // push down the final amount
  VectorCopy (pm->ps->origin, down);
  down[2] -= stepSize;
  pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
  if ( !trace.allsolid )      { VectorCopy (trace.endpos, pm->ps->origin); }
  if ( trace.fraction < 1.0 ) { VectorReflect( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, overbounce_scale ); }  // VectorReflect2D for CPM ??
  // use the step move
  delta = pm->ps->origin[2] - start_o[2];
  if ( delta > 2 ) {
    if      ( delta < 7 )  { PM_AddEvent( EV_STEP_4 ); }
    else if ( delta < 11 ) { PM_AddEvent( EV_STEP_8 ); }
    else if ( delta < 15 ) { PM_AddEvent( EV_STEP_12 ); }
    else                   { PM_AddEvent( EV_STEP_16 ); }
  }
  if ( pm->debugLevel ) { Com_Printf("%i:stepped\n", c_pmove); }
}
// New stepmove tech. Adds:
//   Rampslides
//   Double jumps on rampslides
// Forward declarations
static void core_SlideReflect(vec3_t startOrigin, vec3_t startVel, qboolean gravity, trace_t trace);
static void core_Rampslide(vec3_t startVel, trace_t trace);
// Definition
static void new_StepMove(qboolean gravity) {
  vec3_t     start_o, start_v;
  VectorCopy (pm->ps->origin, start_o);
  VectorCopy (pm->ps->velocity, start_v);

  if ( core_SlideMove( gravity ) == 0 ) {return;}   // we got exactly where we wanted to go first try 
  // Step Down
  vec3_t down;
  VectorCopy(start_o, down);
  down[2] -= phy_step_size;
  trace_t trace; pm->trace (&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
  // Step up
  qboolean jumpvel_cap;
  { // Check if we should cap jumpvel (not doing stepup), if we are over the max jumpvel allowed
    qboolean overcap   = (pm->ps->velocity[2] > phy_step_maxvel) ? qtrue:qfalse;  // We are over the limit
    qboolean activecap = (phy_step_maxvel >= 0)                  ? qtrue:qfalse;  // A limit is active
    jumpvel_cap        = (overcap && activecap)                  ? qtrue:qfalse;  // Store result of both checks
  }
  qboolean timerActive    = ( pm->cmd.serverTime - pm->ps->stats[STAT_TIME_LASTJUMP] < phy_jump_timebuffer ) ? qtrue:qfalse;
  qboolean cantDoubleJump = ( phy_jump_type == VQ3 || !timerActive || jumpvel_cap ) ? qtrue:qfalse;
  vec3_t up; VectorSet(up, 0, 0, 1);
  qboolean isSteepRamp    = DotProduct(trace.plane.normal, up) < MIN_WALK_NORMAL ? qtrue:qfalse;
  // never step up when:
  //   Step-down trace moved all the way down, (or) we are in a steepramp
  //   (and) still have up velocity
  //   (and) You can't doublejump (vq3 or dj-timer is not active)
  //   (and) Vertical speed is bigger than the maximum allowed (included in cantDoubleJump)
  if (((trace.fraction == 1.0 || isSteepRamp) 
      && pm->ps->velocity[2] > 0)
      && cantDoubleJump)
    { return; }

  VectorCopy (start_o, up);
  up[2] += phy_step_size;
  // test the player position if they were a stepheight higher
  pm->trace (&trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
  if ( trace.allsolid ) {
    if ( pm->debugLevel ) { Com_Printf("%i:bend can't step\n", c_pmove); }
    VectorClear(pm->ps->velocity); // Wallbug fix
    return;   // can't step up
  }

  float stepSize = trace.endpos[2] - start_o[2];
  // try slidemove from this position
  VectorCopy (trace.endpos, pm->ps->origin);
  VectorCopy (start_v, pm->ps->velocity);

  core_SlideMove( gravity );

  // push down the final amount
  VectorCopy (pm->ps->origin, down);
  down[2] -= stepSize;
  pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
  if ( !trace.allsolid )      { VectorCopy (trace.endpos, pm->ps->origin); }
  if ( trace.fraction < 1.0 ) { core_SlideReflect(start_o, start_v, gravity, trace); }  
  core_Rampslide(start_v, trace);  // Apply rampslide velocity if needed
  // use the step move
  float delta = pm->ps->origin[2] - start_o[2];
  if ( delta > 2 ) {
    if      ( delta < 7 )  { PM_AddEvent( EV_STEP_4 ); }
    else if ( delta < 11 ) { PM_AddEvent( EV_STEP_8 ); }
    else if ( delta < 15 ) { PM_AddEvent( EV_STEP_12 ); }
    else                   { PM_AddEvent( EV_STEP_16 ); }
  }
  if ( pm->debugLevel ) { Com_Printf("%i:stepped\n", c_pmove); }
}

// Applies rampslide effect
//   Won't do anything if the conditions are not met
static void core_Rampslide(vec3_t startVel, trace_t trace){
  qboolean slick       = (pml.groundTrace.surfaceFlags & SURF_SLICK)  ? qtrue:qfalse;
  qboolean knockback   = (pm->ps->pm_flags & PMF_TIME_KNOCKBACK)      ? qtrue:qfalse;
  qboolean crouchslide = (pm->ps->stats[STAT_TIME_CROUCHSLIDE] > 0)   ? qtrue:qfalse;
  qboolean slide       = (slick || knockback || crouchslide)          ? qtrue:qfalse;
  qboolean style1      = (phy_rampslide_type == VQ1 && slide)         ? qtrue:qfalse;
  qboolean style2      = (phy_rampslide_type == VQ2)                  ? qtrue:qfalse;
  qboolean flatramp    = (trace.plane.normal[2] >= MIN_WALK_NORMAL)   ? qtrue:qfalse;
  float    oldSpeed    = VectorLength2D(startVel);
  float    curSpeed    = VectorLength2D(pm->ps->velocity);
  qboolean lostSpeed   = (oldSpeed > curSpeed)                        ? qtrue:qfalse;
  // if the player is in a flat ramp,
  // AND they are in style 2 or style 1 with slick,
  // AND they lostSpeed because of some collision, 
  if (flatramp && (style1 || style2) && lostSpeed) {
    // restore their original horizontal velocity (but keep their Z)
    VectorSet(pm->ps->velocity, startVel[0], startVel[1], pm->ps->velocity[2]);
    if ( pm->debugLevel ) { Com_Printf("%i:Rampslide.core\n", c_pmove); }
  }
}

static void core_SlideReflect(vec3_t startOrigin, vec3_t startVel, qboolean gravity, trace_t trace) { 
  qboolean steepramp     = (trace.plane.normal[2] < MIN_WALK_NORMAL) ? qtrue:qfalse;

  qboolean stepped       = (trace.endpos[2] > startOrigin[2] + (startVel[2] * pml.frametime)) ? qtrue:qfalse;
  qboolean movingUp      = (startVel[2] > 0)                ? qtrue:qfalse;
  qboolean noRampslide   = (!pm->ps->stats[STAT_RAMPSLIDE]) ? qtrue:qfalse;
  qboolean noJumpInput   = (!(pm->cmd.upmove > 10))         ? qtrue:qfalse;

    qboolean jumpstyle2    = (phy_jump_type == 2) ? qtrue:qfalse;
    qboolean jumpstyle4    = (phy_jump_type == 4) ? qtrue:qfalse;
    int      jumptimer     = pm->cmd.serverTime - pm->ps->stats[STAT_TIME_LASTJUMP];
    qboolean canDoublejump = ((jumptimer <= phy_jump_timebuffer ) && (jumptimer > 0))  ? qtrue:qfalse;
    qboolean doublejump    = ((jumpstyle2 || jumpstyle4) && canDoublejump)             ? qtrue:qfalse;

  if (steepramp) {  // TODO: This case will never be reached?
    VectorCopy(startOrigin, pm->ps->origin);
    VectorCopy(startVel, pm->ps->velocity);
    core_SlideMove(gravity);
    if ( pm->debugLevel ) { Com_Printf("%i:Rampslide.steep\n", c_pmove); }
  // if stepped, while moving up, while not rampsliding and are not holding jump, stick to the ground
  } else if (stepped && movingUp && noRampslide && noJumpInput) {
    if (doublejump) { VectorReflect(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, overbounce_scale); } // VectorReflect2D for CPM ??
    else            { VectorCopy(startVel, pm->ps->velocity); }
    if ( pm->debugLevel && doublejump) { Com_Printf("%i:Rampslide.sticky->doublejump\n", c_pmove); }
    else if (pm->debugLevel)           { Com_Printf("%i:Rampslide.sticky->notdj\n", c_pmove); }
  } else {
    if ( pm->debugLevel ) { Com_Printf("%i:SlideReflect.normal\n", c_pmove); }
    VectorReflect(pm->ps->velocity, trace.plane.normal, pm->ps->velocity, overbounce_scale); // VectorReflect2D for CPM ??
  }
}
