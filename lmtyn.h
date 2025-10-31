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

typedef struct lmtyn_context
{

  u32 vertices_size;
  u32 indices_size;

  f32 *vertices;
  u32 *indices;

} lmtyn_context;

LMTYN_API LMTYN_INLINE void lmtyn(lmtyn_context *ctx)
{
  lmtyn_shape_circle circle_1 = {0.0f, 0.0f, 0.0f, 0.5f};
  lmtyn_shape_circle circle_2 = {0.0f, 1.0f, 0.0f, 1.0f};

  (void)ctx;
  (void)circle_1;
  (void)circle_2;
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
