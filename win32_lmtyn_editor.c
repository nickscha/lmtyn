#include "lmtyn.h"        /* The modelling algorithm */
#include "lmtyn_editor.h" /* The editor/visualization and input processing */
#include "deps/csr.h"     /* Simple Software Renderer */
#include "deps/vm.h"      /* Linear Algebra / Vector Math Library */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

LMTYN_API u8 csr_init(csr_context *ctx, u32 width, u32 height)
{
    u32 memory_size = (u32)csr_memory_size((int)width, (int)height);
    void *memory = (void *)malloc(memory_size);

    if (!memory)
    {
        return 0;
    }

    if (!csr_init_model(ctx, memory, memory_size, (i32)width, (i32)height))
    {
        return 0;
    }

    return 1;
}

LMTYN_API void csr_render_mesh(csr_context *ctx, lmtyn_mesh *mesh, v3 cam_position, v3 model_position, u32 frame)
{
    v3 world_up = vm_v3(0.0f, 1.0f, 0.0f);
    v3 cam_look_at_pos = vm_v3(0.0f, 0.0f, 0.0f);
    f32 cam_fov = 90.0f;
    v3 model_rotation_y = vm_v3(0.0f, 1.0f, 0.0);

    m4x4 projection = vm_m4x4_perspective(vm_radf(cam_fov), (f32)ctx->width / (f32)ctx->height, 0.1f, 1000.0f);
    m4x4 view = vm_m4x4_lookAt(cam_position, cam_look_at_pos, world_up);
    m4x4 projection_view = vm_m4x4_mul(projection, view);
    m4x4 model_base = vm_m4x4_translate(vm_m4x4_identity, model_position);
    m4x4 model_view_projection = vm_m4x4_mul(projection_view, model_base);

    /* Render cube */
    csr_render(
        ctx,
        CSR_RENDER_SOLID,
        CSR_CULLING_CCW_BACKFACE, 3,
        mesh->vertices, mesh->vertices_size,
        (int *)mesh->indices, mesh->indices_size,
        model_view_projection.e);
}

LMTYN_API void csr_blit_scaled(csr_context *ctx, lmtyn_editor *editor)
{
    lmtyn_editor_framebuffer_region *r = &editor->regions[LMTYN_EDITOR_FRAMEBUFFER_REGION_RENDER];

    u32 y;
    u32 x;

    for (y = 0; y < r->h; ++y)
    {
        u32 fb_y = r->y + y;

        if (fb_y >= editor->framebuffer_height)
        {
            break;
        }

        for (x = 0; x < r->w; ++x)
        {
            u32 fb_x = r->x + x;

            /* Sample CSR pixel at pixel center and flip Y */
            u32 src_x = (u32)((x + 0.5f) * ctx->width / r->w);
            u32 src_y = (u32)((y + 0.5f) * ctx->height / r->h);

            csr_color *src;

            if (fb_x >= editor->framebuffer_width)
            {
                break;
            }

            if (src_x >= ctx->width)
            {
                src_x = ctx->width - 1;
            }

            if (src_y >= ctx->height)
            {
                src_y = ctx->height - 1;
            }

            src = &ctx->framebuffer[src_y * ctx->width + src_x];

            editor->framebuffer[fb_y * editor->framebuffer_width + fb_x] = (src->r << 16) | (src->g << 8) | (src->b);
        }
    }
}

LMTYN_API void win32_lmtyn_editor_resize_framebuffer(lmtyn_editor *editor, i32 new_w, i32 new_h, BITMAPINFO *bmi, csr_context *ctx)
{
    if (new_w <= 0 || new_h <= 0)
    {
        return;
    }

    if (editor->framebuffer)
    {
        free(editor->framebuffer);
    }

    editor->framebuffer_width = new_w;
    editor->framebuffer_height = new_h;
    editor->framebuffer = (u32 *)malloc(sizeof(u32) * new_w * new_h);

    bmi->bmiHeader.biWidth = new_w;
    bmi->bmiHeader.biHeight = -new_h; /* top-down */

    editor->regions_split_x = editor->framebuffer_width / 2;
    editor->regions_split_y = editor->framebuffer_height / 2;

    lmtyn_editor_regions_update(editor);

    /* CSR Render Buffer */
    {
        u32 memory_size;
        void *memory;

        if (ctx->framebuffer || ctx->zbuffer)
        {
            free(ctx->framebuffer);
        }

        memory_size = (u32)csr_memory_size((i32)new_w, (i32)new_h);
        memory = (void *)malloc(memory_size);

        csr_init_model(ctx, memory, memory_size, (i32)new_w, (i32)new_h);
    }
}

