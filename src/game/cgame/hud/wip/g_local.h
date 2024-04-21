#ifndef G_LOCAL_H
#define G_LOCAL_H

#include "g_public.h"

//============================================================================

typedef struct gentity_s gentity_t;

struct gentity_s
{
  // incomplete
  entityState_t  s; // communicated by server to clients
  entityShared_t r; // shared by both the server system and game

  // DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
  // EXPECTS THE FIELDS IN THAT ORDER!
  //================================

  int32_t clipmask; // brushes with this content value will be collided against
                    // when moving.  items and corpses do not collide against
                    // players, for instance

  // movers
  gentity_t const* parent;

  gentity_t* target_ent;

  int32_t nextthink;

  int32_t damage;
  int32_t splashDamage; // quad will increase this without increasing radius
  int32_t splashRadius;
};

//
// g_missile.c
//
void fire_grenade(gentity_t* bolt, gentity_t const* ent, vec3_t const start, vec3_t dir);
void fire_rocket(gentity_t* bolt, gentity_t const* ent, vec3_t const start, vec3_t dir);

//
// g_weapon.c
//
void FireWeapon(playerState_t const* ps, gentity_t* m, gentity_t const* ent);

#endif // G_LOCAL_H
