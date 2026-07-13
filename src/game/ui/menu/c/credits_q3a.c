
#include "../../local.h"

//:::::::::::::::::
typedef struct {
  MenuFw menu;
  int    frame;
} Credits;
//:::::::::::::::::
static Credits s_credits;
//:::::::::::::::::

//:::::::::::::::::
// UI_CreditMenu_Draw_ioq3
//:::::::::::::::::
static void menuCredits_draw_ioq3(void) {
  // People that made commits to Subversion. Probably incomplete. (alphabetical order)
  const char* names[] = { "Tim Angus",
                          "James Canete",
                          "Vincent Cojot",
                          "Ryan C. Gordon",
                          "Aaron Gyes",
                          "Zack Middleton",
                          "Ludwig Nussel",
                          "Julian Priestley",
                          "Scirocco Six",
                          "Thilo Schulz",
                          "Zachary J. Slater",
                          "Tony J. White",
                          "...and many, many others!",  // keep this one last.
                          NULL };

  // Center text vertically on the screen
  int y               = (SCREEN_HEIGHT - ARRAY_LEN(names) * (1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE)) / 2;

  uiDrawPString(320, y, "ioquake3 contributors:", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;

  for (int i = 0; names[i]; i++) {
    uiDrawPString(320, y, names[i], UI_CENTER | UI_SMALLFONT, (vec_t*)q3color.text_menu);
    y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  }
  uiDrawPString(320, 459, "http://www.ioquake3.org/", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.key);
}

//:::::::::::::::::
// UI_CreditMenu_Key
//:::::::::::::::::
static sfxHandle_t menuCredits_key(int key) {
  if (key & K_CHAR_FLAG) { return 0; }
  s_credits.frame++;
  if (s_credits.frame == 1) {
    s_credits.menu.draw = menuCredits_draw_ioq3;
  } else {
    // TODO: Change to go back to StartMenu
    id3Cmd_ExecuteText(EXEC_APPEND, "quit\n");
  }
  return 0;
}

//:::::::::::::::::
// UI_CreditMenu_Draw
//:::::::::::::::::
static void menuCredits_draw(void) {
  int y = 12;
  uiDrawPString(320, y, "id Software is:", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Programming", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "John Carmack, Robert A. Duffy, Jim Dose'", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Art", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Adrian Carmack, Kevin Cloud,", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Kenneth Scott, Seneca Menard, Fred Nilsson", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Game Designer", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Graeme Devine", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Level Design", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Tim Willits, Christian Antkow, Paul Jaquays", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "CEO", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Todd Hollenshead", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Director of Business Development", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Marty Stratton", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Biz Assist and id Mom", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Donna Jackson", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.42 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Development Assistance", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);
  y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawPString(320, y, "Eric Webb", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.fg);

  y += 1.35 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
  uiDrawString(320, y, "To order: 1-800-idgames     www.quake3arena.com     www.idsoftware.com", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.key);
  y += SMALLCHAR_HEIGHT;
  uiDrawString(320, y, "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved", UI_CENTER | UI_SMALLFONT, (vec_t*)q3credits.key);
}

//:::::::::::::::::
// UI_CreditMenu
//:::::::::::::::::
void menuCredits(void) {
  memset(&s_credits, 0, sizeof(s_credits));
  s_credits.menu.draw       = menuCredits_draw;
  s_credits.menu.key        = menuCredits_key;
  s_credits.menu.fullscreen = qtrue;
  menuPush(&s_credits.menu);
}
