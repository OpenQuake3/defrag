/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// bg_lib.h -- standard C library replacement routines used by code
// compiled for the virtual machine

// This file is NOT included on native builds
#if !defined( BG_LIB_H ) && defined( Q3_VM )
#define BG_LIB_H

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned int size_t;

typedef char *  va_list;
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#define CHAR_BIT      8             /* number of bits in a char */
#define SCHAR_MAX     0x7f          /* maximum signed char value */
#define SCHAR_MIN   (-SCHAR_MAX - 1) /* minimum signed char value */
#define UCHAR_MAX     0xff          /* maximum unsigned char value */

#define SHRT_MAX      0x7fff        /* maximum (signed) short value */
#define SHRT_MIN    (-SHRT_MAX - 1) /* minimum (signed) short value */
#define USHRT_MAX     0xffff        /* maximum unsigned short value */
#define INT_MAX       0x7fffffff    /* maximum (signed) int value */
#define INT_MIN     (-INT_MAX - 1)  /* minimum (signed) int value */
#define UINT_MAX      0xffffffff    /* maximum unsigned int value */
#define LONG_MAX      0x7fffffffL   /* maximum (signed) long value */
#define LONG_MIN    (-LONG_MAX - 1) /* minimum (signed) long value */
#define ULONG_MAX     0xffffffffUL  /* maximum unsigned long value */

#define isalnum(c)  (isalpha(c) || isdigit(c))
#define isalpha(c)  (isupper(c) || islower(c))
#define isascii(c)  ((c) > 0 && (c) <= 0x7f)
#define iscntrl(c)  (((c) >= 0) && (((c) <= 0x1f) || ((c) == 0x7f)))
#define isdigit(c)  ((c) >= '0' && (c) <= '9')
#define isgraph(c)  ((c) != ' ' && isprint(c))
#define islower(c)  ((c) >=  'a' && (c) <= 'z')
#define isprint(c)  ((c) >= ' ' && (c) <= '~')
#define ispunct(c)  (((c) > ' ' && (c) <= '~') && !isalnum(c))
#define isspace(c)  ((c) ==  ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || \
                     (c) == '\t' || (c) == '\v')
#define isupper(c)  ((c) >=  'A' && (c) <= 'Z')
#define isxdigit(c) (isxupper(c) || isxlower(c))
#define isxlower(c) (isdigit(c) || (c >= 'a' && c <= 'f'))
#define isxupper(c) (isdigit(c) || (c >= 'A' && c <= 'F')) 

// Misc functions
typedef int cmp_t(const void *, const void *);
void qsort(void *a, size_t n, size_t es, cmp_t *cmp);
void	srand( unsigned seed );
int		rand( void );

// String functions
size_t strlen( const char *string );
char *strcat( char *strDestination, const char *strSource );
char *strcpy( char *strDestination, const char *strSource );
int strcmp( const char *string1, const char *string2 );
char *strchr( const char *string, int c );
char *strrchr(const char *string, int c);
char *strstr( const char *string, const char *strCharSet );
char *strncpy( char *strDest, const char *strSource, size_t count );
int tolower( int c );
int toupper( int c );

double atof( const char *string );
double _atof( const char **stringPtr );
double strtod( const char *nptr, char **endptr );
int atoi( const char *string );
int _atoi( const char **stringPtr );
long strtol( const char *nptr, char **endptr, int base );

int Q_vsnprintf( char *buffer, size_t length, const char *fmt, va_list argptr );

int sscanf( const char *buffer, const char *fmt, ... ) __attribute__ ((format (scanf, 2, 3)));

// Memory functions
void *memmove( void *dest, const void *src, size_t count );
void *memset( void *dest, int c, size_t count );
void *memcpy( void *dest, const void *src, size_t count );

// Math functions
double ceil( double x );
double floor( double x );
double sqrt( double x );
double sin( double x );
double cos( double x );
double atan2( double y, double x );
double tan( double x );
int abs( int n );
double fabs( double x );
double acos( double x );

//:: OSDF modded
#define M_E            2.7182818284590452354   /* e */
#define M_LOG2E        1.4426950408889634074   /* log_2 e */
#define M_LOG10E       0.43429448190325182765  /* log_10 e */
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#define M_PI           3.14159265358979323846  /* pi */
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#define M_PI_4         0.78539816339744830962  /* pi/4 */
#define M_1_PI         0.31830988618379067154  /* 1/pi */
#define M_2_PI         0.63661977236758134308  /* 2/pi */
#define M_2_SQRTPI     1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2        1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2      0.70710678118654752440  /* 1/sqrt(2) */

#ifdef NDEBUG
#define assert(exp)     ((void)0)
#else
#define assert(exp) if (!(exp)) trap_Print(__FILE__ ":" STR(__LINE__) ": Assertion `" STR(exp) "' failed.\n");
#endif

float Q_pown(float base, int exp);
float roundf( float x );
float expf( float x );
float logf( float a );
float powf( float x, float y );
//:: OSDF end

#endif // BG_LIB_H