LMTYN_API void win32_lmtyn_editor_draw_region_labels(lmtyn_editor *editor, HDC hdc)
{
    u32 i;

    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);

    for (i = 0; i < LMYTN_EDITOR_FRAMEBUFFER_REGION_COUNT - 1; ++i)
    {
        lmtyn_editor_framebuffer_region *region = &editor->regions[i];

        char *axis_up;
        char *axis_right;

        if (i == LMTYN_EDITOR_FRAMEBUFFER_REGION_XZ)
        {
            axis_up = "Z";
            axis_right = "X";
        }

        if (i == LMTYN_EDITOR_FRAMEBUFFER_REGION_YZ)
        {
            axis_up = "Z";
            axis_right = "Y";
        }

        if (i == LMTYN_EDITOR_FRAMEBUFFER_REGION_XY)
        {
            axis_up = "Y";
            axis_right = "X";
        }

        TextOutA(hdc, region->x + 20, region->y + region->h - 20, axis_right, 1);
        TextOutA(hdc, region->x + 5, region->y + region->h - 40, axis_up, 1);
    }
}

typedef struct win32_lmtyn_editor_state
{
    lmtyn_editor *editor;
    lmtyn_editor_input *input;
    lmtyn_mesh *mesh;
    BITMAPINFO *bmi;
    csr_context *ctx;

} win32_lmtyn_editor_state;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    win32_lmtyn_editor_state *win32_state = (win32_lmtyn_editor_state *)GetWindowLongPtrA(hWnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;
    case WM_CREATE:
    {
        CREATESTRUCTA *cs = (CREATESTRUCTA *)lParam;
        win32_state = (win32_lmtyn_editor_state *)cs->lpCreateParams;
        SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)win32_state);
    }
    case WM_SIZE:
    {
        i32 new_w = LOWORD(lParam);
        i32 new_h = HIWORD(lParam);

        win32_lmtyn_editor_resize_framebuffer(win32_state->editor, new_w, new_h, win32_state->bmi, win32_state->ctx);
        win32_state->input->framebuffer_size_changed = 1;

        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }
    case WM_LBUTTONDOWN:
        win32_state->input->mouse_left.down = 1;
        win32_state->input->mouse_x = LOWORD(lParam);
        win32_state->input->mouse_y = HIWORD(lParam);
        SetCapture(hWnd); /* Ensures drag outside window still tracked */
        return 0;

    case WM_LBUTTONUP:
        win32_state->input->mouse_left.down = 0;
        ReleaseCapture();
        return 0;

    case WM_MOUSEMOVE:
        win32_state->input->mouse_x = LOWORD(lParam);
        win32_state->input->mouse_y = HIWORD(lParam);
        return 0;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        u8 wasDown = (u8)((lParam & ((u32)1 << 30)) != 0);
        u8 isDown = (u8)((lParam & ((u32)1 << 31)) == 0);

        lmtyn_editor_key_state *key = NULL;

        switch (wParam)
        {
        case VK_CONTROL:
            key = &win32_state->input->key_control;
            break;
        case VK_LEFT:
            key = &win32_state->input->key_left;
            break;
        case VK_RIGHT:
            key = &win32_state->input->key_right;
            break;
        case VK_UP:
            key = &win32_state->input->key_up;
            break;
        case VK_DOWN:
            key = &win32_state->input->key_down;
            break;
        case 'Z':
            key = &win32_state->input->key_z;
            break;
        case 'R':
            key = &win32_state->input->key_r;
            break;
        case 'S':
            key = &win32_state->input->key_s;
            break;
        case VK_OEM_PLUS:
        case VK_ADD:
            key = &win32_state->input->key_plus;
            break;
        case VK_OEM_MINUS:
        case VK_SUBTRACT:
            key = &win32_state->input->key_minus;
            break;
        }

        if (key)
        {
            key->was_down = wasDown;
            key->down = isDown;
        }

        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

