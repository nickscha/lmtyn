#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct
{
    float x, y, z;
} Vec3;

typedef struct
{
    float center_x;
    float center_y;
    float center_z;
    float radius;
    float normal_x;
    float normal_y;
    float normal_z;
} Circle3D;

typedef struct
{
    float *vertices;
    unsigned int *indices;
    int vertexCount;
    int indexCount;
} Mesh3D;

/* === Vector Math Utilities === */
static Vec3 vec3_add(Vec3 a, Vec3 b)
{
    Vec3 r = {a.x + b.x, a.y + b.y, a.z + b.z};
    return r;
}

static Vec3 vec3_scale(Vec3 a, float s)
{
    Vec3 r = {a.x * s, a.y * s, a.z * s};
    return r;
}

static Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    Vec3 r = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
    return r;
}

static float vec3_dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 vec3_normalize(Vec3 v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len < 1e-6f)
        len = 1.0f;
    v.x /= len;
    v.y /= len;
    v.z /= len;
    return v;
}

Mesh3D generateMeshFromOrientedCircles3D(Circle3D *circles, int circleCount, int segments)
{
    Mesh3D mesh;
    // +2 vertices for top/bottom centers
    mesh.vertexCount = circleCount * segments + 2;
    // indices for sides + caps
    mesh.indexCount = (circleCount - 1) * segments * 6 + segments * 6;
    mesh.vertices = (float *)malloc(sizeof(float) * 3 * mesh.vertexCount);
    mesh.indices = (unsigned int *)malloc(sizeof(unsigned int) * mesh.indexCount);

    if (!mesh.vertices || !mesh.indices)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    int v = 0;
    for (int c = 0; c < circleCount; ++c)
    {
        Vec3 center = {circles[c].center_x, circles[c].center_y, circles[c].center_z};
        float r = circles[c].radius;
        Vec3 N = {circles[c].normal_x, circles[c].normal_y, circles[c].normal_z};
        N = vec3_normalize(N);

        Vec3 up = {0.0f, 1.0f, 0.0f};
        if (fabsf(vec3_dot(up, N)) > 0.99f)
            up = (Vec3){1, 0, 0};

        Vec3 U = vec3_normalize(vec3_cross(up, N));
        Vec3 V = vec3_cross(N, U);

        for (int s = 0; s < segments; ++s)
        {
            float angle = (2.0f * (float)M_PI * s) / segments;
            Vec3 offset = vec3_add(vec3_scale(U, cosf(angle) * r), vec3_scale(V, sinf(angle) * r));
            Vec3 final = vec3_add(center, offset);
            mesh.vertices[v++] = final.x;
            mesh.vertices[v++] = final.y;
            mesh.vertices[v++] = final.z;
        }
    }

    // Add center vertices for caps
    int bottomCenterIndex = v / 3;
    mesh.vertices[v++] = circles[0].center_x;
    mesh.vertices[v++] = circles[0].center_y;
    mesh.vertices[v++] = circles[0].center_z;

    int topCenterIndex = v / 3;
    mesh.vertices[v++] = circles[circleCount - 1].center_x;
    mesh.vertices[v++] = circles[circleCount - 1].center_y;
    mesh.vertices[v++] = circles[circleCount - 1].center_z;

    // Sides
    int i = 0;
    for (int c = 0; c < circleCount - 1; ++c)
    {
        for (int s = 0; s < segments; ++s)
        {
            int curr = c * segments + s;
            int next = c * segments + (s + 1) % segments;
            int currUp = (c + 1) * segments + s;
            int nextUp = (c + 1) * segments + (s + 1) % segments;

            /* Clockwise
            mesh.indices[i++] = curr;
            mesh.indices[i++] = nextUp;
            mesh.indices[i++] = currUp;

            mesh.indices[i++] = curr;
            mesh.indices[i++] = next;
            mesh.indices[i++] = nextUp;
            */

            // CCW triangle winding (correct for backface culling)
            mesh.indices[i++] = curr;
            mesh.indices[i++] = currUp;
            mesh.indices[i++] = nextUp;

            mesh.indices[i++] = curr;
            mesh.indices[i++] = nextUp;
            mesh.indices[i++] = next;
        }
    }

    // Bottom cap
    for (int s = 0; s < segments; ++s)
    {
        int next = (s + 1) % segments;

        /* Clockwise
        mesh.indices[i++] = bottomCenterIndex;
        mesh.indices[i++] = next;
        mesh.indices[i++] = s;
        */

        mesh.indices[i++] = bottomCenterIndex;
        mesh.indices[i++] = s;
        mesh.indices[i++] = next;
    }

    // Top cap
    int topStart = (circleCount - 1) * segments;
    for (int s = 0; s < segments; ++s)
    {
        int next = (s + 1) % segments;

        /* Clockwise
        mesh.indices[i++] = topCenterIndex;
        mesh.indices[i++] = topStart + s;
        mesh.indices[i++] = topStart + next;
        */

        mesh.indices[i++] = topCenterIndex;
        mesh.indices[i++] = topStart + next;
        mesh.indices[i++] = topStart + s;
    }

    return mesh;
}

