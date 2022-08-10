#include "../elements.h"


//:::::::::::::::::::::::
// uiCursorInRect
//:::::::::::::::::::::::
bool cursorInRect(int x, int y, int width, int height) {
  if (uis.cursorx < x || uis.cursory < y || uis.cursorx > x + width || uis.cursory > y + height) return false;
  return true;
}
//:::::::::::::::::::::::
// cursorGetItem
//:::::::::::::::::::::::
void* cursorGetItem(MenuFw* m) {
  if (m->cursor < 0 || m->cursor >= m->nitems) { return NULL; }
  return m->items[m->cursor];
}
//:::::::::::::::::::
// Menu_CursorMoved
//:::::::::::::::::::
void cursorMoved(MenuFw* m) {
  void (*callback)(void* self, int notification);
  if (m->cursor_prev == m->cursor) return;
  if (m->cursor_prev >= 0 && m->cursor_prev < m->nitems) {
    callback = ((MenuCommon*)(m->items[m->cursor_prev]))->callback;
    if (callback) callback(m->items[m->cursor_prev], MS_LOSTFOCUS);
  }
  if (m->cursor >= 0 && m->cursor < m->nitems) {
    callback = ((MenuCommon*)(m->items[m->cursor]))->callback;
    if (callback) callback(m->items[m->cursor], MS_GOTFOCUS);
  }
}
//:::::::::::::::::::
// Menu_SetCursor
//:::::::::::::::::::
void cursorSet(MenuFw* m, int cursor) {
  if (((MenuCommon*)(m->items[cursor]))->flags & (MFL_GRAYED | MFL_INACTIVE)) { return; }  // cursor can't go there
  m->cursor_prev = m->cursor;
  m->cursor      = cursor;
  cursorMoved(m);
}
//:::::::::::::::::::
// Menu_SetCursorToItem
//:::::::::::::::::::
void cursorSetToItem(MenuFw* m, void* ptr) {
  for (int i = 0; i < m->nitems; i++) {
    if (m->items[i] == ptr) {
      cursorSet(m, i);
      return;
    }
  }
}
//:::::::::::::::::::
// Menu_AdjustCursor
//   Takes the given menu, the direction, and attempts
//   to adjust the menu's cursor so that it's at the next available slot.
//:::::::::::::::::::
void cursorAdjust(MenuFw* m, int dir) {
  MenuCommon* item    = NULL;
  bool        wrapped = false;
wrap:
  while (m->cursor >= 0 && m->cursor < m->nitems) {
    item = (MenuCommon*)m->items[m->cursor];
    if ((item->flags & (MFL_GRAYED | MFL_MOUSEONLY | MFL_INACTIVE))) {
      m->cursor += dir;
    } else {
      break;
    }
  }
  if (dir == 1) {
    if (m->cursor >= m->nitems) {
      if (m->wrapAround) {
        if (wrapped) {
          m->cursor = m->cursor_prev;
          return;
        }
        m->cursor = 0;
        wrapped   = true;
        goto wrap;
      }
      m->cursor = m->cursor_prev;
    }
  } else {
    if (m->cursor < 0) {
      if (m->wrapAround) {
        if (wrapped) {
          m->cursor = m->cursor_prev;
          return;
        }
        m->cursor = m->nitems - 1;
        wrapped   = true;
        goto wrap;
      }
      m->cursor = m->cursor_prev;
    }
  }
}
//:::::::::::::::::::
