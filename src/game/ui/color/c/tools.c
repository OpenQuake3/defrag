#include "../tools.h"
//:::::::::::::::::::::::


//:::::::::::::::::::::::
void ColorSet(vec4_t color, const vec4_t src) {
  color[R] = src[R];
  color[G] = src[G];
  color[B] = src[B];
  color[A] = src[A];
}
void ColorSetA(vec4_t color, const vec4_t src, const vec_t alpha) {
  ColorSet(color, src);
  color[A] = alpha;
}

//:::::::::::::::::::::::
