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

#ifndef Q_ASSERT_H
#define Q_ASSERT_H

// #include "cg_local.h"
// #include "q_shared.h"

#include <signal.h>

#define FLT_DECIMAL_DIG 9

#define static_assert(x, m) _Static_assert(x, m)

#ifdef NDEBUG
#  define assert(x)
#  define ASSERT_TRUE(x)
#  define ASSERT_FALSE(x)
#  define ASSERT_EQ(x, y)
#  define ASSERT_NE(x, y)
#  define ASSERT_GT(x, y)
#  define ASSERT_LT(x, y)
#  define ASSERT_GE(x, y)
#  define ASSERT_LE(x, y)
#  define ASSERT_FLOAT_EQ(x, y)
#  define ASSERT_FLOAT_GE(x, y)
#else
#  define assert(x)                                                                                                    \
    if (!(x))                                                                                                          \
    {                                                                                                                  \
      printf("(%s:%d) %s -> %.*g\n", __FILE__, __LINE__, #x, FLT_DECIMAL_DIG, (double)(intptr_t)(x));                  \
      raise(SIGABRT);                                                                                                  \
    }
#  define ASSERT_TRUE(x)                                                                                               \
    if (!(x)) trap_Error(vaf("(%s:%d) %s -> %.*g\n", __FILE__, __LINE__, #x, FLT_DECIMAL_DIG, (double)(x)));
#  define ASSERT_FALSE(x)                                                                                              \
    if (!(!(x))) trap_Error(vaf("(%s:%d) !(%s) -> !(%.*g)\n", __FILE__, __LINE__, #x, FLT_DECIMAL_DIG, (double)(x)));
#  define ASSERT_EQ(x, y)                                                                                              \
    if (!((x) == (y)))                                                                                                 \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) %s == %s -> %.*g == %.*g\n",                                                                          \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(intptr_t)(x),                                                                                         \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(intptr_t)(y)));
#  define ASSERT_NE(x, y)                                                                                              \
    if (!((x) != (y)))                                                                                                 \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) %s != %s -> %.*g != %.*g\n",                                                                          \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(intptr_t)(x),                                                                                         \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(intptr_t)(y)));
#  define ASSERT_GT(x, y)                                                                                              \
    if (!((x) > (y)))                                                                                                  \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) %s > %s -> %.*g > %.*g\n",                                                                            \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(x),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(y)));
#  define ASSERT_LT(x, y)                                                                                              \
    if (!((x) < (y)))                                                                                                  \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) %s < %s -> %.*g < %.*g\n",                                                                            \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(x),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(y)));
#  define ASSERT_GE(x, y)                                                                                              \
    if (!((x) >= (y)))                                                                                                 \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) %s >= %s -> %.*g >= %.*g\n",                                                                          \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(x),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(y)));
#  define ASSERT_LE(x, y)                                                                                              \
    if (!((x) <= (y)))                                                                                                 \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) %s <= %s -> %.*g <= %.*g\n",                                                                          \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(x),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(y)));
#  define ASSERT_FLOAT_EQ(x, y)                                                                                        \
    if (!(fabsf((x) - (y)) <= fabsf(((x) + (y)) / 2) * 1.e-6f))                                                        \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) |(%s) - (%s)| <= |((%s) + (%s)) / 2| * 1e-6 -> |%.*g - %.*g| <= |%.*g| * 1e-6\n",                     \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(x),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(y),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(((x) + (y)) / 2)));
#  define ASSERT_FLOAT_GE(x, y)                                                                                        \
    if (!((x) - (y) >= fabsf(((x) + (y)) / 2) * -1.e-6f))                                                              \
      trap_Error(vaf(                                                                                                  \
        "(%s:%d) (%s - %s) >= |((%s) + (%s)) / 2| * -1e-6 -> %.*g - %.*g >= |%.*g| * -1e-6\n",                         \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        #x,                                                                                                            \
        #y,                                                                                                            \
        #x,                                                                                                            \
        #y,                                                                                                            \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(x),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(y),                                                                                                   \
        FLT_DECIMAL_DIG,                                                                                               \
        (double)(((x) + (y)) / 2)));
#endif

#endif // Q_ASSERT_H
