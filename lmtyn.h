/* lmtyn.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) Lucid Modelling Tool You Need (LMTYN).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef LMTYN_H
#define LMTYN_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define LMTYN_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
#define LMTYN_INLINE __inline__
#elif defined(_MSC_VER)
#define LMTYN_INLINE __inline
#else
#define LMTYN_INLINE
#endif

#define LMTYN_API static

typedef unsigned char u8;
typedef unsigned int u32;
typedef float f32;

#define LMTYN_STATIC_ASSERT(c, m) typedef char lmtyn_assert_##m[(c) ? 1 : -1]

LMTYN_STATIC_ASSERT(sizeof(u8) == 1, u8_size_must_be_1);
LMTYN_STATIC_ASSERT(sizeof(u32) == 4, u32_size_must_be_4);
LMTYN_STATIC_ASSERT(sizeof(f32) == 4, f32_size_must_be_4);

/* #############################################################################
 * # Linear Algebra / Vector math
 * #############################################################################
 */
#define LMTYN_PI 3.14159265358979323846f
#define LMTYN_PI2 6.28318530717958647692f
#define LMTYN_PI_HALF 1.57079632679489661923f

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4699) /* MSVC-specific aliasing warning */
#endif
LMTYN_API LMTYN_INLINE f32 lmtyn_invsqrt(f32 number)
{
  union
  {
    f32 f;
    long i;
  } conv;

  f32 x2, y;
  const f32 threehalfs = 1.5F;

  x2 = number * 0.5F;
  conv.f = number;
  conv.i = 0x5f3759df - (conv.i >> 1); /* Magic number for approximation */
  y = conv.f;
  y = y * (threehalfs - (x2 * y * y)); /* One iteration of Newton's method */

  return (y);
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

LMTYN_API LMTYN_INLINE f32 lmtyn_sqrtf(f32 x)
{
  return (x * lmtyn_invsqrt(x));
}

LMTYN_API LMTYN_INLINE f32 lmtyn_absf(f32 x)
{
  return (x < 0.0f ? -x : x);
}

#define LMTYN_LUT_SIZE 256
#define LMTYN_LUT_MASK (LMTYN_LUT_SIZE - 1)

static const f32 lmtyn_lut[LMTYN_LUT_SIZE] = {
    0.0000f, 0.0245f, 0.0491f, 0.0736f, 0.0980f, 0.1224f, 0.1467f, 0.1710f,
    0.1951f, 0.2191f, 0.2430f, 0.2667f, 0.2903f, 0.3137f, 0.3369f, 0.3599f,
    0.3827f, 0.4052f, 0.4276f, 0.4496f, 0.4714f, 0.4929f, 0.5141f, 0.5350f,
    0.5556f, 0.5758f, 0.5957f, 0.6152f, 0.6344f, 0.6532f, 0.6716f, 0.6895f,
    0.7071f, 0.7242f, 0.7409f, 0.7572f, 0.7730f, 0.7883f, 0.8032f, 0.8176f,
    0.8315f, 0.8449f, 0.8577f, 0.8701f, 0.8819f, 0.8932f, 0.9040f, 0.9142f,
    0.9239f, 0.9330f, 0.9415f, 0.9495f, 0.9569f, 0.9638f, 0.9700f, 0.9757f,
    0.9808f, 0.9853f, 0.9892f, 0.9925f, 0.9952f, 0.9973f, 0.9988f, 0.9997f,
    1.0000f, 0.9997f, 0.9988f, 0.9973f, 0.9952f, 0.9925f, 0.9892f, 0.9853f,
    0.9808f, 0.9757f, 0.9700f, 0.9638f, 0.9569f, 0.9495f, 0.9415f, 0.9330f,
    0.9239f, 0.9142f, 0.9040f, 0.8932f, 0.8819f, 0.8701f, 0.8577f, 0.8449f,
    0.8315f, 0.8176f, 0.8032f, 0.7883f, 0.7730f, 0.7572f, 0.7409f, 0.7242f,
    0.7071f, 0.6895f, 0.6716f, 0.6532f, 0.6344f, 0.6152f, 0.5957f, 0.5758f,
    0.5556f, 0.5350f, 0.5141f, 0.4929f, 0.4714f, 0.4496f, 0.4276f, 0.4052f,
    0.3827f, 0.3599f, 0.3369f, 0.3137f, 0.2903f, 0.2667f, 0.2430f, 0.2191f,
    0.1951f, 0.1710f, 0.1467f, 0.1224f, 0.0980f, 0.0736f, 0.0491f, 0.0245f,
    0.0000f, -0.0245f, -0.0491f, -0.0736f, -0.0980f, -0.1224f, -0.1467f, -0.1710f,
    -0.1951f, -0.2191f, -0.2430f, -0.2667f, -0.2903f, -0.3137f, -0.3369f, -0.3599f,
    -0.3827f, -0.4052f, -0.4276f, -0.4496f, -0.4714f, -0.4929f, -0.5141f, -0.5350f,
    -0.5556f, -0.5758f, -0.5957f, -0.6152f, -0.6344f, -0.6532f, -0.6716f, -0.6895f,
    -0.7071f, -0.7242f, -0.7409f, -0.7572f, -0.7730f, -0.7883f, -0.8032f, -0.8176f,
    -0.8315f, -0.8449f, -0.8577f, -0.8701f, -0.8819f, -0.8932f, -0.9040f, -0.9142f,
    -0.9239f, -0.9330f, -0.9415f, -0.9495f, -0.9569f, -0.9638f, -0.9700f, -0.9757f,
    -0.9808f, -0.9853f, -0.9892f, -0.9925f, -0.9952f, -0.9973f, -0.9988f, -0.9997f,
    -1.0000f, -0.9997f, -0.9988f, -0.9973f, -0.9952f, -0.9925f, -0.9892f, -0.9853f,
    -0.9808f, -0.9757f, -0.9700f, -0.9638f, -0.9569f, -0.9495f, -0.9415f, -0.9330f,
    -0.9239f, -0.9142f, -0.9040f, -0.8932f, -0.8819f, -0.8701f, -0.8577f, -0.8449f,
    -0.8315f, -0.8176f, -0.8032f, -0.7883f, -0.7730f, -0.7572f, -0.7409f, -0.7242f,
    -0.7071f, -0.6895f, -0.6716f, -0.6532f, -0.6344f, -0.6152f, -0.5957f, -0.5758f,
    -0.5556f, -0.5350f, -0.5141f, -0.4929f, -0.4714f, -0.4496f, -0.4276f, -0.4052f,
    -0.3827f, -0.3599f, -0.3369f, -0.3137f, -0.2903f, -0.2667f, -0.2430f, -0.2191f,
    -0.1951f, -0.1710f, -0.1467f, -0.1224f, -0.0980f, -0.0736f, -0.0491f, -0.0245f};

LMTYN_API LMTYN_INLINE f32 lmtyn_sinf(f32 x)
{
  f32 index, frac;
  int i, i2;

  x -= LMTYN_PI2 * (f32)((int)(x * (1.0f / LMTYN_PI2)));

  if (x < 0)
  {
    x += LMTYN_PI2;
  }

  index = x * (LMTYN_LUT_SIZE / LMTYN_PI2);
  i = (int)index;
  frac = index - (f32)i;

  i &= (LMTYN_LUT_SIZE - 1);
  i2 = (i + 1) & (LMTYN_LUT_SIZE - 1);

  return (lmtyn_lut[i] + frac * (lmtyn_lut[i2] - lmtyn_lut[i]));
}

LMTYN_API LMTYN_INLINE f32 lmtyn_cosf(f32 x)
{
  return (lmtyn_sinf(x + LMTYN_PI_HALF));
}

typedef struct lmtyn_v3
{
  f32 x;
  f32 y;
  f32 z;

} lmtyn_v3;

LMTYN_API LMTYN_INLINE lmtyn_v3 lmtyn_v3_add(lmtyn_v3 a, lmtyn_v3 b)
{
  lmtyn_v3 r;

  r.x = a.x + b.x;
  r.y = a.y + b.y;
  r.z = a.z + b.z;

  return r;
}

LMTYN_API LMTYN_INLINE lmtyn_v3 lmtyn_v3_scale(lmtyn_v3 a, f32 s)
{
  lmtyn_v3 r;

  r.x = a.x * s;
  r.y = a.y * s;
  r.z = a.z * s;

  return r;
}

LMTYN_API LMTYN_INLINE lmtyn_v3 lmtyn_v3_cross(lmtyn_v3 a, lmtyn_v3 b)
{
  lmtyn_v3 r;

  r.x = a.y * b.z - a.z * b.y;
  r.y = a.z * b.x - a.x * b.z;
  r.z = a.x * b.y - a.y * b.x;

  return r;
}

LMTYN_API LMTYN_INLINE f32 lmtyn_v3_dot(lmtyn_v3 a, lmtyn_v3 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

LMTYN_API LMTYN_INLINE lmtyn_v3 lmtyn_v3_normalize(lmtyn_v3 v)
{
  f32 len = lmtyn_sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

  if (len < 1e-6f)
  {
    len = 1.0f;
  }

  v.x /= len;
  v.y /= len;
  v.z /= len;

  return v;
}

/* #############################################################################
 * # LMTYN Functions
 * #############################################################################
 */
typedef struct lmtyn_shape_circle
{

  /* Center-Point */
  f32 center_x;
  f32 center_y;
  f32 center_z;

  f32 radius;

  /* The orientation of the circle */
  f32 normal_x;
  f32 normal_y;
  f32 normal_z;

} lmtyn_shape_circle;

typedef struct lmtyn_mesh
{
  u32 vertices_capacity;
  u32 vertices_size;

  u32 indices_capacity;
  u32 indices_size;

  f32 *vertices;
  u32 *indices;

} lmtyn_mesh;

LMTYN_API LMTYN_INLINE u8 lmtyn_mesh_generate(
    lmtyn_mesh *mesh,
    lmtyn_shape_circle *circles,
    u32 circles_count,
    u32 segments)
{

  u32 vertices_count;
  u32 indices_count;

  u32 i, c, s, v;
  u32 bottomCenterIndex, topCenterIndex, topStart;

  if (!mesh || !circles || circles_count == 0 || segments == 0)
  {
    return 0;
  }

  vertices_count = circles_count * segments + 2;
  indices_count = (circles_count - 1) * segments * 6 + segments * 6;

  if (mesh->vertices_capacity < sizeof(f32) * 3 * vertices_count ||
      mesh->indices_capacity < sizeof(u32) * indices_count)
  {
    return 0;
  }

  mesh->vertices_size = vertices_count;
  mesh->indices_size = indices_count;

  i = 0;
  v = 0;

  for (c = 0; c < circles_count; ++c)
  {

    lmtyn_v3 up = {0.0f, 1.0f, 0.0f};
    lmtyn_v3 center;
    lmtyn_v3 normal;
    lmtyn_v3 U, V;
    f32 radius = circles[c].radius;

    center.x = circles[c].center_x;
    center.y = circles[c].center_y;
    center.z = circles[c].center_z;

    normal.x = circles[c].normal_x;
    normal.y = circles[c].normal_y;
    normal.z = circles[c].normal_z;
    normal = lmtyn_v3_normalize(normal);

    if (lmtyn_absf(lmtyn_v3_dot(up, normal)) > 0.99f)
    {
      up.x = 1.0f;
      up.y = 0.0f;
      up.z = 0.0f;
    }

    U = lmtyn_v3_normalize(lmtyn_v3_cross(up, normal));
    V = lmtyn_v3_cross(normal, U);

    for (s = 0; s < segments; ++s)
    {
      f32 angle = (2.0f * LMTYN_PI * (f32)s) / (f32)segments;

      lmtyn_v3 offset = lmtyn_v3_add(
          lmtyn_v3_scale(U, lmtyn_cosf(angle) * radius),
          lmtyn_v3_scale(V, lmtyn_sinf(angle) * radius));

      lmtyn_v3 final = lmtyn_v3_add(center, offset);

      mesh->vertices[v++] = final.x;
      mesh->vertices[v++] = final.y;
      mesh->vertices[v++] = final.z;
    }
  }

  /* Add center vertices for caps */
  bottomCenterIndex = v / 3;
  mesh->vertices[v++] = circles[0].center_x;
  mesh->vertices[v++] = circles[0].center_y;
  mesh->vertices[v++] = circles[0].center_z;

  topCenterIndex = v / 3;
  mesh->vertices[v++] = circles[circles_count - 1].center_x;
  mesh->vertices[v++] = circles[circles_count - 1].center_y;
  mesh->vertices[v++] = circles[circles_count - 1].center_z;

  /* Sides */
  for (c = 0; c < circles_count - 1; ++c)
  {
    for (s = 0; s < segments; ++s)
    {
      u32 curr = c * segments + s;
      u32 next = c * segments + (s + 1) % segments;
      u32 currUp = (c + 1) * segments + s;
      u32 nextUp = (c + 1) * segments + (s + 1) % segments;

      /* Clockwise
      mesh->indices[i++] = curr;
      mesh->indices[i++] = nextUp;
      mesh->indices[i++] = currUp;

      mesh->indices[i++] = curr;
      mesh->indices[i++] = next;
      mesh->indices[i++] = nextUp;
      */

      /* CCW triangle winding (correct for backface culling) */
      mesh->indices[i++] = curr;
      mesh->indices[i++] = currUp;
      mesh->indices[i++] = nextUp;

      mesh->indices[i++] = curr;
      mesh->indices[i++] = nextUp;
      mesh->indices[i++] = next;
    }
  }

  /* Bottom cap */
  for (s = 0; s < segments; ++s)
  {
    u32 next = (s + 1) % segments;

    /* Clockwise
    mesh->indices[i++] = bottomCenterIndex;
    mesh->indices[i++] = next;
    mesh->indices[i++] = s;
    */

    mesh->indices[i++] = bottomCenterIndex;
    mesh->indices[i++] = s;
    mesh->indices[i++] = next;
  }

  /* Top cap */
  topStart = (circles_count - 1) * segments;

  for (s = 0; s < segments; ++s)
  {
    u32 next = (s + 1) % segments;

    /* Clockwise
    mesh->indices[i++] = topCenterIndex;
    mesh->indices[i++] = topStart + s;
    mesh->indices[i++] = topStart + next;
    */

    mesh->indices[i++] = topCenterIndex;
    mesh->indices[i++] = topStart + next;
    mesh->indices[i++] = topStart + s;
  }

  return 1;
}

LMTYN_API LMTYN_INLINE u8 lmtyn_mesh_normalize(
    lmtyn_mesh *mesh,
    f32 target_x,
    f32 target_y,
    f32 target_z,
    f32 targetSize)
{
  f32 min_x = 1e9f, min_y = 1e9f, min_z = 1e9f;
  f32 max_x = -1e9f, max_y = -1e9f, max_z = -1e9f;

  u32 i;
  f32 scale = 1.0f;
  f32 center_x, center_y, center_z;
  f32 size_x, size_y, size_z;
  f32 size_max;

  if (!mesh || !mesh->vertices || mesh->vertices_size < 1)
  {
    return 0;
  }

  /* Compute bounding box */
  for (i = 0; i < mesh->vertices_size; ++i)
  {
    f32 x = mesh->vertices[i * 3 + 0];
    f32 y = mesh->vertices[i * 3 + 1];
    f32 z = mesh->vertices[i * 3 + 2];

    if (x < min_x)
    {
      min_x = x;
    }

    if (y < min_y)
    {
      min_y = y;
    }

    if (z < min_z)
    {
      min_z = z;
    }

    if (x > max_x)
    {
      max_x = x;
    }

    if (y > max_y)
    {
      max_y = y;
    }

    if (z > max_z)
    {
      max_z = z;
    }
  }

  /* Compute center and size */
  center_x = (min_x + max_x) * 0.5f;
  center_y = (min_y + max_y) * 0.5f;
  center_z = (min_z + max_z) * 0.5f;

  size_x = max_x - min_x;
  size_y = max_y - min_y;
  size_z = max_z - min_z;

  size_max = size_x;

  if (size_y > size_max)
  {
    size_max = size_y;
  }

  if (size_z > size_max)
  {
    size_max = size_z;
  }

  if (size_max < 1e-6f)
  {
    return 0; /* prevent divide by zero */
  }

  if (targetSize > 0.0f && size_max > 1e-6f)
  {
    scale = targetSize / size_max;
  }

  /* Apply normalization (translate + scale) */
  for (i = 0; i < mesh->vertices_size; ++i)
  {
    /* move to origin first, then scale, then move to target */
    f32 x = (mesh->vertices[i * 3 + 0] - center_x) * scale + target_x;
    f32 y = (mesh->vertices[i * 3 + 1] - center_y) * scale + target_y;
    f32 z = (mesh->vertices[i * 3 + 2] - center_z) * scale + target_z;

    mesh->vertices[i * 3 + 0] = x;
    mesh->vertices[i * 3 + 1] = y;
    mesh->vertices[i * 3 + 2] = z;
  }

  return 1;
}

#endif /* LMTYN_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
