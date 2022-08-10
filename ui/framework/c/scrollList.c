#include "../elements.h"

//:::::::::::::::::::::::
// ScrollList_Init
//:::::::::::::::::::::::
void scrollList_init(MenuList* l) {
  l->oldvalue = 0;
  l->curvalue = 0;
  l->top      = 0;
  if (!l->columns) {
    l->columns    = 1;
    l->separation = 0;
  } else if (!l->separation) {
    l->separation = 3;
  }
  int w             = ((l->width + l->separation) * l->columns - l->separation) * SMALLCHAR_WIDTH;
  l->generic.left   = l->generic.x;
  l->generic.top    = l->generic.y;
  l->generic.right  = l->generic.x + w;
  l->generic.bottom = l->generic.y + l->height * SMALLCHAR_HEIGHT;
  if (l->generic.flags & MFL_CENTER_JUSTIFY) {
    l->generic.left -= w / 2;
    l->generic.right -= w / 2;
  }
}

//:::::::::::::::::::::::
// ScrollList_Key
//:::::::::::::::::::::::
sfxHandle_t scrollList_key(MenuList* l, int key) {
  int x, y, w;
  int i, j, c;
  int cursorx, cursory;
  int index, column, scroll;
  switch (key) {
    case K_MOUSE1:
      if (!(l->generic.flags & MFL_HASMOUSEFOCUS)) { break; }
      // check scroll region
      x = l->generic.x;
      y = l->generic.y;
      w = ((l->width + l->separation) * l->columns - l->separation) * SMALLCHAR_WIDTH;
      if (l->generic.flags & MFL_CENTER_JUSTIFY) { x -= w / 2; }
      if (!cursorInRect(x, y, w, l->height * SMALLCHAR_HEIGHT)) { return (q3sound.menu_null); }  // absorbed, silent sound effect
      cursorx = (uis.cursorx - x) / SMALLCHAR_WIDTH;
      column  = cursorx / (l->width + l->separation);
      cursory = (uis.cursory - y) / SMALLCHAR_HEIGHT;
      index   = column * l->height + cursory;
      if (l->top + index < l->numitems) {
        l->oldvalue = l->curvalue;
        l->curvalue = l->top + index;
        if (l->oldvalue != l->curvalue && l->generic.callback) {
          l->generic.callback(l, MS_GOTFOCUS);
          return (q3sound.menu_move);
        }
      }

    case K_KP_HOME:
    case K_HOME:
      l->oldvalue = l->curvalue;
      l->curvalue = 0;
      l->top      = 0;
      if (!(l->oldvalue != l->curvalue && l->generic.callback)) { return (q3sound.menu_buzz); }
      l->generic.callback(l, MS_GOTFOCUS);
      return (q3sound.menu_move);

    case K_KP_END:
    case K_END:
      l->oldvalue = l->curvalue;
      l->curvalue = l->numitems - 1;
      if (l->columns > 1) {
        c      = (l->curvalue / l->height + 1) * l->height;
        l->top = c - (l->columns * l->height);
      } else {
        l->top = l->curvalue - (l->height - 1);
      }
      if (l->top < 0) l->top = 0;
      if (l->oldvalue != l->curvalue && l->generic.callback) {
        l->generic.callback(l, MS_GOTFOCUS);
        return (q3sound.menu_move);
      }
      return (q3sound.menu_buzz);

    case K_PGUP:
    case K_KP_PGUP:
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->curvalue > 0)) { return (q3sound.menu_buzz); }
      l->oldvalue = l->curvalue;
      l->curvalue -= l->height - 1;
      if (l->curvalue < 0) l->curvalue = 0;
      l->top = l->curvalue;
      if (l->top < 0) l->top = 0;
      if (l->generic.callback) l->generic.callback(l, MS_GOTFOCUS);
      return (q3sound.menu_move);

    case K_PGDN:
    case K_KP_PGDN:
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->curvalue < l->numitems - 1)) { return (q3sound.menu_buzz); }
      l->oldvalue = l->curvalue;
      l->curvalue += l->height - 1;
      if (l->curvalue > l->numitems - 1) l->curvalue = l->numitems - 1;
      l->top = l->curvalue - (l->height - 1);
      if (l->top < 0) l->top = 0;
      if (l->generic.callback) l->generic.callback(l, MS_GOTFOCUS);
      return (q3sound.menu_move);

    case K_MWHEELUP:
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->top > 0)) { return (q3sound.menu_buzz); }
      // if scrolling 3 lines would replace over half of the displayed items, only scroll 1 item at a time.
      scroll = l->height < 6 ? 1 : 3;
      l->top -= scroll;
      if (l->top < 0) l->top = 0;
      if (l->generic.callback) l->generic.callback(l, MS_GOTFOCUS);
      return (q3sound.menu_null);  // make scrolling silent

    case K_MWHEELDOWN:
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->top < l->numitems - l->height)) { return (q3sound.menu_buzz); }
      // if scrolling 3 items would replace over half of the displayed items, only scroll 1 item at a time.
      scroll = l->height < 6 ? 1 : 3;
      l->top += scroll;
      if (l->top > l->numitems - l->height) l->top = l->numitems - l->height;
      if (l->generic.callback) l->generic.callback(l, MS_GOTFOCUS);
      return (q3sound.menu_null);  // make scrolling silent

    case K_KP_UPARROW:
    case K_UPARROW:
      if (l->curvalue == 0) { return q3sound.menu_buzz; }
      l->oldvalue = l->curvalue;
      l->curvalue--;
      if (l->curvalue < l->top) {
        if (l->columns == 1) {  // TODO: Untangle this
          l->top--;
        } else {
          l->top -= l->height;
        }
      }
      if (l->generic.callback) { l->generic.callback(l, MS_GOTFOCUS); }
      return (q3sound.menu_move);

    case K_KP_DOWNARROW:
    case K_DOWNARROW:
      if (l->curvalue == l->numitems - 1) { return q3sound.menu_buzz; }
      l->oldvalue = l->curvalue;
      l->curvalue++;
      if (l->curvalue >= l->top + l->columns * l->height) {
        if (l->columns == 1) {
          l->top++;
        } else {
          l->top += l->height;
        }
      }
      if (l->generic.callback) { l->generic.callback(l, MS_GOTFOCUS); }
      return q3sound.menu_move;

    case K_KP_LEFTARROW:
    case K_LEFTARROW:
      if (l->columns == 1) { return q3sound.menu_null; }
      if (l->curvalue < l->height) { return q3sound.menu_buzz; }
      l->oldvalue = l->curvalue;
      l->curvalue -= l->height;
      if (l->curvalue < l->top) { l->top -= l->height; }
      if (l->generic.callback) { l->generic.callback(l, MS_GOTFOCUS); }
      return q3sound.menu_move;

    case K_KP_RIGHTARROW:
    case K_RIGHTARROW:
      if (l->columns == 1) { return q3sound.menu_null; }
      c = l->curvalue + l->height;
      if (c >= l->numitems) { return q3sound.menu_buzz; }
      l->oldvalue = l->curvalue;
      l->curvalue = c;
      if (l->curvalue > l->top + l->columns * l->height - 1) { l->top += l->height; }
      if (l->generic.callback) { l->generic.callback(l, MS_GOTFOCUS); }
      return q3sound.menu_move;
  }
  // cycle look for ascii key inside list items
  if (!Q_isprint(key)) { return (0); }
  // force to lower for case insensitive compare
  if (Q_isupper(key)) { key -= 'A' - 'a'; }
  // iterate list items
  for (i = 1; i <= l->numitems; i++) {
    j = (l->curvalue + i) % l->numitems;
    c = l->itemnames[j][0];
    if (Q_isupper(c)) { c -= 'A' - 'a'; }
    if (c == key) {
      // set current item, mimic windows listbox scroll behavior
      if (j < l->top) {
        // behind top most item, set this as new top
        l->top = j;
      } else if (j > l->top + l->height - 1) {
        // past end of list box, do page down
        l->top = (j + 1) - l->height;
      }
      if (l->curvalue != j) {
        l->oldvalue = l->curvalue;
        l->curvalue = j;
        if (l->generic.callback) l->generic.callback(l, MS_GOTFOCUS);
        return (q3sound.menu_move);
      }
      return (q3sound.menu_buzz);
    }
  }
  return (q3sound.menu_buzz);
}

