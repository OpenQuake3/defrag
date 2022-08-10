#ifndef UI_COLOR_SCHEMES_H
#define UI_COLOR_SCHEMES_H
//:::::::::::::::::::::::

#include "../shared.h"

//:::::::::::::::::::::::
// Quake3 Color scheme structure
typedef struct q3scheme_s {
  const vec4_t* pulse;
  const vec4_t* listbar;
  const vec4_t* text_disabled;
  const vec4_t* text_normal;
  const vec4_t* text_highlight;
  const vec4_t* text_status;
  const vec4_t* text_menu;
} Q3Scheme;
//:::::::::::::::::::::::
// Color Schemes
typedef struct colorScheme_s {
  const vec4_t* bg;         // Background. Meant to fill space and not draw attention
  const vec4_t* neutral;    // Need to draw attention, but only over a background. Text over a bg, general info, etc
  const vec4_t* fg;         // Remarkable over neutral, but still not flashy. Decoration, or highlights for neutral elements
  const vec4_t* highlight;  // Draws a lot of a attention to important things, but can be forgotten. Call to action, features
  const vec4_t* key;        // Crucial information that needs to be remembered. Logo, brand, etc
} ColorScheme;
//:::::::::::::::::::::::
extern Q3Scheme    q3color;     // Default Quake3 Color scheme
extern ColorScheme q3credits;   // Default Quake3 Credits screen colors
extern ColorScheme mStartQ3;    // Start menu scheme with Quake3 colors.  TODO: Remove, just for testing
extern ColorScheme mStartColor; // Start menu scheme
//:::::::::::::::::::::::

//:::::::::::::::::::::::
#endif//UI_COLOR_SCHEMES_H
