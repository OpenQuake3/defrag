#include "../elements.h"
#include "../../local.h"

//:::::::::::::::::::::::
static void MField_charEvent(Field*, int);
static void MField_clear(Field*);
//:::::::::::::::::::::::

//:::::::::::::::::::::::
// MField_Paste
//:::::::::::::::::::::::
static void MField_paste(Field* edit) {
  char pasteBuffer[64];
  id3GetClipboardData(pasteBuffer, 64);
  // send as if typed, so insert / overstrike works properly
  int pasteLen = strlen(pasteBuffer);
  for (int i = 0; i < pasteLen; i++) { MField_charEvent(edit, pasteBuffer[i]); }
}

//:::::::::::::::::::::::
// MField_CharEvent
//:::::::::::::::::::::::
static void MField_charEvent(Field* edit, int ch) {
  if (ch == 'v' - 'a' + 1) {  // ctrl-v is paste
    MField_paste(edit);
    return;
  }
  if (ch == 'c' - 'a' + 1) {  // ctrl-c clears the field
    MField_clear(edit);
    return;
  }

  int len = strlen(edit->buffer);
  if (ch == 'h' - 'a' + 1) {  // ctrl-h is backspace
    if (!(edit->cursor > 0)) { return; }
    memmove(edit->buffer + edit->cursor - 1, edit->buffer + edit->cursor, len + 1 - edit->cursor);
    edit->cursor--;
    if (edit->cursor < edit->scroll) { edit->scroll--; }
  }

  if (ch == 'a' - 'a' + 1) {  // ctrl-a is home
    edit->cursor = 0;
    edit->scroll = 0;
    return;
  }
  if (ch == 'e' - 'a' + 1) {  // ctrl-e is end
    edit->cursor = len;
    edit->scroll = edit->cursor - edit->widthInChars + 1;
    if (edit->scroll < 0) edit->scroll = 0;
    return;
  }

  //
  // ignore any other non printable chars
  if (ch < 32) { return; }
  if (id3Key_GetOverstrikeMode()) {
    if ((edit->cursor == MAX_EDIT_LINE - 1) || (edit->maxchars && edit->cursor >= edit->maxchars)) { return; }
  } else {
    // insert mode
    if ((len == MAX_EDIT_LINE - 1) || (edit->maxchars && len >= edit->maxchars)) { return; }
    memmove(edit->buffer + edit->cursor + 1, edit->buffer + edit->cursor, len + 1 - edit->cursor);
  }
  edit->buffer[edit->cursor] = ch;
  if (!edit->maxchars || edit->cursor < edit->maxchars - 1) edit->cursor++;
  if (edit->cursor >= edit->widthInChars) { edit->scroll++; }
  if (edit->cursor == len + 1) { edit->buffer[edit->cursor] = 0; }
}

//:::::::::::::::::::::::
// MField_Draw
//   Handles horizontal scrolling and cursor blinking
//   x, y, are in pixels
//:::::::::::::::::::::::
static void MField_draw(Field* edit, int x, int y, int style, vec4_t color) {
  int drawLen = edit->widthInChars;
  int len     = strlen(edit->buffer) + 1;
  // guarantee that cursor will be visible
  int prestep;
  if (len <= drawLen) {
    prestep = 0;
  } else {
    if (edit->scroll + drawLen > len) {
      edit->scroll = len - drawLen;
      if (edit->scroll < 0) { edit->scroll = 0; }
    }
    prestep = edit->scroll;
  }
  if (prestep + drawLen > len) { drawLen = len - prestep; }

  // extract <drawLen> characters from the field at <prestep>
  if (drawLen >= MAX_STRING_CHARS) { id3Error("drawLen >= MAX_STRING_CHARS"); }
  char str[MAX_STRING_CHARS];
  memcpy(str, edit->buffer + prestep, drawLen);
  str[drawLen] = 0;

  uiDrawString(x, y, str, style, color);

  // draw the cursor
  if (!(style & UI_PULSE)) { return; }
  int cursorChar = (id3Key_GetOverstrikeMode()) ? 11 : 10;

  style &= ~UI_PULSE;
  style |= UI_BLINK;
  int charw = (style & UI_SMALLFONT) ? SMALLCHAR_WIDTH : BIGCHAR_WIDTH;
  charw     = (style & UI_GIANTFONT) ? GIANTCHAR_WIDTH : charw;
  if (style & UI_CENTER) {
    len = strlen(str);
    x   = x - len * charw / 2;
  } else if (style & UI_RIGHT) {
    len = strlen(str);
    x   = x - len * charw;
  }
  uiDrawChar(x + (edit->cursor - prestep) * charw, y, cursorChar, style & ~(UI_CENTER | UI_RIGHT), color);
}

