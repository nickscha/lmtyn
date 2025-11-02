/* lmtyn.h - v0.2 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) Lucid Modelling Tool You Need (LMTYN).

This Test class defines cases to verify that we don't break the excepted behaviours in the future upon changes.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include "../lmtyn.h"     /* Lucid Modelling Tool You Need */
#include "../deps/test.h" /* Simple Testing framework      */
#include "../deps/csr.h"  /* Software Based Renderer       */
#include "../deps/vm.h"   /* Linear Algebra Library        */

#include "stdlib.h"
#include "stdio.h"

static void csr_save_ppm(char *filename_format, int frame, csr_context *model)
{
  FILE *fp;
  char filename[64];

  /* Format the filename with the frame number */
  sprintf(filename, filename_format, frame);

  fp = fopen(filename, "wb");

  if (!fp)
  {
    fprintf(stderr, "Error: Could not open file %s for writing.\n", filename);
    return;
  }

  /* PPM header */
  fprintf(fp, "P6\n%d %d\n255\n", model->width, model->height);

  /* Pixel data */
  fwrite(model->framebuffer, sizeof(csr_color), (size_t)(model->width * model->height), fp);

  fclose(fp);
}

static u8 csr_init(csr_context *ctx, u32 width, u32 height)
{
  u32 memory_size = (u32)csr_memory_size((int)width, (int)height);
  void *memory = (void *)malloc(memory_size);

  if (!memory)
  {
    return 0;
  }

  if (!csr_init_model(ctx, memory, memory_size, (int)width, (int)height))
  {
    return 0;
  }

  return 1;
}

static void csr_render_mesh(csr_context *ctx, lmtyn_mesh *mesh, v3 cam_position, v3 model_position, u32 frame)
{
  v3 world_up = vm_v3(0.0f, 1.0f, 0.0f);
  v3 cam_look_at_pos = vm_v3(0.0f, 0.5f, 0.0f);
  float cam_fov = 90.0f;

  m4x4 projection = vm_m4x4_perspective(vm_radf(cam_fov), (f32)ctx->width / (f32)ctx->height, 0.1f, 1000.0f);
  m4x4 view = vm_m4x4_lookAt(cam_position, cam_look_at_pos, world_up);
  m4x4 projection_view = vm_m4x4_mul(projection, view);

  m4x4 model_base = vm_m4x4_translate(vm_m4x4_identity, model_position);
  v3 model_rotation_x = vm_v3(1.0f, 0.0f, 0.0);
  v3 model_rotation_y = vm_v3(0.0f, 1.0f, 0.0);

  /* Rotate the cube around the model_rotation axis */
  m4x4 model_view_projection = vm_m4x4_mul(
      projection_view,
      frame == 0 ? model_base : vm_m4x4_rotate(model_base, vm_radf(5.0f * (float)(frame + 1)), (frame / 100) % 2 == 0 ? model_rotation_x : model_rotation_y));

  /* Render cube */
  csr_render(
      ctx,
      (frame / 50) % 2 == 0
          ? CSR_RENDER_WIREFRAME
          : CSR_RENDER_SOLID,
      CSR_CULLING_CCW_BACKFACE, 3,
      mesh->vertices, mesh->vertices_size,
      (int *)mesh->indices, mesh->indices_size,
      model_view_projection.e);
}

#define NUM_VERTICES 512
#define NUM_INDICES 512

static void lmtyn_create_mesh(lmtyn_mesh *mesh, lmtyn_shape_circle *circles, u32 circles_count, u32 segments)
{
  mesh->vertices_capacity = sizeof(f32) * NUM_VERTICES;
  mesh->indices_capacity = sizeof(u32) * NUM_INDICES;
  mesh->vertices = malloc(sizeof(f32) * mesh->vertices_capacity);
  mesh->indices = malloc(sizeof(u32) * mesh->indices_capacity);

  assert(lmtyn_mesh_generate(mesh, 0, circles, circles_count, segments));
  assert(lmtyn_mesh_normalize(mesh, 0.0f, 0.0f, 0.0f, 1.0f));
}

