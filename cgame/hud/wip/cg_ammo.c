#include "cg_ammo.h"

#include "cg_cvar.h"
#include "cg_draw.h"
#include "cg_local.h"
#include "cg_utils.h"
#include "help.h"

static vmCvar_t ammo;
static vmCvar_t hud_ammo_graph_xywh;
static vmCvar_t hud_ammo_text_xh;
static vmCvar_t hud_ammo_text_rgba;

static cvarTable_t hud_ammo_cvartable[] = {
  { &ammo, "hud_ammo", "0b0011", CVAR_ARCHIVE_ND },
  { &hud_ammo_graph_xywh, "hud_ammo_graph_xywh", "610 100 24 24", CVAR_ARCHIVE_ND },
  { &hud_ammo_text_xh, "hud_ammo_text_xh", "6 12", CVAR_ARCHIVE_ND },
  { &hud_ammo_text_rgba, "hud_ammo_text_rgba", "1 1 1 1", CVAR_ARCHIVE_ND },
};

static help_t hud_ammo_help[] = {
  {
    hud_ammo_cvartable + 0,
    BINARY_LITERAL,
    {
      "hud_ammo 0bXXXX",
      "           ||||",
      "           |||+- draw hud",
      "           ||+-- show weaponless ammo",
      "           |+--- use gun icons",
      "           +---- use 3D models",
    },
  },
#define AMMO_DRAW           1
#define AMMO_WEAPONLESSAMMO 2
#define AMMO_GUN            4
#define AMMO_3D             8
  {
    hud_ammo_cvartable + 1,
    X | Y | W | H,
    {
      "hud_ammo_graph_xywh X X X X",
    },
  },
  {
    hud_ammo_cvartable + 2,
    X | H,
    {
      "hud_ammo_text_xh X X",
    },
  },
  {
    hud_ammo_cvartable + 3,
    RGBA,
    {
      "hud_ammo_text_rgba X X X X",
    },
  },
};

typedef struct
{
  qhandle_t graph_icons[16];
  qhandle_t graph_models[16];
  vec3_t    graph_model_origin;
  vec3_t    graph_model_angles;

  vec4_t graph_xywh;
  vec2_t text_xh;

  vec4_t text_rgba;
} hud_ammo_t;

static hud_ammo_t hud_ammo_;

void hud_ammo_init(void)
{
  cvartable_init(hud_ammo_cvartable, ARRAY_LEN(hud_ammo_cvartable));
  hud_help_init(hud_ammo_help, ARRAY_LEN(hud_ammo_help));

  hud_ammo_.graph_icons[0]  = trap_R_RegisterShader("icons/icona_machinegun");
  hud_ammo_.graph_icons[1]  = trap_R_RegisterShader("icons/icona_shotgun");
  hud_ammo_.graph_icons[2]  = trap_R_RegisterShader("icons/icona_grenade");
  hud_ammo_.graph_icons[3]  = trap_R_RegisterShader("icons/icona_rocket");
  hud_ammo_.graph_icons[4]  = trap_R_RegisterShader("icons/icona_lightning");
  hud_ammo_.graph_icons[5]  = trap_R_RegisterShader("icons/icona_railgun");
  hud_ammo_.graph_icons[6]  = trap_R_RegisterShader("icons/icona_plasma");
  hud_ammo_.graph_icons[7]  = trap_R_RegisterShader("icons/icona_bfg");
  hud_ammo_.graph_icons[8]  = trap_R_RegisterShader("icons/iconw_machinegun");
  hud_ammo_.graph_icons[9]  = trap_R_RegisterShader("icons/iconw_shotgun");
  hud_ammo_.graph_icons[10] = trap_R_RegisterShader("icons/iconw_grenade");
  hud_ammo_.graph_icons[11] = trap_R_RegisterShader("icons/iconw_rocket");
  hud_ammo_.graph_icons[12] = trap_R_RegisterShader("icons/iconw_lightning");
  hud_ammo_.graph_icons[13] = trap_R_RegisterShader("icons/iconw_railgun");
  hud_ammo_.graph_icons[14] = trap_R_RegisterShader("icons/iconw_plasma");
  hud_ammo_.graph_icons[15] = trap_R_RegisterShader("icons/iconw_bfg");

  hud_ammo_.graph_models[0]  = trap_R_RegisterModel("models/powerups/ammo/machinegunam.md3");
  hud_ammo_.graph_models[1]  = trap_R_RegisterModel("models/powerups/ammo/shotgunam.md3");
  hud_ammo_.graph_models[2]  = trap_R_RegisterModel("models/powerups/ammo/grenadeam.md3");
  hud_ammo_.graph_models[3]  = trap_R_RegisterModel("models/powerups/ammo/rocketam.md3");
  hud_ammo_.graph_models[4]  = trap_R_RegisterModel("models/powerups/ammo/lightningam.md3");
  hud_ammo_.graph_models[5]  = trap_R_RegisterModel("models/powerups/ammo/railgunam.md3");
  hud_ammo_.graph_models[6]  = trap_R_RegisterModel("models/powerups/ammo/plasmaam.md3");
  hud_ammo_.graph_models[7]  = trap_R_RegisterModel("models/powerups/ammo/bfgam.md3");
  hud_ammo_.graph_models[8]  = trap_R_RegisterModel("models/weapons2/machinegun/machinegun.md3");
  hud_ammo_.graph_models[9]  = trap_R_RegisterModel("models/weapons2/shotgun/shotgun.md3");
  hud_ammo_.graph_models[10] = trap_R_RegisterModel("models/weapons2/grenadel/grenadel.md3");
  hud_ammo_.graph_models[11] = trap_R_RegisterModel("models/weapons2/rocketl/rocketl.md3");
  hud_ammo_.graph_models[12] = trap_R_RegisterModel("models/weapons2/lightning/lightning.md3");
  hud_ammo_.graph_models[13] = trap_R_RegisterModel("models/weapons2/railgun/railgun.md3");
  hud_ammo_.graph_models[14] = trap_R_RegisterModel("models/weapons2/plasma/plasma.md3");
  hud_ammo_.graph_models[15] = trap_R_RegisterModel("models/weapons2/bfg/bfg.md3");

  memset(hud_ammo_.graph_model_origin, 0, 3 * sizeof(vec_t));
  hud_ammo_.graph_model_origin[0] = 70.f;
  memset(hud_ammo_.graph_model_angles, 0, 3 * sizeof(vec_t));
}

