#ifndef LMTYN_EDITOR_H
#define LMTYN_EDITOR_H

#include "lmtyn.h"

typedef enum lmtyn_editor_regions
{
    LMTYN_EDITOR_REGION_XZ = 0,
    LMTYN_EDITOR_REGION_YZ,
    LMTYN_EDITOR_REGION_XY,
    LMTYN_EDITOR_REGION_RENDER,
    LMTYN_EDITOR_REGION_TOOLBAR,
    LMTYN_EDITOR_REGION_COUNT

} lmtyn_editor_regions;

typedef struct lmtyn_editor_region
{
    u32 x;
    u32 y;
    u32 w;
    u32 h;

    u32 color_background;

    f32 grid_scroll_offset_x;
    f32 grid_scroll_offset_y;

} lmtyn_editor_region;

typedef struct lmtyn_editor
{
    u32 *framebuffer;
    u32 framebuffer_width;
    u32 framebuffer_height;

    lmtyn_editor_region regions[LMTYN_EDITOR_REGION_COUNT];
    i32 regions_selected_region_index;

    /* Horizontal and vertical dividers of the regions */
    u32 regions_split_x;
    u32 regions_split_y;
    u32 regions_split_size_min;
    u32 regions_split_size_factor;

    u32 regions_color_border;
    u32 regions_color_border_selected;

    u8 snap_enabled;
    f32 snap_interval;

    f32 grid_scale;     /* 10.0f */
    f32 grid_cell_size; /* 1.0f */
    f32 grid_scroll_speed;
    u32 grid_color;
    u32 grid_color_axis;

    /* Data storage */
    lmtyn_shape_circle *circles;
    u32 circles_capacity;
    u32 circles_count;
    u32 circles_selected_circle_index;
    u32 circles_color;
    u32 circles_color_line;
    u32 circles_color_selected;

    f32 circles_last_x;
    f32 circles_last_y;
    f32 circles_last_z;

} lmtyn_editor;

typedef struct lmtyn_editor_key_state
{

    u8 down;     /* Is currently held down */
    u8 pressed;  /* Became pressed this frame (edge trigger) */
    u8 active;   /* Toggle state (flips when pressed) */
    u8 was_down; /* Previous frame’s state */

} lmtyn_editor_key_state;

typedef struct lmtyn_editor_input
{
    u8 framebuffer_size_changed;

    lmtyn_editor_key_state key_control;
    lmtyn_editor_key_state key_left;
    lmtyn_editor_key_state key_right;
    lmtyn_editor_key_state key_up;
    lmtyn_editor_key_state key_down;
    lmtyn_editor_key_state key_z;
    lmtyn_editor_key_state key_r;
    lmtyn_editor_key_state key_s;
    lmtyn_editor_key_state key_plus;
    lmtyn_editor_key_state key_minus;

    lmtyn_editor_key_state mouse_left;
    lmtyn_editor_key_state mouse_right;

    u32 mouse_x;
    u32 mouse_y;

} lmtyn_editor_input;

LMTYN_API LMTYN_INLINE u32 lmtyn_absi(u32 x)
{
    return (x < 0 ? -x : x);
}

LMTYN_API LMTYN_INLINE f32 lmtyn_floorf(f32 x)
{
    i32 i = (i32)x;
    return (x < (f32)i) ? (f32)(i - 1) : (f32)i;
}

LMTYN_API LMTYN_INLINE f32 lmtyn_ceilf(f32 x)
{
    i32 i = (i32)x;
    return (x > (f32)i) ? (f32)(i + 1) : (f32)i;
}

LMTYN_API LMTYN_INLINE f32 lmtyn_roundf(f32 x)
{
    /* If positive, add 0.5 and truncate. If negative, subtract 0.5 and truncate. */
    return (x >= 0.0f) ? (f32)((i32)(x + 0.5f)) : (f32)((i32)(x - 0.5f));
}