// Linear interpolation
static float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Subdivide circles along Y
Circle3D *subdivideCircles(const Circle3D *circles, int circleCount, int subdivisions, int *outCount)
{
    if (circleCount < 2 || subdivisions < 1)
    {
        *outCount = circleCount;
        Circle3D *copy = malloc(sizeof(Circle3D) * circleCount);
        for (int i = 0; i < circleCount; ++i)
            copy[i] = circles[i];
        return copy;
    }

    int totalCircles = (circleCount - 1) * (subdivisions + 1) + 1;
    Circle3D *result = malloc(sizeof(Circle3D) * totalCircles);
    int idx = 0;

    for (int i = 0; i < circleCount - 1; ++i)
    {
        Circle3D c0 = circles[i];
        Circle3D c1 = circles[i + 1];

        // add the first circle
        result[idx++] = c0;

        // add interpolated circles
        for (int s = 1; s <= subdivisions; ++s)
        {
            float t = (float)s / (subdivisions + 1);
            Circle3D c;
            c.center_x = lerp(c0.center_x, c1.center_x, t);
            c.center_y = lerp(c0.center_y, c1.center_y, t);
            c.center_z = lerp(c0.center_z, c1.center_z, t);
            c.radius = lerp(c0.radius, c1.radius, t);
            c.normal_x = lerp(c0.normal_x, c1.normal_x, t);
            c.normal_y = lerp(c0.normal_y, c1.normal_y, t);
            c.normal_z = lerp(c0.normal_z, c1.normal_z, t);
            result[idx++] = c;
        }
    }

    // add last circle
    result[idx++] = circles[circleCount - 1];

    *outCount = totalCircles;
    return result;
}

void centerMesh(Mesh3D *mesh, float target_x, float target_y, float target_z)
{
    if (!mesh || !mesh->vertices || mesh->vertexCount <= 0)
        return;

    float minX = 1e9f, minY = 1e9f, minZ = 1e9f;
    float maxX = -1e9f, maxY = -1e9f, maxZ = -1e9f;

    // Compute bounding box
    for (int i = 0; i < mesh->vertexCount; ++i)
    {
        float x = mesh->vertices[i * 3 + 0];
        float y = mesh->vertices[i * 3 + 1];
        float z = mesh->vertices[i * 3 + 2];

        if (x < minX)
            minX = x;
        if (y < minY)
            minY = y;
        if (z < minZ)
            minZ = z;
        if (x > maxX)
            maxX = x;
        if (y > maxY)
            maxY = y;
        if (z > maxZ)
            maxZ = z;
    }

    // Find model center
    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;

    // Compute offset to move center to target
    float offsetX = target_x - centerX;
    float offsetY = target_y - centerY;
    float offsetZ = target_z - centerZ;

    // Apply translation
    for (int i = 0; i < mesh->vertexCount; ++i)
    {
        mesh->vertices[i * 3 + 0] += offsetX;
        mesh->vertices[i * 3 + 1] += offsetY;
        mesh->vertices[i * 3 + 2] += offsetZ;
    }
}

