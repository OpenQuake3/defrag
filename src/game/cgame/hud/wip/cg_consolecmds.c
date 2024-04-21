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
#include "cg_vm.h"
#include "help.h"

#include <stdlib.h>

static void cmdHelp(void);
#ifndef NDEBUG
static void cmdPointsTo_DebugOnly(void);
#endif

typedef struct
{
  char const* cmd;
  void (*function)(void);
} consoleCommand_t;

static consoleCommand_t commands[] = {
  { "hud_help", cmdHelp },
#ifndef NDEBUG
  { "hud_points_to", cmdPointsTo_DebugOnly },
#endif
};

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================*/
qboolean CG_ConsoleCommand(void)
{
  char cmd[MAX_STRING_CHARS];
  trap_Argv(0, cmd, sizeof(cmd));

  for (uint8_t i = 0; i < ARRAY_LEN(commands); ++i)
  {
    if (!Q_stricmp(cmd, commands[i].cmd))
    {
      commands[i].function();
      return qtrue;
    }
  }

  return qfalse;
}

/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================*/
void CG_InitConsoleCommands(void)
{
  for (uint8_t i = 0; i < ARRAY_LEN(commands); ++i)
  {
    trap_AddCommand(commands[i].cmd);
  }
}

static void cmdHelp(void)
{
  if (trap_Argc() != 2)
  {
    trap_Print("usage: hud_help <cvar>\n");
    cvartable_with_help();
    return;
  }

  char cvar[MAX_STRING_CHARS];
  trap_Argv(1, cvar, sizeof(cvar));

  cvar_help(cvar);
}

#ifndef NDEBUG
static void cmdPointsTo_DebugOnly(void)
{
  if (trap_Argc() != 2)
  {
    trap_Print("usage: hud_points_to <pointer>\n");
    return;
  }

  char cmd[MAX_STRING_CHARS];
  trap_Argv(1, cmd, sizeof(cmd));

  long const offset = strtol(cmd, NULL, 0);
  trap_Print(vaf("%s -> 0x%lx\n", cmd, *(int32_t*)VM_ArgPtr(offset)));
}
#endif