LMTYN_API LMTYN_INLINE f32 lmtyn_snap(f32 v, f32 interval)
{
    return lmtyn_roundf(v / interval) * interval;
}

LMTYN_API void lmtyn_editor_screen_to_world(
    lmtyn_editor *editor,
    u32 region_index,
    u32 sx, u32 sy, f32 *wx, f32 *wy)
{
    lmtyn_editor_region *r = &editor->regions[region_index];

    /* Normalize screen coordinates to [-1, 1] */
    f32 nx = ((sx - r->x) / (f32)r->w - 0.5f) * 2.0f;
    f32 ny = ((sy - r->y) / (f32)r->h - 0.5f) * 2.0f;

    if (region_index == LMTYN_EDITOR_REGION_XY)
    {
        ny = -ny;
    }

    /* Convert to world coordinates considering scroll and scale */
    *wx = nx * editor->grid_scale + r->grid_scroll_offset_x;
    *wy = ny * editor->grid_scale + r->grid_scroll_offset_y;
}

LMTYN_API void lmtyn_editor_world_to_screen(
    lmtyn_editor *editor,
    u32 region_index,
    f32 wx, f32 wy, u32 *sx, u32 *sy)
{
    lmtyn_editor_region *r = &editor->regions[region_index];

    /* Normalize to [-1, 1] */
    f32 nx = (wx - r->grid_scroll_offset_x) / editor->grid_scale;
    f32 ny = (wy - r->grid_scroll_offset_y) / editor->grid_scale;

    /* Y positive world is negative in screen space */
    if (region_index == LMTYN_EDITOR_REGION_XY)
    {
        ny = -ny;
    }

    /* Map to [0, 1] range */
    nx = (nx * 0.5f) + 0.5f;
    ny = (ny * 0.5f) + 0.5f;

    /* Convert to pixel coordinates */
    *sx = r->x + (u32)(nx * r->w);
    *sy = r->y + (u32)(ny * r->h);
}

LMTYN_API void lmtyn_editor_draw_circle(
    lmtyn_editor *editor,
    u32 region_index,
    i32 cx,
    i32 cy,
    i32 radius,
    u32 color)
{
    lmtyn_editor_region *r = &editor->regions[region_index];

    u32 fb_w = editor->framebuffer_width;
    u32 fb_h = editor->framebuffer_height;

    i32 x = radius;
    i32 y = 0;
    i32 err = 1 - x;

    while (x >= y)
    {
        i32 pts[8][2] = {
            {cx + x, cy + y},
            {cx + y, cy + x},
            {cx - y, cy + x},
            {cx - x, cy + y},
            {cx - x, cy - y},
            {cx - y, cy - x},
            {cx + y, cy - x},
            {cx + x, cy - y}};

        for (int i = 0; i < 8; ++i)
        {
            i32 px = pts[i][0];
            i32 py = pts[i][1];

            if (px >= (i32)r->x && px < (i32)(r->x + r->w) &&
                py >= (i32)r->y && py < (i32)(r->y + r->h) &&
                px >= 0 && px < fb_w &&
                py >= 0 && py < fb_h)
            {
                editor->framebuffer[py * editor->framebuffer_width + px] = color;
            }
        }

        y++;

        if (err < 0)
        {
            err += 2 * y + 1;
        }
        else
        {
            x--;
            err += 2 * (y - x + 1);
        }
    }
}

LMTYN_API void lmtyn_editor_draw_background(
    lmtyn_editor *editor,
    u32 region_index)
{
    lmtyn_editor_region *r = &editor->regions[region_index];

    u32 x;
    u32 y;

    for (y = r->y; y < r->y + r->h; ++y)
    {
        u32 *row = editor->framebuffer + y * editor->framebuffer_width + r->x;

        for (x = 0; x < r->w; ++x)
        {
            row[x] = r->color_background;
        }
    }
}

