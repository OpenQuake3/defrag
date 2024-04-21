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

#include "cg_local.h"
#include "../qcommon/q_assert.h"

#include <ctype.h>
#include <stdlib.h>

char const* VectorParse(char const* data, vec_t* vec, uint8_t size) {
  assert(data); assert(vec); ASSERT_GT(size, 0);
  char* end;
  for (uint8_t i = 0; i < size; ++i) {
    vec[i] = strtof(data, &end);
    assert(data != end);
    data = end;
  }
  return data;
}

char const* VectorParse4(char const* data, vec4_t* vec, uint8_t size) {
  assert(data); assert(vec); ASSERT_GT(size, 0);
  data = VectorParse(data, vec[0], ARRAY_LEN(*vec));
  for (uint8_t i = 1; i < size; ++i) {
    while (*data++ != '/') assert(data[-1] != '\0');
    data = VectorParse(data, vec[i], ARRAY_LEN(*vec));
  }
  return data;
}

int32_t cvar_getInteger(char const* var_name) {
  char buffer[MAX_CVAR_VALUE_STRING];
  trap_Cvar_VariableStringBuffer(var_name, buffer, sizeof(buffer));
  char const* s    = buffer;
  int8_t      sign = 1;
  while (isspace(*s)) { ++s; }
  if      (*s == '-') { sign = -1; ++s; }
  else if (*s == '+') { ++s; }
  if (*s == '0' && (s[1] == 'b' || s[1] == 'B')) {
    return sign * strtol(s + 2, NULL, 2);
  }
  return sign * strtol(s, NULL, 0);
}

float cvar_getValue(char const* var_name) {
  char buffer[MAX_CVAR_VALUE_STRING];
  trap_Cvar_VariableStringBuffer(var_name, buffer, sizeof(buffer));
  return strtof(buffer, NULL);
}

void cvartable_init(cvarTable_t const* cvartable, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    trap_Cvar_Register(cvartable[i].vmCvar, cvartable[i].cvarName, cvartable[i].defaultString, cvartable[i].cvarFlags);
  }
}

void cvartable_update(cvarTable_t const* cvartable, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    trap_Cvar_Update(cvartable[i].vmCvar);
  }
}

void cvar_help(char const* cvarName) {
  for (size_t i = 0; i < helpTableIdx; ++i) {
    for (size_t j = 0; j < helpTable[i].size; ++j) {
      help_t const* help = &helpTable[i].help[j];
      if (Q_stricmp(cvarName, help->cvarTable->cvarName)) {continue;}

      assert(help->message);
      cvar_help_prep(help->kind, help->cvarTable->defaultString);
      for (size_t k = 0; k < ARRAY_LEN(help->message); ++k) {
        char const* message = help->message[k];
        if (!message) {break;}
        trap_Print(vaf("^3%s^7\n", message));
      }
      cvar_help_post(help->kind);
      return;
    }
  }
  cvartable_with_help();
}

void cvartable_with_help(void) {
  trap_Print("^3There is help for the cvartable:^7\n");
  for (size_t i = 0; i < ARRAY_LEN(helpTable); ++i) {
    for (size_t j = 0; j < helpTable[i].size; ++j) {
      help_t const* help = &helpTable[i].help[j];
      trap_Print(vaf("  ^2%s^7\n", help->cvarTable->cvarName));
    }
  }
}

static void cvar_help_prep(cvarKind_t kind, char const* defaultString) {
  ASSERT_GT(kind, 0);
  if (kind < 16) {
    cvarKind_t n     = kind;
    uint8_t    count = 0;
    while(n) { count += n & 1; n >>= 1; }
    ASSERT_GE(count, 1);
    ASSERT_LE(count, 4);
    trap_Print(vaf("^2This cvar accepts %u %s:^7\n", count, count == 1 ? "number" : "numbers"));

    float const x = cgs.glconfig.vidWidth / cgs.screenXScale;
    float const y = cgs.glconfig.vidHeight / cgs.screenXScale;
    if (kind & X) trap_Print(vaf("  ^3x^2-coordinate [0,%.0f]^7\n", x));
    if (kind & Y) trap_Print(vaf("  ^3y^2-coordinate [0,%.0f]^7\n", y));
    if (kind & W) trap_Print(vaf("  ^3w^2idth        [0,%.0f]^7\n", x));
    if (kind & H) trap_Print(vaf("  ^3h^2eight       [0,%.0f]^7\n", y));
    return;
  }
  ASSERT_GE(kind, 16);
  switch (kind) {
  case BINARY_LITERAL:
    trap_Print("^2This cvar can accept binary-literals - a sequence starting^7\n");
    trap_Print(vaf("^2with ^30b^2 followed by ^31^2's and ^30^2's (e.g. ^3%s^2).^7\n", defaultString));
    trap_Print("^2Enable/disable items by replacing ^3X^2's with ^31^2/^30^2 resp.\n");
    return;
  case RGBA:
    trap_Print("^2This cvar accepts a set of 4 numbers:^7\n");
    trap_Print("  ^3r^2ed   [0,1]^7\n");
    trap_Print("  ^3g^2reen [0,1]^7\n");
    trap_Print("  ^3b^2lue  [0,1]^7\n");
    trap_Print("  ^3a^2lpha [0,1]^7\n");
    return;
  case RGBAS:
    trap_Print("^2This cvar accepts sets of 4 numbers:^7\n");
    trap_Print("  ^3r^2ed   [0,1]^7\n");
    trap_Print("  ^3g^2reen [0,1]^7\n");
    trap_Print("  ^3b^2lue  [0,1]^7\n");
    trap_Print("  ^3a^2lpha [0,1]^7\n");
    return;
  default:
    assert(0);
    return;
  }
}

static void cvar_help_post(cvarKind_t kind) {
  ASSERT_GT(kind, 0);
  if (kind < 16) { return; }
  ASSERT_GE(kind, 16);
  switch (kind) {
  case BINARY_LITERAL: return;// trap_Print("^2Examples: github.com/Jelvan1/cgame_proxymod#examples^7\n"); return;
  case RGBA:
  case RGBAS: return;
  default: assert(0); return;
  }
}
