#include "bg_public.h"
#include "cg_cvar.h"
#include "g_local.h"

static float  s_quadFactor;
static vec3_t forward;
static vec3_t muzzle;

/*
======================================================================

GRENADE LAUNCHER

======================================================================*/
static void weapon_grenadelauncher_fire(gentity_t* m, gentity_t const* ent)
{
  // extra vertical velocity
  forward[2] += .2f;
  VectorNormalize(forward);

  fire_grenade(m, ent, muzzle, forward);
  m->damage       = (int32_t)(m->damage * s_quadFactor);
  m->splashDamage = (int32_t)(m->splashDamage * s_quadFactor);

  // VectorAdd(m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta); // "real" physics
}

/*
======================================================================

ROCKET

======================================================================*/
static void Weapon_RocketLauncher_Fire(gentity_t* m, gentity_t const* ent)
{
  fire_rocket(m, ent, muzzle, forward);
  m->damage       = (int32_t)(m->damage * s_quadFactor);
  m->splashDamage = (int32_t)(m->splashDamage * s_quadFactor);

  // VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );  // "real" physics
}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============*/
static void CalcMuzzlePointOrigin(playerState_t const* ps, gentity_t const* ent, vec3_t muzzlePoint)
{
  VectorCopy(ent->s.pos.trBase, muzzlePoint);
  muzzlePoint[2] += ps->viewheight;
  VectorMA(muzzlePoint, 14, forward, muzzlePoint);
  // snap to integer coordinates for more efficient network bandwidth usage
  SnapVector(muzzlePoint);
}

/*
===============
FireWeapon
===============*/
void FireWeapon(playerState_t const* ps, gentity_t* m, gentity_t const* ent)
{
  if (ps->powerups[PW_QUAD])
  {
    s_quadFactor = cvar_getValue("g_quadfactor");
  }
  else
  {
    s_quadFactor = 1;
  }

  // set aiming directions
  AngleVectors(ps->viewangles, forward, NULL, NULL);

  CalcMuzzlePointOrigin(ps, ent, muzzle);

  // fire the specific weapon
  switch (ps->weapon)
  {
  // case WP_GAUNTLET: Weapon_Gauntlet(ent); break;
  // case WP_LIGHTNING: Weapon_LightningFire(ent); break;
  // case WP_SHOTGUN: weapon_supershotgun_fire(ent); break;
  // case WP_MACHINEGUN:
  //   if (g_gametype.integer != GT_TEAM)
  //   {
  //     Bullet_Fire(ent, MACHINEGUN_SPREAD, g_q3p_mgdamage.value, MOD_MACHINEGUN);
  //   }
  //   else
  //   {
  //     Bullet_Fire(ent, MACHINEGUN_SPREAD, g_q3p_mgteamdamage.value, MOD_MACHINEGUN);
  //   }
  //   break;
  case WP_GRENADE_LAUNCHER:
    weapon_grenadelauncher_fire(m, ent);
    break;
  case WP_ROCKET_LAUNCHER:
    Weapon_RocketLauncher_Fire(m, ent);
    break;
  // case WP_PLASMAGUN: Weapon_Plasmagun_Fire(ent); break;
  // case WP_RAILGUN: weapon_railgun_fire(ent); break;
  // case WP_BFG: BFG_Fire(ent); break;
  // case WP_GRAPPLING_HOOK: Weapon_GrapplingHook_Fire(ent); break;
  default:
    // FIXME    G_Error( "Bad ent->s.weapon" );
    break;
  }
}