void hud_ammo_update(void)
{
  cvartable_update(hud_ammo_cvartable, ARRAY_LEN(hud_ammo_cvartable));
  ammo.integer = cvar_getInteger("hud_ammo");
}

void hud_ammo_draw(void)
{
  if (!(ammo.integer & AMMO_DRAW)) return;

  VectorParse(hud_ammo_graph_xywh.string, hud_ammo_.graph_xywh, 4);
  VectorParse(hud_ammo_text_rgba.string, hud_ammo_.text_rgba, 4);

  float                      y  = hud_ammo_.graph_xywh[1];
  playerState_t const* const ps = getPs();
  for (uint8_t i = 0; i < 8; ++i)
  {
    int32_t const  ammoLeft  = ps->ammo[i + 2];
    qboolean const hasWeapon = ps->stats[STAT_WEAPONS] & (1 << (i + 2));

    if (!hasWeapon && (!(ammo.integer & AMMO_WEAPONLESSAMMO) || !ammoLeft)) continue;

    if (!(ammo.integer & AMMO_3D))
    {
      CG_DrawPic(
        hud_ammo_.graph_xywh[0],
        y,
        hud_ammo_.graph_xywh[2],
        hud_ammo_.graph_xywh[3],
        hud_ammo_.graph_icons[i + (ammo.integer & AMMO_GUN ? 8 : 0)]);
    }
    else
    {
      hud_ammo_.graph_model_angles[YAW] = 90.f + 20.f * sinf(getSnap()->serverTime / 1000.f);
      CG_Draw3DModel(
        hud_ammo_.graph_xywh[0],
        y,
        hud_ammo_.graph_xywh[2],
        hud_ammo_.graph_xywh[3],
        hud_ammo_.graph_models[i + (ammo.integer & AMMO_GUN ? 8 : 0)],
        0,
        hud_ammo_.graph_model_origin,
        hud_ammo_.graph_model_angles);
    }

    if (!hasWeapon) // Mark weapon as unavailable
    {
      CG_DrawPic(hud_ammo_.graph_xywh[0], y, hud_ammo_.graph_xywh[2], hud_ammo_.graph_xywh[3], cgs.media.deferShader);
    }

    VectorParse(hud_ammo_text_xh.string, hud_ammo_.text_xh, 2);
    qboolean const alignRight = hud_ammo_.graph_xywh[0] + hud_ammo_.graph_xywh[2] / 2.f > cgs.screenWidth / 2;
    CG_DrawText(
      alignRight ? hud_ammo_.graph_xywh[0] - hud_ammo_.text_xh[0]
                 : hud_ammo_.graph_xywh[0] + hud_ammo_.graph_xywh[2] + hud_ammo_.text_xh[0],
      y + hud_ammo_.graph_xywh[3] / 2.f - .5f * hud_ammo_.text_xh[1],
      hud_ammo_.text_xh[1],
      vaf("%i", ammoLeft),
      hud_ammo_.text_rgba,
      alignRight,
      qtrue /*shadow*/);
    y += hud_ammo_.graph_xywh[3];
  }
}