void scaleMeshToFit(Mesh3D *mesh, float targetSize)
{
    if (!mesh || !mesh->vertices || mesh->vertexCount <= 0)
        return;

    float minX = 1e9f, minY = 1e9f, minZ = 1e9f;
    float maxX = -1e9f, maxY = -1e9f, maxZ = -1e9f;

    // Compute bounding box
    for (int i = 0; i < mesh->vertexCount; ++i)
    {
        float x = mesh->vertices[i * 3 + 0];
        float y = mesh->vertices[i * 3 + 1];
        float z = mesh->vertices[i * 3 + 2];

        if (x < minX)
            minX = x;
        if (y < minY)
            minY = y;
        if (z < minZ)
            minZ = z;
        if (x > maxX)
            maxX = x;
        if (y > maxY)
            maxY = y;
        if (z > maxZ)
            maxZ = z;
    }

    // Compute current size and scale
    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float maxSize = sizeX;
    if (sizeY > maxSize)
        maxSize = sizeY;
    if (sizeZ > maxSize)
        maxSize = sizeZ;

    if (maxSize < 1e-6f)
        return; // avoid divide by zero

    float scale = targetSize / maxSize;

    // Apply uniform scale
    for (int i = 0; i < mesh->vertexCount; ++i)
    {
        mesh->vertices[i * 3 + 0] *= scale;
        mesh->vertices[i * 3 + 1] *= scale;
        mesh->vertices[i * 3 + 2] *= scale;
    }
}

void normalizeMesh(Mesh3D *mesh, float target_x, float target_y, float target_z, float targetSize)
{
    if (!mesh || !mesh->vertices || mesh->vertexCount <= 0)
        return;

    float minX = 1e9f, minY = 1e9f, minZ = 1e9f;
    float maxX = -1e9f, maxY = -1e9f, maxZ = -1e9f;

    // Compute bounding box
    for (int i = 0; i < mesh->vertexCount; ++i)
    {
        float x = mesh->vertices[i * 3 + 0];
        float y = mesh->vertices[i * 3 + 1];
        float z = mesh->vertices[i * 3 + 2];

        if (x < minX)
            minX = x;
        if (y < minY)
            minY = y;
        if (z < minZ)
            minZ = z;
        if (x > maxX)
            maxX = x;
        if (y > maxY)
            maxY = y;
        if (z > maxZ)
            maxZ = z;
    }

    // Compute center and size
    float centerX = (minX + maxX) * 0.5f;
    float centerY = (minY + maxY) * 0.5f;
    float centerZ = (minZ + maxZ) * 0.5f;

    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;

    float maxSize = sizeX;
    if (sizeY > maxSize)
        maxSize = sizeY;
    if (sizeZ > maxSize)
        maxSize = sizeZ;

    if (maxSize < 1e-6f)
        return; // prevent divide by zero

    float scale = 1.0f;
    if (targetSize > 0.0f && maxSize > 1e-6f)
    {
        scale = targetSize / maxSize;
    }

    // Apply normalization (translate + scale)
    for (int i = 0; i < mesh->vertexCount; ++i)
    {
        float x = mesh->vertices[i * 3 + 0];
        float y = mesh->vertices[i * 3 + 1];
        float z = mesh->vertices[i * 3 + 2];

        // move to origin first, then scale, then move to target
        x = (x - centerX) * scale + target_x;
        y = (y - centerY) * scale + target_y;
        z = (z - centerZ) * scale + target_z;

        mesh->vertices[i * 3 + 0] = x;
        mesh->vertices[i * 3 + 1] = y;
        mesh->vertices[i * 3 + 2] = z;
    }
}

#include "../deps/csr.h"
#include "../deps/vm.h"

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