LMTYN_API void lmtyn_editor_draw_borders(
    lmtyn_editor *editor)
{
    u32 x;
    u32 y;

    if (editor->regions_selected_region_index >= 0)
    {
        /* Selected region */
        lmtyn_editor_region *r = &editor->regions[editor->regions_selected_region_index];

        /* Top & Bottom edges */
        for (x = r->x; x < r->x + r->w; ++x)
        {
            u32 yb = r->y + r->h - 1;

            if (r->y >= 0 && r->y < editor->framebuffer_height)
            {
                editor->framebuffer[r->y * editor->framebuffer_width + x] = editor->regions_color_border_selected;
            }

            if (yb >= 0 && yb < editor->framebuffer_height)
            {
                editor->framebuffer[yb * editor->framebuffer_width + x] = editor->regions_color_border_selected;
            }
        }

        /* Left & Right edges */
        for (y = r->y; y < r->y + r->h; ++y)
        {
            u32 xr = r->x + r->w - 1;

            if (r->x >= 0 && r->x < editor->framebuffer_width)
            {
                editor->framebuffer[y * editor->framebuffer_width + r->x] = editor->regions_color_border_selected;
            }

            if (xr >= 0 && xr < editor->framebuffer_width)
            {
                editor->framebuffer[y * editor->framebuffer_width + xr] = editor->regions_color_border_selected;
            }
        }
    }
}

LMTYN_API void lmtyn_editor_draw_grid(
    lmtyn_editor *editor,
    u32 region_index)
{
    lmtyn_editor_region *r = &editor->regions[region_index];
    u32 fb_w = editor->framebuffer_width;
    u32 fb_h = editor->framebuffer_height;

    f32 cx = r->x + r->w * 0.5f;
    f32 cy = r->y + r->h * 0.5f;

    f32 pixels_per_unit_x = r->w / (2.0f * editor->grid_scale);
    f32 pixels_per_unit_y = r->h / (2.0f * editor->grid_scale);

    /* Compute visible world range */
    f32 half_w_units = editor->grid_scale;
    f32 half_h_units = editor->grid_scale;

    f32 world_left = -half_w_units + r->grid_scroll_offset_x;
    f32 world_right = half_w_units + r->grid_scroll_offset_x;
    f32 world_bottom = -half_h_units + r->grid_scroll_offset_y;
    f32 world_top = half_h_units + r->grid_scroll_offset_y;

    i32 start_x = (i32)lmtyn_floorf(world_left / editor->grid_cell_size);
    i32 end_x = (i32)lmtyn_ceilf(world_right / editor->grid_cell_size);
    i32 start_y = (i32)lmtyn_floorf(world_bottom / editor->grid_cell_size);
    i32 end_y = (i32)lmtyn_ceilf(world_top / editor->grid_cell_size);

    i32 gx, gy;
    u32 x, y;

    /* Vertical lines */
    for (gx = start_x; gx <= end_x; ++gx)
    {
        f32 wx = gx * editor->grid_cell_size;
        i32 px = (i32)(cx + (wx - r->grid_scroll_offset_x) * pixels_per_unit_x);

        if (px < (i32)r->x || px >= (i32)(r->x + r->w))
        {
            continue;
        }

        for (y = r->y; y < r->y + r->h; ++y)
        {
            editor->framebuffer[y * fb_w + px] = editor->grid_color;
        }
    }

    /* Horizontal lines */
    for (gy = start_y; gy <= end_y; ++gy)
    {
        f32 wy = gy * editor->grid_cell_size;
        i32 py = (i32)(cy - (wy - r->grid_scroll_offset_y) * pixels_per_unit_y); /* flip Y */

        if (py < (i32)r->y || py >= (i32)(r->y + r->h))
        {
            continue;
        }

        for (x = r->x; x < r->x + r->w; ++x)
            editor->framebuffer[py * fb_w + x] = editor->grid_color;
    }

    /* Axis lines at world (0,0) */
    i32 axis_px = (i32)(cx - r->grid_scroll_offset_x * pixels_per_unit_x);
    i32 axis_py = (i32)(cy + r->grid_scroll_offset_y * pixels_per_unit_y); /* Y flip */

    if (axis_px >= (i32)r->x && axis_px < (i32)(r->x + r->w))
    {
        for (y = r->y; y < r->y + r->h; ++y)
        {
            editor->framebuffer[y * fb_w + axis_px] = editor->grid_color_axis;
        }
    }

    if (axis_py >= (i32)r->y && axis_py < (i32)(r->y + r->h))
    {
        for (x = r->x; x < r->x + r->w; ++x)
        {
            editor->framebuffer[axis_py * fb_w + x] = editor->grid_color_axis;
        }
    }
}

