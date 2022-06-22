/*
  ==============================
  Written by:
    id software :            Quake III Arena
    nightmare, hk, Jelvan1 : mdd cgame Proxymod
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
#ifndef HUD_LOCAL_H  // Originally cg_hud.h
#define HUD_LOCAL_H

// #include <stdint.h>

// stats[13] fields
// TODO: remove this
#define PSF_USERINPUT_NONE     0
#define PSF_USERINPUT_FORWARD  1
#define PSF_USERINPUT_BACKWARD 2
#define PSF_USERINPUT_LEFT     8
#define PSF_USERINPUT_RIGHT    16
#define PSF_USERINPUT_JUMP     32
#define PSF_USERINPUT_CROUCH   64
#define PSF_USERINPUT_ATTACK   256
#define PSF_USERINPUT_WALK     512

// Core methods  ||  Originally cg_hud.c
void hud_init(void);
void hud_term(void);
void hud_update(void);
void hud_draw(void);

// Accel methods  ||  Originally cg_cgaz.h
void hud_accel_init(void);
void hud_accel_update(void);
void hud_accel_draw(void);

// Snap methods  ||  Originally cg_snap.h


// Help Methods  ||  Originally help.h
#include "../cg_local.h"

void hud_help_init(help_t const* help, size_t size);
void hud_help_term(void);

#endif // HUD_LOCAL_H
