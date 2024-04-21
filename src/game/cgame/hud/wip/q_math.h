#ifndef Q_MATH_H
#define Q_MATH_H

#include "q_shared.h"

#include <math.h>

/* This file was automatically generated.  Do not edit! */
int         Q_log2(int val);
void        Vector4Scale(vec4_t const in, vec_t scale, vec4_t out);
void        _VectorScale(vec3_t const in, vec_t scale, vec3_t out);
void        _VectorCopy(vec3_t const in, vec3_t out);
void        _VectorAdd(vec3_t const veca, vec3_t const vecb, vec3_t out);
void        _VectorSubtract(vec3_t const veca, vec3_t const vecb, vec3_t out);
vec_t       _DotProduct(vec3_t const v1, vec3_t const v2);
void        _VectorMA(vec3_t const veca, float scale, vec3_t const vecb, vec3_t vecc);
void        AddPointToBounds(vec3_t const v, vec3_t mins, vec3_t maxs);
void        ClearBounds(vec3_t mins, vec3_t maxs);
float       RadiusFromBounds(vec3_t const mins, vec3_t const maxs);
#if defined __LCC__ || defined C_ONLY || !id386 ||                                                                     \
  defined __VECTORC && !((defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123
int BoxOnPlaneSide(vec3_t const emins, vec3_t const emaxs, struct cplane_s* p);
#endif
#if !(defined __LCC__ || defined C_ONLY || !id386 || defined __VECTORC) &&                                             \
  !((defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123
__declspec(naked) int BoxOnPlaneSide(vec3_t const emins, vec3_t const emaxs, struct cplane_s* p);
#endif
void  SetPlaneSignbits(cplane_t* out);
float AngleDelta(float angle1, float angle2);
float AngleMod(float a);
void  AnglesSubtract(vec3_t const v1, vec3_t const v2, vec3_t v3);
float AngleSubtract(float a1, float a2);
float LerpAngle(float from, float to, float frac);
void  VectorRotate(vec3_t const in, vec3_t matrix[3], vec3_t out);
void  MakeNormalVectors(vec3_t const forward, vec3_t right, vec3_t up);
#if !idppc
float Q_fabs(float f);
#endif
void         ProjectPointOnPlane(vec3_t dst, vec3_t const p, vec3_t const normal);
void         RotateAroundDirection(vec3_t axis[3], float yaw);
void         MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
qboolean     PlaneFromPoints(vec4_t plane, vec3_t const a, vec3_t const b, vec3_t const c);
float        NormalizeColor(vec3_t const in, vec3_t out);
void         ByteToDir(int b, vec3_t dir);
int          DirToByte(vec3_t const dir);
signed short ClampShort(int i);
signed char  ClampChar(int i);
#if !idppc
float Q_rsqrt(float number);
#endif

#endif // Q_MATH_H