LMTYN_API void lmtyn_editor_draw_circles(lmtyn_editor *editor)
{
    u32 i;
    u32 c;

    for (i = 0; i < LMTYN_EDITOR_REGION_COUNT - 1; ++i)
    {
        lmtyn_editor_region *r = &editor->regions[i];

        for (c = 0; c < editor->circles_count; ++c)
        {
            lmtyn_shape_circle *circle = &editor->circles[c];
            lmtyn_shape_circle *circle_prev = (c > 0) ? &editor->circles[c - 1] : (void *)0;

            f32 a = 0.0f;
            f32 b = 0.0f;

            f32 a_prev = 0.0f;
            f32 b_prev = 0.0f;

            u32 px;
            u32 py;
            u32 pr;

            if (i == LMTYN_EDITOR_REGION_XZ)
            {
                a = circle->center_x;
                b = circle->center_z;

                if (circle_prev)
                {
                    a_prev = circle_prev->center_x;
                    b_prev = circle_prev->center_z;
                }
            }
            else if (i == LMTYN_EDITOR_REGION_YZ)
            {
                a = circle->center_y;
                b = circle->center_z;

                if (circle_prev)
                {
                    a_prev = circle_prev->center_y;
                    b_prev = circle_prev->center_z;
                }
            }
            else if (i == LMTYN_EDITOR_REGION_XY)
            {
                a = circle->center_x;
                b = circle->center_y;

                if (circle_prev)
                {
                    a_prev = circle_prev->center_x;
                    b_prev = circle_prev->center_y;
                }
            }

            lmtyn_editor_world_to_screen(editor, i, a, b, &px, &py);

            pr = (u32)(circle->radius * (r->w / (2.0f * editor->grid_scale)));

            /* TODO: draw line between current and previous circle using circle_prev */

            lmtyn_editor_draw_circle(
                editor, i, px, py, pr,
                c == editor->circles_selected_circle_index
                    ? editor->circles_color_selected
                    : editor->circles_color);
        }
    }
}

LMTYN_API void lmtyn_editor_regions_update(
    lmtyn_editor *editor)
{
    lmtyn_editor_region *r_xz = &editor->regions[LMTYN_EDITOR_REGION_XZ];
    lmtyn_editor_region *r_yz = &editor->regions[LMTYN_EDITOR_REGION_YZ];
    lmtyn_editor_region *r_xy = &editor->regions[LMTYN_EDITOR_REGION_XY];
    lmtyn_editor_region *r_render = &editor->regions[LMTYN_EDITOR_REGION_RENDER];

    r_xz->x = 0;
    r_xz->y = 0;
    r_xz->w = editor->regions_split_x;
    r_xz->h = editor->regions_split_y;

    r_yz->x = editor->regions_split_x;
    r_yz->y = 0;
    r_yz->w = editor->framebuffer_width - editor->regions_split_x;
    r_yz->h = editor->regions_split_y;

    r_xy->x = 0;
    r_xy->y = editor->regions_split_y;
    r_xy->w = editor->regions_split_x;
    r_xy->h = editor->framebuffer_height - editor->regions_split_y;

    r_render->x = editor->regions_split_x;
    r_render->y = editor->regions_split_y;
    r_render->w = editor->framebuffer_width - editor->regions_split_x;
    r_render->h = editor->framebuffer_height - editor->regions_split_y;
}

