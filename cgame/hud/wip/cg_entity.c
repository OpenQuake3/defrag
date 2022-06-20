#include "cg_entity.h"

#include "cg_cvar.h"
#include "cg_utils.h"

static vmCvar_t sound_local_only;

static cvarTable_t sound_cvartable[] = {
  { &sound_local_only, "hud_sound_local_only", "0", CVAR_ARCHIVE_ND },
};

static entityState_t cg_entityStates[1024];

void hud_entityStates_init(void)
{
  cvartable_init(sound_cvartable, ARRAY_LEN(sound_cvartable));

  memset(cg_entityStates, -1, sizeof(cg_entityStates));
}

void hud_entityStates_update(void)
{
  cvartable_update(sound_cvartable, ARRAY_LEN(sound_cvartable));

  snapshot_t const* const snap = getSnap();
  for (int i = 0; i < snap->numEntities; i++)
  {
    cg_entityStates[snap->entities[i].number] = snap->entities[i];
  }
}

int8_t should_filter_sound(int entity_num, int8_t is_loop)
{
  // TODO: also bail if its sp?
  if (!sound_local_only.integer) return 0;

  // this messes up our own weapon noises
  // can't do anything about this w/o df code
  if (!is_loop && entity_num == ENTITYNUM_WORLD) return 1;

  // refers to a entity number we dont have (i.e. ourself!)
  // no clue whose sound it could be just let it play
  if (cg_entityStates[entity_num].number == -1) return 0;

  return cg_entityStates[entity_num].clientNum != getSnap()->ps.clientNum;
}