//:::::::::::::::::::::::
// MField_KeyDownEvent
//   Performs the basic line editing functions for the console, in-game talk, and menu fields
//   Key events are used for non-printable characters, others are gotten from char events.
//:::::::::::::::::::::::
static void MField_keyDownEvent(Field* edit, int key) {
  // shift-insert is paste
  if (((key == K_INS) || (key == K_KP_INS)) && id3Key_IsDown(K_SHIFT)) {
    MField_paste(edit);
    return;
  }
  int len = strlen(edit->buffer);

  if (key == K_DEL || key == K_KP_DEL) {
    if (edit->cursor < len) { memmove(edit->buffer + edit->cursor, edit->buffer + edit->cursor + 1, len - edit->cursor); }
    return;
  }

  if (key == K_RIGHTARROW || key == K_KP_RIGHTARROW) {
    if (edit->cursor < len) { edit->cursor++; }
    if (edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= len) { edit->scroll++; }
    return;
  }

  if (key == K_LEFTARROW || key == K_KP_LEFTARROW) {
    if (edit->cursor > 0) { edit->cursor--; }
    if (edit->cursor < edit->scroll) { edit->scroll--; }
    return;
  }

  if (key == K_HOME || key == K_KP_HOME || (tolower(key) == 'a' && id3Key_IsDown(K_CTRL))) {
    edit->cursor = 0;
    edit->scroll = 0;
    return;
  }

  if (key == K_END || key == K_KP_END || (tolower(key) == 'e' && id3Key_IsDown(K_CTRL))) {
    edit->cursor = len;
    edit->scroll = len - edit->widthInChars + 1;
    if (edit->scroll < 0) edit->scroll = 0;
    return;
  }

  if (key == K_INS || key == K_KP_INS) {
    id3Key_SetOverstrikeMode(!id3Key_GetOverstrikeMode());
    return;
  }
}
//:::::::::::::::::::::::
// MField_clear
//:::::::::::::::::::::::
static void MField_clear(Field* edit) {
  edit->buffer[0] = 0;
  edit->cursor    = 0;
  edit->scroll    = 0;
}

//:::::::::::::::::::::::
// MenuField_Init
//:::::::::::::::::::::::
void menuField_init(MenuField* m) {
  MField_clear(&m->field);
  bool smallfont    = (m->generic.flags & MFL_SMALLFONT);
  int  w            = (smallfont) ? SMALLCHAR_WIDTH : BIGCHAR_WIDTH;
  int  h            = (smallfont) ? SMALLCHAR_HEIGHT : BIGCHAR_HEIGHT;
  int  l            = (m->generic.name) ? (strlen(m->generic.name) + 1) * w : 0;
  m->generic.left   = m->generic.x - l;
  m->generic.top    = m->generic.y;
  m->generic.right  = m->generic.x + w + m->field.widthInChars * w;
  m->generic.bottom = m->generic.y + h;
}

//:::::::::::::::::::::::
// MenuField_Draw
//:::::::::::::::::::::::
void menuField_draw(MenuField* f) {
  int    x         = f->generic.x;
  int    y         = f->generic.y;
  bool   smallfont = (f->generic.flags & MFL_SMALLFONT);
  int    w         = (smallfont) ? SMALLCHAR_WIDTH : BIGCHAR_WIDTH;
  int    style     = (smallfont) ? UI_SMALLFONT : UI_BIGFONT;
  bool   grayed    = (f->generic.flags & MFL_GRAYED);
  float* color     = (grayed) ? (vec_t*)q3color.text_disabled : (vec_t*)q3color.text_normal;
  bool   focus     = (cursorGetItem(f->generic.parent) == f);
  if (focus) {
    color = (vec_t*)q3color.text_highlight;
    style |= UI_PULSE;
    // draw cursor
    uiFillRect(f->generic.left,
               f->generic.top,
               f->generic.right - f->generic.left + 1,
               f->generic.bottom - f->generic.top + 1,
               (vec_t*)q3color.listbar);
    uiDrawChar(x, y, 13, UI_CENTER | UI_BLINK | style, color);
  }
  if (f->generic.name) { uiDrawString(x - w, y, f->generic.name, style | UI_RIGHT, color); }
  MField_draw(&f->field, x + w, y, style, color);
}

//:::::::::::::::::::::::
// MenuField_Key
//:::::::::::::::::::::::
sfxHandle_t menuField_key(MenuField* m, int* key) {
  int keycode = *key;
  switch (keycode) {
    case K_KP_ENTER:
    case K_ENTER:
    case K_JOY1:
    case K_JOY2:
    case K_JOY3:
    case K_JOY4: *key = K_TAB; break;  // have enter go to next cursor point

    case K_TAB:
    case K_KP_DOWNARROW:
    case K_DOWNARROW:
    case K_KP_UPARROW:
    case K_UPARROW: break;

    default:
      if (!(keycode & K_CHAR_FLAG)) {
        MField_keyDownEvent(&m->field, keycode);
        break;
      }
      keycode &= ~K_CHAR_FLAG;
      if ((m->generic.flags & MFL_UPPERCASE) && Q_islower(keycode)) keycode -= 'a' - 'A';
      else if ((m->generic.flags & MFL_LOWERCASE) && Q_isupper(keycode)) keycode -= 'A' - 'a';
      else if ((m->generic.flags & MFL_NUMBERSONLY) && Q_isalpha(keycode)) return (q3sound.menu_buzz);
      MField_charEvent(&m->field, keycode);
      break;
  }
  return (0);
}