LMTYN_API u8 lmtyn_editor_regions_find_selected_region_index(
    lmtyn_editor *editor,
    u32 x, u32 y)
{
    u32 i;

    for (i = 0; i < LMTYN_EDITOR_REGION_COUNT; ++i)
    {
        lmtyn_editor_region *region = &editor->regions[i];

        if (x >= region->x &&
            y >= region->y &&
            x < region->x + region->w &&
            y < region->y + region->h)
        {
            editor->regions_selected_region_index = i;
            return 1;
        }
    }

    editor->regions_selected_region_index = -1;

    return 0;
}

LMTYN_API u8 lmtyn_editor_initialize(
    lmtyn_editor *editor,
    u32 *framebuffer,
    u32 framebuffer_width,
    u32 framebuffer_height,
    lmtyn_shape_circle *circles,
    u32 circles_capacity)
{
    if (!editor || !framebuffer || !circles || framebuffer_width < 1 || framebuffer_height < 1 || circles_capacity < 1)
    {
        return 0;
    }

    editor->framebuffer = framebuffer;
    editor->framebuffer_width = framebuffer_width;
    editor->framebuffer_height = framebuffer_height;

    editor->grid_scale = 10.0f;
    editor->grid_cell_size = 1.0f;
    editor->grid_scroll_speed = 1.0f;
    editor->grid_color = 0x20404040;
    editor->grid_color_axis = 0x00666666;

    editor->regions[LMTYN_EDITOR_REGION_XZ].color_background = 0x00202040;
    editor->regions[LMTYN_EDITOR_REGION_YZ].color_background = 0x00204020;
    editor->regions[LMTYN_EDITOR_REGION_XY].color_background = 0x00402020;
    editor->regions[LMTYN_EDITOR_REGION_RENDER].color_background = 0x00303030;

    editor->regions_selected_region_index = -1;
    editor->regions_split_x = editor->framebuffer_width / 2;
    editor->regions_split_y = editor->framebuffer_height / 2;
    editor->regions_split_size_min = 100;
    editor->regions_split_size_factor = 20;
    editor->regions_color_border = 0x404040;
    editor->regions_color_border_selected = 0x00FFCE1B;

    editor->snap_enabled = 1;
    editor->snap_interval = 1.0f;

    editor->circles = circles;
    editor->circles_capacity = circles_capacity;
    editor->circles_color = 0x00FFCE1B;
    editor->circles_color_line = 0x00FFFFFF;
    editor->circles_color_selected = 0x00FF0000;

    lmtyn_editor_regions_update(editor);

    return 1;
}

LMTYN_API void lmtyn_editor_input_key_update(lmtyn_editor_key_state *k)
{
    k->pressed = 0;

    /* Edge detect: just pressed this frame */
    if (k->down && !k->was_down)
    {
        k->pressed = 1;
        k->active = !k->active; /* Toggle active state */
    }

    k->was_down = k->down;
}

LMTYN_API void lmtyn_editor_input_keys_update(lmtyn_editor_input *in)
{
    lmtyn_editor_input_key_update(&in->key_control);
    lmtyn_editor_input_key_update(&in->key_left);
    lmtyn_editor_input_key_update(&in->key_right);
    lmtyn_editor_input_key_update(&in->key_up);
    lmtyn_editor_input_key_update(&in->key_down);
    lmtyn_editor_input_key_update(&in->key_z);
    lmtyn_editor_input_key_update(&in->key_r);
    lmtyn_editor_input_key_update(&in->key_s);
    lmtyn_editor_input_key_update(&in->key_plus);
    lmtyn_editor_input_key_update(&in->key_minus);
    lmtyn_editor_input_key_update(&in->mouse_left);
    lmtyn_editor_input_key_update(&in->mouse_right);
}

