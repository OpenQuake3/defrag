#include "bg_public.h"
#include "q_math.h"

/*
================
BG_EvaluateTrajectory

================*/
void BG_EvaluateTrajectory(trajectory_t const* tr, int32_t atTime, vec3_t result)
{
  float deltaTime;
  float phase;

  switch (tr->trType)
  {
  case TR_STATIONARY:
  case TR_INTERPOLATE:
    VectorCopy(tr->trBase, result);
    break;
  case TR_LINEAR:
    deltaTime = (atTime - tr->trTime) * .001f; // milliseconds to seconds
    VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
    break;
  case TR_SINE:
    deltaTime = (atTime - tr->trTime) / (float)tr->trDuration;
    phase     = sinf(deltaTime * (float)M_PI * 2);
    VectorMA(tr->trBase, phase, tr->trDelta, result);
    break;
  case TR_LINEAR_STOP:
    // milliseconds to seconds
    deltaTime = ((atTime > tr->trTime + tr->trDuration ? tr->trTime + tr->trDuration : atTime) - tr->trTime) * .001f;
    if (deltaTime < 0)
    {
      deltaTime = 0;
    }
    VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
    break;
  case TR_GRAVITY:
    deltaTime = (atTime - tr->trTime) * .001f; // milliseconds to seconds
    VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
    result[2] -= .5f * DEFAULT_GRAVITY * deltaTime * deltaTime; // FIXME: local gravity...
    break;
  default:
    // Com_Error(ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trType);
    break;
  }
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================*/
void BG_EvaluateTrajectoryDelta(trajectory_t const* tr, int32_t atTime, vec3_t result)
{
  float deltaTime;
  float phase;

  switch (tr->trType)
  {
  case TR_STATIONARY:
  case TR_INTERPOLATE:
    VectorClear(result);
    break;
  case TR_LINEAR:
    VectorCopy(tr->trDelta, result);
    break;
  case TR_SINE:
    deltaTime = (atTime - tr->trTime) / (float)tr->trDuration;
    phase     = cosf(deltaTime * (float)M_PI * 2); // derivative of sin = cos
    phase *= .5f;
    VectorScale(tr->trDelta, phase, result);
    break;
  case TR_LINEAR_STOP:
    if (atTime > tr->trTime + tr->trDuration)
    {
      VectorClear(result);
      return;
    }
    VectorCopy(tr->trDelta, result);
    break;
  case TR_GRAVITY:
    deltaTime = (atTime - tr->trTime) * .001f; // milliseconds to seconds
    VectorCopy(tr->trDelta, result);
    result[2] -= DEFAULT_GRAVITY * deltaTime; // FIXME: local gravity...
    break;
  default:
    // Com_Error(ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trType);
    break;
  }
}

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================*/
void BG_PlayerStateToEntityState(playerState_t const* ps, entityState_t* s, qboolean snap)
{
  s->number = ps->clientNum;

  s->pos.trType = TR_INTERPOLATE;
  VectorCopy(ps->origin, s->pos.trBase);
  if (snap)
  {
    SnapVector(s->pos.trBase);
  }
  // set the trDelta for flag direction
  VectorCopy(ps->velocity, s->pos.trDelta);

  s->apos.trType = TR_INTERPOLATE;
  VectorCopy(ps->viewangles, s->apos.trBase);
  if (snap)
  {
    SnapVector(s->apos.trBase);
  }

  s->eFlags = ps->eFlags;

  s->weapon          = ps->weapon;
  s->groundEntityNum = ps->groundEntityNum;
}
