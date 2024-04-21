#include "cg_local.h"
#include "g_local.h"

#define MISSILE_PRESTEP_TIME 50

/*
=================
fire_grenade
=================*/
void fire_grenade(gentity_t* bolt, gentity_t const* self, vec3_t const start, vec3_t dir)
{
  VectorNormalize(dir);

  bolt->nextthink    = cg.time + 2500;
  bolt->s.eType      = ET_MISSILE;
  bolt->s.weapon     = WP_GRENADE_LAUNCHER;
  bolt->s.eFlags     = EF_BOUNCE_HALF;
  bolt->r.ownerNum   = self->s.number;
  bolt->parent       = self;
  bolt->damage       = 100;
  bolt->splashDamage = 100;
  bolt->splashRadius = 150;
  bolt->clipmask     = MASK_SHOT;
  bolt->target_ent   = NULL;

  bolt->s.pos.trType = TR_GRAVITY;
  bolt->s.pos.trTime = cg.time - MISSILE_PRESTEP_TIME; // move a bit on the very first frame
  VectorCopy(start, bolt->s.pos.trBase);
  VectorScale(dir, 700, bolt->s.pos.trDelta);
  SnapVector(bolt->s.pos.trDelta); // save net bandwidth

  VectorCopy(start, bolt->r.currentOrigin);
}

/*
=================
fire_rocket
=================*/
void fire_rocket(gentity_t* bolt, gentity_t const* self, vec3_t const start, vec3_t dir)
{
  VectorNormalize(dir);

  bolt->nextthink    = cg.time + 15000;
  bolt->s.eType      = ET_MISSILE;
  bolt->s.weapon     = WP_ROCKET_LAUNCHER;
  bolt->r.ownerNum   = self->s.number;
  bolt->parent       = self;
  bolt->damage       = 100;
  bolt->splashDamage = 100;
  bolt->splashRadius = 120;
  bolt->clipmask     = MASK_SHOT;
  bolt->target_ent   = NULL;

  bolt->s.pos.trType = TR_LINEAR;
  bolt->s.pos.trTime = cg.time - MISSILE_PRESTEP_TIME; // move a bit on the very first frame
  VectorCopy(start, bolt->s.pos.trBase);
  VectorScale(dir, 900, bolt->s.pos.trDelta);
  SnapVector(bolt->s.pos.trDelta); // save net bandwidth

  VectorCopy(start, bolt->r.currentOrigin);
}