static u8 initialized;

LMTYN_API void lmtyn_editor_input_update(
    lmtyn_editor *editor,
    lmtyn_editor_input *input)
{
    lmtyn_editor_input_keys_update(input);

    if (input->key_control.down)
    {
        u32 size_min = editor->regions_split_size_min;
        u32 size_factor = editor->regions_split_size_factor;

        if (input->key_left.pressed && editor->regions_split_x > size_min)
        {
            editor->regions_split_x -= size_factor;
        }
        if (input->key_right.pressed && editor->regions_split_x < editor->framebuffer_width - size_min)
        {
            editor->regions_split_x += size_factor;
        }
        if (input->key_up.pressed && editor->regions_split_y > size_min)
        {
            editor->regions_split_y -= size_factor;
        }
        if (input->key_down.pressed && editor->regions_split_y < editor->framebuffer_height - size_min)
        {
            editor->regions_split_y += size_factor;
        }
        if (input->key_z.pressed && editor->circles_count > 0)
        {
            editor->circles_count--;
        }

        lmtyn_editor_regions_update(editor);
    }
    else
    {
        if (editor->regions_selected_region_index >= 0)
        {
            lmtyn_editor_region *r = &editor->regions[editor->regions_selected_region_index];

            switch (editor->regions_selected_region_index)
            {
            case LMTYN_EDITOR_REGION_XZ:
                if (input->key_left.pressed)
                    r->grid_scroll_offset_x -= editor->grid_scroll_speed;
                if (input->key_right.pressed)
                    r->grid_scroll_offset_x += editor->grid_scroll_speed;
                if (input->key_up.pressed)
                    r->grid_scroll_offset_y += editor->grid_scroll_speed; /* Z axis */
                if (input->key_down.pressed)
                    r->grid_scroll_offset_y -= editor->grid_scroll_speed;
                break;

            case LMTYN_EDITOR_REGION_YZ:
                if (input->key_left.pressed)
                    r->grid_scroll_offset_x -= editor->grid_scroll_speed; /* Y axis */
                if (input->key_right.pressed)
                    r->grid_scroll_offset_x += editor->grid_scroll_speed;
                if (input->key_up.pressed)
                    r->grid_scroll_offset_y += editor->grid_scroll_speed; /* Z axis */
                if (input->key_down.pressed)
                    r->grid_scroll_offset_y -= editor->grid_scroll_speed;
                break;

            case LMTYN_EDITOR_REGION_XY:
                if (input->key_left.pressed)
                    r->grid_scroll_offset_x -= editor->grid_scroll_speed; /* X axis */
                if (input->key_right.pressed)
                    r->grid_scroll_offset_x += editor->grid_scroll_speed;
                if (input->key_up.pressed)
                    r->grid_scroll_offset_y += editor->grid_scroll_speed; /* Y axis */
                if (input->key_down.pressed)
                    r->grid_scroll_offset_y -= editor->grid_scroll_speed;
                break;

            default:
                break;
            }
        }

        if (input->key_s.pressed)
        {
            editor->snap_enabled = !editor->snap_enabled;
        }

        if (input->key_r.pressed)
        {
            u32 i;

            editor->circles_count = 2;
            editor->circles_selected_circle_index = 1;
            editor->grid_scale = 10.0f;
            editor->circles[0].center_x = 0.0f;
            editor->circles[0].center_y = 0.0f;
            editor->circles[0].center_z = 0.0f;
            editor->circles[0].radius = 1.0f;

            for (i = 0; i < LMTYN_EDITOR_REGION_COUNT; ++i)
            {
                editor->regions[i].grid_scroll_offset_x = 0.0f;
                editor->regions[i].grid_scroll_offset_y = 0.0f;
            }
        }
    }

    if (input->key_plus.pressed || input->key_minus.pressed)
    {
        f32 grid_scale_factor = input->key_plus.pressed ? 0.9f : 1.1f;

        editor->grid_scale *= grid_scale_factor;

        lmtyn_editor_regions_update(editor);
    }

    lmtyn_editor_regions_find_selected_region_index(editor, input->mouse_x, input->mouse_y);

    if (input->framebuffer_size_changed)
    {
        lmtyn_editor_regions_update(editor);

        input->framebuffer_size_changed = 0;
    }

    /*  Add circle at mouse */
    {
        if (!initialized)
        {
            editor->circles_count++;
            editor->circles_selected_circle_index = editor->circles_count - 1;
            initialized = 1;
        }

        if (editor->regions_selected_region_index >= 0 && editor->regions_selected_region_index != LMTYN_EDITOR_REGION_RENDER)
        {
            lmtyn_editor_region *r = &editor->regions[editor->regions_selected_region_index];

            f32 wx, wy;
            lmtyn_editor_screen_to_world(editor, editor->regions_selected_region_index, input->mouse_x, input->mouse_y, &wx, &wy);

            if (editor->snap_enabled)
            {
                wx = lmtyn_snap(wx, editor->snap_interval);
                wy = lmtyn_snap(wy, editor->snap_interval);
            }

            u32 current_circle_index = editor->circles_count > 0 ? editor->circles_count - 1 : editor->circles_count;

            lmtyn_shape_circle *circle = &editor->circles[current_circle_index];

            editor->circles_selected_circle_index = current_circle_index;

            circle->radius = current_circle_index > 0 ? editor->circles[current_circle_index - 1].radius : 1.0f;

            if (editor->regions_selected_region_index == LMTYN_EDITOR_REGION_XZ)
            {
                circle->center_x = wx;
                circle->center_z = wy;
                circle->center_y = editor->circles_last_y;
            }
            else if (editor->regions_selected_region_index == LMTYN_EDITOR_REGION_YZ)
            {
                circle->center_y = wx;
                circle->center_z = wy;
                circle->center_x = editor->circles_last_x;
            }
            else if (editor->regions_selected_region_index == LMTYN_EDITOR_REGION_XY)
            {
                circle->center_x = wx;
                circle->center_y = wy;
                circle->center_z = editor->circles_last_z;
            }

            if (input->mouse_left.pressed)
            {
                editor->circles_last_x = circle->center_x;
                editor->circles_last_y = circle->center_y;
                editor->circles_last_z = circle->center_z;
                editor->circles_count++;

                /* Set new circle to current position to avoid flickering of positions */
                editor->circles[editor->circles_count - 1].center_x = editor->circles_last_x;
                editor->circles[editor->circles_count - 1].center_y = editor->circles_last_y;
                editor->circles[editor->circles_count - 1].center_z = editor->circles_last_z;
                editor->circles[editor->circles_count - 1].radius = circle->radius;
            }
        }
    }
}

LMTYN_API void lmtyn_editor_render(
    lmtyn_editor *editor,
    lmtyn_editor_input *input)
{
    lmtyn_editor_input_update(editor, input);

    lmtyn_editor_draw_background(editor, LMTYN_EDITOR_REGION_XZ);
    lmtyn_editor_draw_background(editor, LMTYN_EDITOR_REGION_YZ);
    lmtyn_editor_draw_background(editor, LMTYN_EDITOR_REGION_XY);
    lmtyn_editor_draw_background(editor, LMTYN_EDITOR_REGION_RENDER);

    lmtyn_editor_draw_grid(editor, LMTYN_EDITOR_REGION_XZ);
    lmtyn_editor_draw_grid(editor, LMTYN_EDITOR_REGION_YZ);
    lmtyn_editor_draw_grid(editor, LMTYN_EDITOR_REGION_XY);

    lmtyn_editor_draw_borders(editor);

    lmtyn_editor_draw_circles(editor);
}

#endif /* LMTYN_EDITOR_H */