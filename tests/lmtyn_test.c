/* lmtyn.h - v0.1 - public domain data structures - nickscha 2025

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
  u32 memory_size = csr_memory_size((int)width, (int)height);
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

int main(void)
{
  /* #############################################################################
   * # LMTYN Usage
   * #############################################################################
   */
  lmtyn_shape_circle pillar[5] = {
      {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f}, /* bottom    */
      {0.0f, 1.0f, 0.0f, 0.6f, 0.0f, 1.0f, 0.0f}, /* low mid   */
      {0.0f, 2.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f}, /* center    */
      {0.0f, 3.0f, 0.0f, 0.6f, 0.0f, 1.0f, 0.0f}, /* upper mid */
      {0.0f, 4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f}  /* top       */
  };

  lmtyn_mesh mesh = {0};
  mesh.vertices_capacity = sizeof(f32) * 1024;
  mesh.indices_capacity = sizeof(u32) * 1024;
  mesh.vertices = malloc(sizeof(f32) * mesh.vertices_capacity);
  mesh.indices = malloc(sizeof(u32) * mesh.indices_capacity);

  assert(lmtyn_mesh_generate(
      &mesh,
      0,
      pillar,
      sizeof(pillar) / sizeof(pillar[0]),
      4));

  assert(lmtyn_mesh_normalize(
      &mesh,
      0.0f, 0.0f, 0.0f,
      1.0f));

  /* #############################################################################
   * # Render to PPM Frames
   * #############################################################################
   */
  {

    csr_color clear_color = {40, 40, 40};
    csr_context ctx = {0};

    assert(csr_init(&ctx, 600, 400));

    {
      v3 world_up = vm_v3(0.0f, 1.0f, 0.0f);
      v3 cam_position = vm_v3(0.0f, 0.6f, 1.0f);
      v3 cam_look_at_pos = vm_v3(0.0f, 0.0f, 0.0f);
      float cam_fov = 90.0f;

      m4x4 projection = vm_m4x4_perspective(vm_radf(cam_fov), (f32)ctx.width / (f32)ctx.height, 0.1f, 1000.0f);
      m4x4 view = vm_m4x4_lookAt(cam_position, cam_look_at_pos, world_up);
      m4x4 projection_view = vm_m4x4_mul(projection, view);

      m4x4 model_base = vm_m4x4_translate(vm_m4x4_identity, vm_v3_zero);
      v3 model_rotation_x = vm_v3(1.0f, 0.0f, 0.0);
      v3 model_rotation_y = vm_v3(0.0f, 1.0f, 0.0);

      int frame;

      for (frame = 0; frame < 200; ++frame)
      {

        /* Rotate the cube around the model_rotation axis */
        m4x4 model_view_projection = vm_m4x4_mul(
            projection_view,
            frame == 0 ? model_base : vm_m4x4_rotate(model_base, vm_radf(5.0f * (float)(frame + 1)), (frame / 100) % 2 == 0 ? model_rotation_x : model_rotation_y));

        /* Clear Screen Frame and Depth Buffer */
        csr_render_clear_screen(&ctx, clear_color);

        /* Render cube */
        csr_render(
            &ctx,
            (frame / 50) % 2 == 0
                ? CSR_RENDER_WIREFRAME
                : CSR_RENDER_SOLID,
            CSR_CULLING_CCW_BACKFACE, 3,
            mesh.vertices, mesh.vertices_size,
            (int *)mesh.indices, mesh.indices_size,
            model_view_projection.e);

        csr_save_ppm("test_%05d.ppm", frame, &ctx);
      }
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