// ScrollList_Draw
void scrollList_draw(MenuList* l) {
  float* color;
  bool   hasfocus = (l->generic.parent->cursor == l->generic.menuPosition);
  int    x        = l->generic.x;
  for (int column = 0; column < l->columns; column++) {
    int style = UI_LEFT | UI_SMALLFONT;
    int y     = l->generic.y;
    int base  = l->top + column * l->height;
    for (int i = base; i < base + l->height; i++) {
      if (i >= l->numitems) { break; }
      if (i == l->curvalue) {
        int u = x - 2;
        if (l->generic.flags & MFL_CENTER_JUSTIFY) { u -= (l->width * SMALLCHAR_WIDTH) / 2 + 1; }
        uiFillRect(u, y, l->width * SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT + 2, (vec_t*)q3color.listbar);
        color = (vec_t*)q3color.text_highlight;
        if (hasfocus) { style |= UI_PULSE; }
      } else {
        color = (vec_t*)q3color.text_normal;
      }
      if (l->generic.flags & MFL_CENTER_JUSTIFY) { style |= UI_CENTER; }
      uiDrawString(x, y, l->itemnames[i], style, color);
      y += SMALLCHAR_HEIGHT;
    }
    x += (l->width + l->separation) * SMALLCHAR_WIDTH;
  }
}