int main(void)
{

  /* #############################################################################
   * # LMTYN Usage
   * #############################################################################
   */
  lmtyn_shape_circle arc[] = {
      {0.0f, 0.0f, 0.0f, 1.0f}, /* base */
      {0.0f, 4.0f, 0.0f, 1.0f}, /* move up */
      {1.0f, 5.0f, 0.0f, 1.0f}, /* slightly to right */
      {3.0f, 5.0f, 0.0f, 1.0f},
      {5.0f, 5.0f, 0.0f, 1.0f},
      {6.0f, 4.0f, 0.0f, 1.0f},
      {6.0f, 0.0f, 0.0f, 1.0f},
  };

  lmtyn_shape_circle pillar[] = {
      {0.0f, 0.0f, 0.0f, 1.0f}, /* bottom    */
      {0.0f, 1.0f, 0.0f, 0.6f}, /* low mid   */
      {0.0f, 2.0f, 0.0f, 0.5f}, /* center    */
      {0.0f, 3.0f, 0.0f, 0.6f}, /* upper mid */
      {0.0f, 4.0f, 0.0f, 1.0f}, /* top low   */
      {0.0f, 4.5f, 0.0f, 0.5f}  /* top       */
  };

  lmtyn_shape_circle circle[] = {
      {1.0f, 0.0f, 0.0f, 0.3f},
      {0.707f, 0.707f, 0.0f, 0.3f},
      {0.0f, 1.0f, 0.0f, 0.3f},
      {-0.707f, 0.707f, 0.0f, 0.3f},
      {-1.0f, 0.0f, 0.0f, 0.3f},
      {-0.707f, -0.707f, 0.0f, 0.3f},
      {0.0f, -1.0f, 0.0f, 0.3f},
      {0.707f, -0.707f, 0.0f, 0.3f},
      {1.0f, 0.0f, 0.0f, 0.3f} /* Repeat first circle */
  };

  lmtyn_shape_circle lamp[] = {
      {0.0f, 0.0f, 0.0f, 1.25f},
      {0.0f, 1.0f, 0.0f, 1.25f},
      {0.0f, 2.0f, 0.0f, 0.5f},
      {0.0f, 4.0f, 0.0f, 0.25f},
      {0.0f, 6.0f, 0.0f, 0.1f},
      {0.0f, 6.5f, 0.0f, 1.5f},
      {0.0f, 7.5f, 0.0f, 0.5f},
  };

  lmtyn_shape_circle pipe[] = {
      {0.0f, 0.0f, 0.0f, 0.3f},
      {1.0f, 0.0f, 0.0f, 0.3f},
      {2.0f, 1.0f, 0.0f, 0.3f},
      {2.5f, 1.0f, 0.0f, 0.3f}};

  lmtyn_shape_circle tower[] = {
      {0.0f, 0.0f, 0.0f, 1.5f},
      {0.0f, 0.5f, 0.0f, 1.5f},
      {0.0f, 0.5f, 0.0f, 0.5f},
      {0.0f, 5.0f, 0.0f, 0.5f},
      {0.0f, 6.0f, 0.0f, 1.5f},
      {0.0f, 7.0f, 0.0f, 2.0f},
      {0.0f, 7.0f, 0.0f, 1.0f},
      {0.0f, 8.0f, 0.0f, 0.1f}
  };

  lmtyn_mesh mesh_arc = {0};
  lmtyn_mesh mesh_pillar = {0};
  lmtyn_mesh mesh_circle = {0};
  lmtyn_mesh mesh_lamp = {0};
  lmtyn_mesh mesh_pipe = {0};
  lmtyn_mesh mesh_tower = {0};

  lmtyn_create_mesh(&mesh_arc, arc, sizeof(arc) / sizeof(arc[0]), 4);
  lmtyn_create_mesh(&mesh_pillar, pillar, sizeof(pillar) / sizeof(pillar[0]), 8);
  lmtyn_create_mesh(&mesh_circle, circle, sizeof(circle) / sizeof(circle[0]), 4);
  lmtyn_create_mesh(&mesh_lamp, lamp, sizeof(lamp) / sizeof(lamp[0]), 12);
  lmtyn_create_mesh(&mesh_pipe, pipe, sizeof(pipe) / sizeof(pipe[0]), 16);
  lmtyn_create_mesh(&mesh_tower, tower, sizeof(tower) / sizeof(tower[0]), 4);

  /* #############################################################################
   * # Render to PPM Frames
   * #############################################################################
   */
  {
    csr_color clear_color = {40, 40, 40};
    csr_context ctx = {0};

    u32 frame;
    v3 cam_position = vm_v3(0.0f, 0.6f, 1.4f);

    assert(csr_init(&ctx, 600, 400));

    for (frame = 0; frame < 200; ++frame)
    {
      csr_render_clear_screen(&ctx, clear_color);
      csr_render_mesh(&ctx, &mesh_arc, cam_position, vm_v3(-1.0f, 0.0f, 0.0f), frame);
      csr_render_mesh(&ctx, &mesh_pillar, cam_position, vm_v3_zero, frame);
      csr_render_mesh(&ctx, &mesh_circle, cam_position, vm_v3(1.0f, 0.0f, 0.0f), frame);
      csr_render_mesh(&ctx, &mesh_lamp, cam_position, vm_v3(-1.0f, 1.0f, 0.0f), frame);
      csr_render_mesh(&ctx, &mesh_pipe, cam_position, vm_v3(0.0f, 1.0f, 0.0f), frame);
      csr_render_mesh(&ctx, &mesh_tower, cam_position, vm_v3(1.0f, 1.0f, 0.0f), frame);
      csr_save_ppm("test_%05d.ppm", (int)frame, &ctx);
    }
  }

  printf("[lmtyn] finished\n");

  return 0;
}

/*
   -----------------------------------------------------------------------------
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
