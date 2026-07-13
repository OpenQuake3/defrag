#include "../elements.h"


//:::::::::::::::::::::::
// menuList_getMaxHeight
//   Return the max possible text height (percentage) for all items in the list
//:::::::::::::::::::::::
float menuList_getMaxHeight(MenuList* l) {
  float result = 0;
  for (int row = 0; row < l->itemCount; row++) {
    const char* name   = l->itemNames[row];
    float       height = uiTextGetHeight(name, &l->font, fontScale(&l->font), strlen(name));
    if (height > result) { result = height; }
  }
  return result;
}
//:::::::::::::::::::::::
// menuList_getRowStride
//   Return the vertical distance (percentage) between one row and the next
//:::::::::::::::::::::::
float menuList_getRowStride(MenuList* l) {
  return menuList_getMaxHeight(l) + l->separation[Y];
}
//:::::::::::::::::::::::
// menuList_getMaxCharWidth
//   Return the max possible character width (percentage), for all items in the list
float menuList_getMaxCharWidth(MenuList* l) {
  float result = 0;
  for (int row = 0; row < l->itemCount; row++) {
    const char* name = l->itemNames[row];
    for (int ch = 0; name[ch] != '\0'; ch++) {
      char  single[2] = { name[ch], '\0' };
      float width      = uiTextGetWidth(single, &l->font, fontScale(&l->font), 1);
      if (width > result) { result = width; }
    }
  }
  return result;
}

//:::::::::::::::::::::::
// ScrollList_Init
//:::::::::::::::::::::::
void menuList_init(MenuList* l) {
  l->topId    = 0;  // Start topId as the first item (id 0) in the list  (top item we are drawing)
  l->curvalue = 0;  // Make first item in the list (id 0) the selected one
  l->oldvalue = 0;  // Make the previously selected item to be id 0
  // Horizontal measurements
  if (l->columns <= 1) {  // If columns is negative, 1 or not defined
    l->columns       = 1;
    l->separation[X] = 0;
  } else if (!l->separation[X]) {  // Set some default X separation between columns, for when sep[X] is not defined
    l->separation[X] = l->width * 0.01;
  }
  float w          = l->width;  // ((l->width + l->separation[X]) * l->columns - l->separation[X]) * SMALLCHAR_WIDTH;
  l->generic.left  = l->generic.x;
  l->generic.right = l->generic.x + w;
  if (l->generic.flags & MFL_CENTER_JUSTIFY) {
    l->generic.left -= (w / 2);
    l->generic.right += (w / 2);
  } else if (l->generic.flags & MFL_RIGHT_JUSTIFY) {
    l->generic.left -= w;
    l->generic.right = l->generic.x;
  }
  l->itemSize[X] = (l->width / l->columns) * GL_W;

  // Vertical measurements
  if (l->rows <= 1) {  // If rows is negative, 1 or not defined
    l->rows          = 1;
    l->separation[Y] = 0;
  } else if (!l->separation[Y]) {  // Set some default Y separation between columns, for when sep[Y] is not defined
    l->separation[Y] = l->height * 0.01;
  }

  l->generic.top      = l->generic.y;
  l->generic.bottom   = (l->generic.y + l->height);

  float maxItemHeight = (l->height / l->rows);
  float itemHeight    = menuList_getRowStride(l);
  if (itemHeight > maxItemHeight) {  // Total height will overflow, so reduce the number of rows
    // Find the max row count that would fit the max height
    for (int maxRows = l->rows; maxRows > 0; maxRows--) {   // For every row, reducing the maximum by one each time
      bool overflows = (itemHeight * maxRows > l->height);  // Still overflows when current count height exceeds total height
      if (overflows) { continue; }                          // Reduce maxRows (aka go to next loop iteration) if we still overflow
      l->rows = maxRows;                                    // This row count will fit, so assign it
      break;                                                // Stop searching here (aka only remove as few items as required)
    }
  }

  l->itemSize[Y] = itemHeight * GL_H;
}