/* === Example Usage === */
int main(void)
{

    /* Initial

    Circle3D circles[3];
    circles[0].center_x = 0.0f;
    circles[0].center_y = 0.0f;
    circles[0].center_z = 0.0f;
    circles[0].radius = 1.0f;
    circles[0].normal_x = 0.0f;
    circles[0].normal_y = 0.0f;
    circles[0].normal_z = 1.0f;

    circles[1].center_x = 1.0f;
    circles[1].center_y = 0.0f;
    circles[1].center_z = 1.0f;
    circles[1].radius = 0.8f;
    circles[1].normal_x = 0.2f;
    circles[1].normal_y = 0.0f;
    circles[1].normal_z = 1.0f;

    circles[2].center_x = 2.0f;
    circles[2].center_y = 0.0f;
    circles[2].center_z = 2.0f;
    circles[2].radius = 0.6f;
    circles[2].normal_x = 0.4f;
    circles[2].normal_y = 0.0f;
    circles[2].normal_z = 1.0f;
    */

    /*
        Circle3D circles[8] = {
            {2.0f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f, 1.0f},
            {1.8478f, 0.7654f, 0.5f, 0.8f, 0.0f, 0.0f, 1.0f},
            {1.4142f, 1.4142f, 1.0f, 0.8f, 0.0f, 0.0f, 1.0f},
            {0.7654f, 1.8478f, 1.5f, 0.8f, 0.0f, 0.0f, 1.0f},
            {0.0f, 2.0f, 2.0f, 0.8f, 0.0f, 0.0f, 1.0f},
            {-0.7654f, 1.8478f, 2.5f, 0.8f, 0.0f, 0.0f, 1.0f},
            {-1.4142f, 1.4142f, 3.0f, 0.8f, 0.0f, 0.0f, 1.0f},
            {-1.8478f, 0.7654f, 3.5f, 0.8f, 0.0f, 0.0f, 1.0f}};
            */

    /* pillar     */
    Circle3D circles[5] = {
        // Bottom circle (bigger)
        {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
        // Lower middle
        {0.0f, 1.0f, 0.0f, 0.6f, 0.0f, 1.0f, 0.0f},
        // Center
        {0.0f, 2.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f},
        // Upper middle
        {0.0f, 3.0f, 0.0f, 0.6f, 0.0f, 1.0f, 0.0f},
        // Top circle (bigger)
        {0.0f, 4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f}};

    /*
        Mesh3D mesh = generateMeshFromOrientedCircles3D(
            circles,
            sizeof(circles) / sizeof(circles[0]),
            16);
            */

    int subdividedCount;
    Circle3D *smoothCircles = subdivideCircles(circles, 5, 0, &subdividedCount);

    Mesh3D mesh = generateMeshFromOrientedCircles3D(
        smoothCircles, subdividedCount, 4);

    /*
    centerMesh(&mesh, 0.0f, 0.0f, 0.0f);
    scaleMeshToFit(&mesh, 1.0f);
    */

    normalizeMesh(&mesh, 0.0f, 0.0f, 0.0f, 1.0f);

    /* Define the render area */
#define WIDTH 600
#define HEIGHT 400
#define MEMORY_SIZE (WIDTH * HEIGHT * sizeof(csr_color)) + (WIDTH * HEIGHT * sizeof(float))
    unsigned char memory_total[MEMORY_SIZE] = {0};
    void *memory = (void *)memory_total;

    csr_color clear_color = {40, 40, 40};
    csr_context context = {0};

    if (!csr_init_model(&context, memory, MEMORY_SIZE, WIDTH, HEIGHT))
    {
        return 1;
    }

    v3 world_up = vm_v3(0.0f, 1.0f, 0.0f);
    v3 cam_position = vm_v3(0.0f, 0.6f, 1.0f);
    v3 cam_look_at_pos = vm_v3(0.0f, 0.0f, 0.0f);
    float cam_fov = 90.0f;

    m4x4 projection = vm_m4x4_perspective(vm_radf(cam_fov), (float)context.width / (float)context.height, 0.1f, 1000.0f);
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
        csr_render_clear_screen(&context, clear_color);

        /* Render cube */
        csr_render(
            &context,
            (frame / 50) % 2 == 0
                ? CSR_RENDER_WIREFRAME
                : CSR_RENDER_SOLID,
            CSR_CULLING_CCW_BACKFACE, 3,
            mesh.vertices, mesh.vertexCount,
            mesh.indices, mesh.indexCount,
            model_view_projection.e);

        csr_save_ppm("test_%05d.ppm", frame, &context);
    }

    printf("finished\n");

    return 0;
}