i32 WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, i32 nShow)
{
    u32 width = 800;
    u32 height = 800;

    csr_context ctx = {0};
    lmtyn_editor editor = {0};
    lmtyn_editor_input editor_input = {0};

    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

#define CIRCLES_CAPACITY 1024
    lmtyn_shape_circle circles[CIRCLES_CAPACITY];

    circles[0].center_x = 0.0f;
    circles[0].center_y = 0.0f;
    circles[0].center_z = 0.0f;
    circles[0].radius = 1.0f;
    editor.circles_count = 1;

    win32_lmtyn_editor_resize_framebuffer(&editor, width, height, &bmi, &ctx);

    lmtyn_editor_initialize(
        &editor,
        editor.framebuffer,
        width,
        height,
        circles,
        CIRCLES_CAPACITY);

    lmtyn_mesh mesh = {0};
    mesh.vertices_capacity = sizeof(f32) * 4096;
    mesh.indices_capacity = sizeof(u32) * 4096;
    mesh.vertices = (f32 *)malloc(sizeof(f32) * mesh.vertices_capacity);
    mesh.indices = (u32 *)malloc(sizeof(u32) * mesh.indices_capacity);

    win32_lmtyn_editor_state state = {0};
    state.editor = &editor;
    state.input = &editor_input;
    state.mesh = &mesh;
    state.bmi = &bmi;
    state.ctx = &ctx;

    WNDCLASS wc;
    MSG msg;
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = 0;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = "FBWindowClass";
    RegisterClass(&wc);

    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, "lmtyn",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        0, 0,
        rect.right - rect.left, rect.bottom - rect.top,
        0, 0,
        hInst,
        &state);

    /* Center the mouse in the XY framebuffer region */
    {
        lmtyn_editor_framebuffer_region *xy_region = &editor.regions[LMTYN_EDITOR_FRAMEBUFFER_REGION_XY];
        i32 center_x = xy_region->x + (int)(xy_region->w / 2);
        i32 center_y = xy_region->y + (int)(xy_region->h / 2);
        POINT pt = {center_x, center_y};
        ClientToScreen(hwnd, &pt);
        SetCursorPos(pt.x, pt.y);
        editor_input.mouse_x = center_x;
        editor_input.mouse_y = center_y;
    }

    HDC hdc = GetDC(hwnd);

    csr_color clear_color = {40, 40, 40};
    v3 cam_position = vm_v3(0.0f, 0.0f, 1.0f);

    u32 frame;

    for (;;)
    {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        lmtyn_editor_render(&editor, &editor_input);

        /* Draw 3D Model */
        if (editor.circles_count > 1)
        {
            mesh.vertices_size = 0;
            mesh.indices_size = 0;

            lmtyn_mesh_generate(&mesh, 0, circles, editor.circles_count, 4);
            lmtyn_mesh_normalize(&mesh, 0.0f, 0.0f, 0.0f, 1.0f);

            csr_render_clear_screen(&ctx, clear_color);
            csr_render_mesh(&ctx, &mesh, cam_position, vm_v3_zero, frame);
            csr_blit_scaled(&ctx, &editor);

            lmtyn_editor_draw_borders(&editor);
        }

        StretchDIBits(hdc,
                      0, 0, editor.framebuffer_width, editor.framebuffer_height,
                      0, 0, editor.framebuffer_width, editor.framebuffer_height,
                      editor.framebuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);

        win32_lmtyn_editor_draw_region_labels(&editor, hdc);

        Sleep(16);

        frame++;
    }
}