//:::::::::::::::::::::::
// ScrollList_Key
//:::::::::::::::::::::::
sfxHandle_t menuList_key(MenuList* l, int key) {
  switch (key) {
    case K_MOUSE1: {
      if (!(l->generic.flags & MFL_HASMOUSEFOCUS)) { break; }
      // check scroll region
      float x = l->generic.x;
      float y = l->generic.y;
      float w = l->width;
      if (l->generic.flags & MFL_CENTER_JUSTIFY) { x -= w / 2; }
      if (!cursorInRect(x, x + w, y, y + l->height)) { return (uiSound.silence); }
      float gridw = l->width / l->columns;                        // Get maximum possible width for any of the columns
      float gridh = menuList_getRowStride(l);
      // Convert screen-space to list-space, and find int position id in a grid of maxWidth by maxHeight
      int cursorx = ((float)uis.cursorx / GL_W - x) / gridw;  // (xpos-adj)/grid
      int column  = cursorx / (l->width + l->separation[X]);  // Find the hovered column id, from the grid (char-width divided x)
      int cursory = ((float)uis.cursory / GL_H - y) / gridh;  // (ypos-adj)/grid
      int index   = column * l->rows + cursory;               // Find the absolute id in the grid

      if (l->topId + index < l->itemCount) {                      // Check that relative id is within the list itemCount bounds
        int clickedItem = l->topId + index;
        // Double-click: same item clicked again within 300ms
        if (clickedItem == l->lastClickIndex && uis.realtime - l->lastClickTime < 300) {
          l->lastClickTime = 0;
          if (l->generic.callback) { l->generic.callback(l, MST_ACTIVE); }
          return (uiSound.move);
        }
        l->lastClickTime  = uis.realtime;
        l->lastClickIndex = clickedItem;
        l->oldvalue = l->curvalue;                                // Store current value in prev
        l->curvalue = clickedItem;                                // Store relative value as current
        if (l->oldvalue != l->curvalue && l->generic.callback) {  // If they are different, and there is a callback, do movement
          l->generic.callback(l, MST_FOCUS);                      // Callback will process with the cur/old values updated right above this
        }
      }
      return (uiSound.move);
    }

    case K_KP_ENTER: /* fall-through */
    case K_ENTER: {
      if (l->generic.callback) { l->generic.callback(l, MST_ACTIVE); }
      return (uiSound.move);
    }

    case K_KP_HOME: /* fall-through */
    case K_HOME: {
      l->oldvalue = l->curvalue;
      l->curvalue = 0;
      l->topId    = 0;
      if (!(l->oldvalue != l->curvalue && l->generic.callback)) { return (uiSound.error); }
      l->generic.callback(l, MST_FOCUS);
      return (uiSound.move);
    }

    case K_KP_END: /* fall-through */
    case K_END: {
      l->oldvalue = l->curvalue;
      l->curvalue = l->itemCount - 1;
      if (l->columns > 1) {
        int c    = (l->curvalue / l->rows + 1) * l->rows;
        l->topId = c - (l->columns * l->rows);
      } else {
        l->topId = l->curvalue - (l->rows - 1);
      }
      if (l->topId < 0) l->topId = 0;
      if (l->oldvalue != l->curvalue && l->generic.callback) {
        l->generic.callback(l, MST_FOCUS);
        return (uiSound.move);
      }
      return (uiSound.error);
    }

    case K_PGUP: /* fall-through */
    case K_KP_PGUP: {
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->curvalue > 0)) { return (uiSound.error); }
      l->oldvalue = l->curvalue;
      l->curvalue -= l->rows - 1;
      if (l->curvalue < 0) l->curvalue = 0;
      l->topId = l->curvalue;
      if (l->topId < 0) l->topId = 0;
      if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
      return (uiSound.move);
    }

    case K_PGDN: /* fall-through */
    case K_KP_PGDN: {
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->curvalue < l->itemCount - 1)) { return (uiSound.error); }
      l->oldvalue = l->curvalue;
      l->curvalue += l->rows - 1;
      if (l->curvalue > l->itemCount - 1) l->curvalue = l->itemCount - 1;
      l->topId = l->curvalue - (l->rows - 1);
      if (l->topId < 0) l->topId = 0;
      if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
      return (uiSound.move);
    }

    case K_MWHEELUP: {
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->topId > 0)) { return (uiSound.error); }
      // if scrolling 3 lines would replace over half of the displayed items, only scroll 1 item at a time.
      int scroll = l->rows < 6 ? 1 : 3;
      l->topId -= scroll;
      if (l->topId < 0) l->topId = 0;
      if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
      return (q3sound.menu_null);  // make scrolling silent
    }

    case K_MWHEELDOWN: {
      if (l->columns > 1) { return q3sound.menu_null; }
      if (!(l->topId < l->itemCount - l->rows)) { return (uiSound.error); }
      // if scrolling 3 items would replace over half of the displayed items, only scroll 1 item at a time.
      int scroll = l->rows < 6 ? 1 : 3;
      l->topId += scroll;
      if (l->topId > l->itemCount - l->rows) l->topId = l->itemCount - l->rows;
      if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
      return (q3sound.menu_null);  // make scrolling silent
    }

    case K_KP_UPARROW: /* fall-through */
    case K_UPARROW: {
      if (l->curvalue == 0) { return uiSound.error; }
      l->oldvalue = l->curvalue;
      l->curvalue--;
      if (l->curvalue < l->topId) { // TODO: Untangle this
        if (l->columns == 1) l->topId--;
        else                 l->topId -= l->rows;
      }
      if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
      return (uiSound.move);
    }

    case K_KP_DOWNARROW: /* fall-through */
    case K_DOWNARROW: {
      if (l->curvalue == l->itemCount - 1) return uiSound.error;
      l->oldvalue = l->curvalue;
      l->curvalue++;
      if (l->curvalue >= l->topId + l->columns * l->rows) {
        if (l->columns == 1) l->topId++;
        else l->topId += l->rows;
      }
      if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
      return uiSound.move;
    }

    case K_KP_LEFTARROW: /* fall-through */
    case K_LEFTARROW: {
      if (l->columns == 1      ) return q3sound.menu_null;
      if (l->curvalue < l->rows) return uiSound.error;
      l->oldvalue = l->curvalue;
      l->curvalue -= l->rows;
      if (l->curvalue < l->topId) l->topId -= l->rows;
      if (l->generic.callback)    l->generic.callback(l, MST_FOCUS);
      return uiSound.move;
    }

    case K_KP_RIGHTARROW: /* fall-through */
    case K_RIGHTARROW: {
      if (l->columns == 1) return q3sound.menu_null;
      int c = l->curvalue + l->rows;
      if (c >= l->itemCount) return uiSound.error;
      l->oldvalue = l->curvalue;
      l->curvalue = c;
      if (l->curvalue > l->topId + l->columns * l->rows - 1) l->topId += l->rows;
      if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
      return uiSound.move;
    }
  }
  // cycle look for ascii key inside list items
  if (!Q_isprint(key)) { return (0); }
  // force to lower for case insensitive compare
  if (Q_isupper(key)) { key -= 'A' - 'a'; }
  // For every item in the list
  for (int item = 1; item <= l->itemCount; ++item) {
    int id = (l->curvalue + item) % l->itemCount;
    int ch = l->itemNames[id][0];
    if (Q_isupper(ch)) { ch -= 'A' - 'a'; }
    if (ch == key) {
      // set current item, mimic windows listbox scroll behavior
      if (id < l->topId) l->topId = id;                                     // behind top most item, set this as new top
      else if (id > l->topId + l->rows - 1) l->topId = (id + 1) - l->rows;  // past end of list box, do page down
      if (l->curvalue != id) {
        l->oldvalue = l->curvalue;
        l->curvalue = id;
        if (l->generic.callback) l->generic.callback(l, MST_FOCUS);
        return (uiSound.move);
      }
      return (uiSound.error);
    }
  }
  return (uiSound.error);
}

// ScrollList_Draw
void menuList_draw(MenuList* l) {
  // Define a max width (instead of a width per column)
  // Cap each column to not go over its max width
  // Divide width by the number of columns when we draw
  float x    = l->generic.left;  // Always draw starting from the top-left
  float xAdj = l->separation[X] * 0.5;
  float yAdj = l->separation[Y] * 0.5;
  for (int column = 0; column < l->columns; column++) {
    float y       = l->generic.top;  // Always draw starting from the top-left
    int   currTop = l->topId + column * l->rows;
    for (int row = currTop; row < currTop + l->rows; row++) {  // For every row that we are drawing
      if (row >= l->itemCount) { break; }                      // If we are at the end of the list
      vec4_t color;
      char*  itemName  = l->itemNames[row];
      int    maxChars  = strlen(itemName);
      int    nameWidth = uiTextGetWidthPix(itemName, &l->font, fontScale(&l->font), maxChars);
      if (row == l->curvalue) {  // Draw active item
        // if (l->generic.flags & MFL_CENTER_JUSTIFY) { u -= (l->width * SMALLCHAR_WIDTH) / 2 + 1; }
        ColorSetA(color, *mColor.key, 0.5);
        uiFillRect(x, y, nameWidth, l->itemSize[Y], color);
        ColorSet(color, *mColor.fg);
        // If the item has focus
        bool shouldPulse = l->generic.parent->cursor == l->generic.activeId && l->generic.flags & MFL_PULSEIFFOCUS;
        if (shouldPulse) { l->style |= UI_PULSE; }
      } else {
        if (l->style & UI_PULSE) l->style &= ~UI_PULSE;  // Remove pulse, since item is not active
        ColorSet(color, *mColor.neutral);
      }
      if (l->generic.flags & MFL_CENTER_JUSTIFY) { l->style |= UI_CENTER; }  // Not using this atm. Font alignment property covers it
      if (nameWidth > (int)l->itemSize[X]) {                                 // Name would overflow the allowed width for this column
        for (int currMax = 0; currMax < maxChars; currMax++) {               // Cap the maximum number of characters we can draw
          // If the text width overflows the bounds, reduce the maxChars allowed to be drawn
          if (uiTextGetWidthPix(itemName, &l->font, fontScale(&l->font), maxChars) >= l->itemSize[X]) { maxChars--; }
        }
      }
      float itemH = l->itemSize[Y] / GL_H;
      uiTextDraw(itemName, &l->font, x + xAdj, y + yAdj + itemH, fontScale(&l->font), color, 0, l->style, maxChars, l->align);
      y += l->itemSize[Y] / GL_H;
    }
    x += (l->itemSize[X] / GL_W) + l->separation[X];  // (l->width + l->separation) * SMALLCHAR_WIDTH;
  }
}
