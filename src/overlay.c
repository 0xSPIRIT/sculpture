static Uint8 type_to_outline_color[CELL_TYPE_COUNT*4] = {
    // Type              R    G    B
    CELL_NONE,          255,   0,   0,
    CELL_DIRT,          255,   0,   0,
    CELL_SAND,          109,   9, 121,
    CELL_WATER,          14, 182,  30,
    CELL_ICE,            80,  94, 250,
    CELL_STEAM,          94,  86, 142,
    CELL_WOOD_LOG,       13,   9, 249,
    CELL_WOOD_PLANK,    150,   6, 118,
    CELL_STONE,         255, 255, 255,
    CELL_MARBLE,          0,   0, 255,
    CELL_SANDSTONE,       0, 255, 255,
    CELL_CEMENT,        190,  22,  46,
    CELL_CONCRETE,      127,  57, 185,
    CELL_QUARTZ,         00, 120,   0,
    CELL_GLASS,         255,   0, 255,
    CELL_GRANITE,       255, 127, 255,
    CELL_BASALT,        185, 185,  57,
    CELL_DIAMOND,        70, 230, 134,
    CELL_UNREFINED_COAL, 45,   9, 200,
    CELL_REFINED_COAL,  110, 246, 190,
    CELL_LAVA,            9,  57, 185,
    CELL_SMOKE,         126,  22, 238,
    CELL_DUST,           77,   9, 249,
};

// name_generic is a formatted printf with a %d for the number, from 0 to num-1.
static Overlay_Changes overlay_load_changes(const char *name_generic, int num) {
    Overlay_Changes result = {0};
    result.count = num;

    int w = -1, h = -1;

    for (int i = 0; i < num; i++) {
        char name[64];
        sprintf(name, name_generic, i);

        SDL_Surface *surf = IMG_Load(name);
        Assert(surf);

        if (w == -1) {
            w = surf->w;
            h = surf->h;
        } else {
            Assert(w == surf->w);
            Assert(h == surf->h);
        }

        Assert(w == gs->gw);
        Assert(h == gs->gh);

        result.grids[i] = PushArray(gs->persistent_memory,
                                    surf->w*surf->h,
                                    sizeof(int));

        for (int y = 0; y < surf->h; y++) {
            for (int x = 0; x < surf->w; x++) {
                Uint8 r=0, g=0, b=0;

                if (surf->format->BytesPerPixel == 1) {
                    Uint8 pixel = ((Uint8*)surf->pixels)[x + y * w];
                    SDL_GetRGB(pixel, surf->format, &r, &g, &b);
                } else if (surf->format->BytesPerPixel == 4) {
                    Uint32 pixel = ((Uint32*)surf->pixels)[x + y * w];
                    SDL_GetRGB(pixel, surf->format, &r, &g, &b);
                } else {
                    Assert(0);
                }

                int cell = 0;

                for (int j = 0; j < CELL_TYPE_COUNT; j++) {
                    SDL_Color c = {
                        type_to_rgb_table[j*4 + 1],
                        type_to_rgb_table[j*4 + 2],
                        type_to_rgb_table[j*4 + 3],
                        255
                    };

                    if (r == c.r && g == c.g && b == c.b) {
                        cell = j;
                        break;
                    }
                }

                result.grids[i][x+y*surf->w] = cell;

            }
        }

        SDL_FreeSurface(surf);
    }

    return result;
}

static void overlay_init(void) {
    Overlay *overlay = &gs->overlay;

    overlay->tool = OVERLAY_TOOL_BRUSH;
    overlay->temp = false;

    if (overlay->grid == null) {
        overlay->grid = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
        overlay->temp_grid = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
    }

    memset(overlay->grid, 0, sizeof(int)*gs->gw*gs->gh);
    memset(overlay->temp_grid, 0, sizeof(int)*gs->gw*gs->gh);

    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            overlay->grid[x+y*gs->gw] =
                gs->levels[gs->level_current].desired_grid[x+y*gs->gw].type;
        }
    }

    overlay->temp_x = -1;
    overlay->temp_y = -1;
    overlay->size = 3;

    overlay->show = false;

    overlay->eraser_mode = false;

    overlay->r.x = overlay->r.y = -1;

    memset(&overlay->changes, 0, sizeof(Overlay_Changes));

    switch (gs->level_current+1) {
        case 7: {
            overlay->changes =
                overlay_load_changes(RES_DIR "lvl/changes/lvl7/%d.png", 5);
            break;
        }
    }
}

static void overlay_set_circle(int x, int y, int r, int value) {
    Overlay *overlay = &gs->overlay;

    for (int yy = -r; yy <= r; yy++) {
        for (int xx = -r; xx <= r; xx++) {
            if (xx*xx + yy*yy > r*r) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;

            overlay->grid[x+xx+(y+yy)*gs->gw] = value;
        }
    }
}

static void overlay_set_line(int *grid, int x1, int y1, int x2, int y2, int value) {
    f64 dx = x2-x1;
    f64 dy = y2-y1;

    const f64 clamp = 22.5;

    f64 angle = atan2f(dy, dx);
    angle /= 2 * M_PI;
    angle *= 360;
    angle = ((int)angle) % 360;
    angle = angle / clamp;
    angle = clamp * round(angle);

    angle /= 360;
    angle *= 2 * M_PI;

    f64 deg_angle = Degrees(angle);

    f64 len = distancei(x1, y1, x2, y2);

    f64 ux = 0, uy = 0;

    ux = cos(angle);
    uy = sin(angle);
    if (is_angle_45(deg_angle) || deg_angle == 90 || deg_angle == 180 || deg_angle == 270 || deg_angle == 0) {
        ux = round(ux);
        uy = round(uy);
    }

    if (is_angle_225(deg_angle)) {
        ux *= 2;
        ux = round(ux);
        ux /= 2.0;

        uy *= 2;
        uy = round(uy);
        uy /= 2.0;
    }

    f64 x = x1;
    f64 y = y1;

    f64 curr_dist = 0;
    while (curr_dist <= len) {
        int ix = ceil(x);
        int iy = ceil(y);
        grid[ix+iy*gs->gw] = value;
        x += ux;
        y += uy;

        curr_dist = distance(x, y, x1, y1);
    }
}

static void overlay_flood_fill(int *grid, int x, int y, int value) {
    if (!is_in_bounds(x, y)) return;
    if (grid[x+y*gs->gw]) return;

    grid[x+y*gs->gw] = value;

    overlay_flood_fill(grid, x+1, y, value);
    overlay_flood_fill(grid, x, y+1, value);
    overlay_flood_fill(grid, x-1, y, value);
    overlay_flood_fill(grid, x, y-1, value);
}

static void overlay_set_rectangle(int *grid, SDL_Rect r, int value) {
    if (r.w < 0) {
        r.w *= -1;
        r.x -= r.w;
    }
    if (r.h < 0) {
        r.h *= -1;
        r.y -= r.h;
    }

    for (int yy = r.y; yy <= r.y+r.h; yy++) {
        for (int xx = r.x; xx <= r.x+r.w; xx++) {
            if (is_in_bounds(xx, yy)) {
                grid[xx+yy*gs->gw] = value;
            }
        }
    }
}

static void overlay_swap_to_next(void) {
    Overlay *overlay = &gs->overlay;

    if (overlay->changes.count && overlay->changes.index < overlay->changes.count-1) {
        overlay->changes.index++;
        memcpy(overlay->grid,
               overlay->changes.grids[overlay->changes.index],
               sizeof(int)*gs->gw*gs->gh);
    }
}

static bool has_any_chisel_chiseled(void) {
    if (gs->chisel_small.num_times_chiseled  > 0) return true;
    if (gs->chisel_medium.num_times_chiseled > 0) return true;
    if (gs->chisel_large.num_times_chiseled  > 0) return true;
    return false;
}

static void overlay_swap_tick(void) {
    Overlay *overlay = &gs->overlay;

    if (!overlay->changes.was_grid_none) {
        bool none = true;
        for (int i = 0; i < gs->gw*gs->gh; i++) {
            if (gs->grid[i].type) {
                none = false;
                break;
            }
        }

        if (none) {
            overlay->changes.was_grid_none = true;
        }
    }

    if (overlay->changes.was_grid_none) {
        if (gs->chisel->did_chisel_this_frame)
            overlay->temp = true;
        if (gs->placers[gs->current_placer].did_place_this_frame)
            overlay->temp = true;
    }
    if (overlay->changes.was_grid_none && overlay->temp) {
        if (gs->overlay.show && !gs->gui.popup && !gs->conversions.active) { overlay->changes.temp += 0.02f; }
    }

    const int c = 15;

    if (overlay->changes.temp >= c) {
        overlay_swap_to_next();
        overlay->changes.temp = 0;
    }

    overlay->changes.alpha = overlay->changes.temp/c;
}

static void overlay_tick(void) {
    Overlay *overlay = &gs->overlay;

    memset(overlay->temp_grid, 0, sizeof(int)*gs->gw*gs->gh);

    const f32 speed = 0.1f;
    if (gs->input.keys[SDL_SCANCODE_UP]) {
        overlay->size += speed;
    } else if (gs->input.keys[SDL_SCANCODE_DOWN]) {
        overlay->size -= speed;
    }
    if (overlay->size < 0) overlay->size = 0;

    if (gs->input.keys[SDL_SCANCODE_ESCAPE]) {
        gs->overlay.show = false;
    }

    if (gs->is_mouse_over_any_button) return;
    switch (overlay->tool) {
        case OVERLAY_TOOL_BRUSH: case OVERLAY_TOOL_ERASER_BRUSH: {
            int value = overlay->tool == OVERLAY_TOOL_BRUSH ? 1 : 0;

            if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {

                overlay_set_circle(gs->input.mx, gs->input.my, overlay->size, value);
            }
            break;
        }
        case OVERLAY_TOOL_RECTANGLE: case OVERLAY_TOOL_ERASER_RECTANGLE: {
            int value = overlay->tool == OVERLAY_TOOL_RECTANGLE ? 1 : 0;

            if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                if (overlay->r.x == -1) {
                    overlay->r.x = gs->input.mx;
                    overlay->r.y = gs->input.my;
                }
                overlay->r.w = gs->input.mx - overlay->r.x;
                overlay->r.h = gs->input.my - overlay->r.y;

                if (overlay->tool == OVERLAY_TOOL_ERASER_RECTANGLE) {
                    overlay_set_rectangle(overlay->temp_grid, overlay->r, 2);
                } else {
                    overlay_set_rectangle(overlay->temp_grid, overlay->r, value);
                }
            } else {
                if (overlay->r.x != -1) {
                    overlay_set_rectangle(overlay->grid, overlay->r, value);
                    overlay->r.x = overlay->r.y = -1;
                }
            }
            break;
        }
        case OVERLAY_TOOL_LINE: case OVERLAY_TOOL_SPLINE: {
            if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                if (overlay->temp_x == -1) {
                    overlay->temp_x = gs->input.mx;
                    overlay->temp_y = gs->input.my;
                }

                if (overlay->tool == OVERLAY_TOOL_LINE) {
                    overlay_set_line(overlay->temp_grid,
                                     overlay->temp_x,
                                     overlay->temp_y,
                                     gs->input.mx,
                                     gs->input.my,
                                     1);
                } else {
                    //overlay_set_spline();
                }
            } else if (gs->input.mouse_released[SDL_BUTTON_LEFT]) {
                if (overlay->tool == OVERLAY_TOOL_LINE) {
                    overlay_set_line(overlay->grid,
                                     overlay->temp_x,
                                     overlay->temp_y,
                                     gs->input.mx,
                                     gs->input.my,
                                     1);
                } else {
                    //overlay_set_spline();
                }

                overlay->temp_x = -1;
                overlay->temp_y = -1;
            }
            break;
        }
        case OVERLAY_TOOL_BUCKET: {
            if (gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
                overlay_flood_fill(overlay->grid, gs->input.mx, gs->input.my, 1);
            }
            break;
        }
    }
}

static bool int_array_any_neighbours_not_same(int *array, int x, int y) {
    int a = array[x+y*gs->gw];
    for (int xx = -1; xx <= 1; xx++) {
        for (int yy = -1; yy <= 1; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (abs(xx)-abs(yy) == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (array[x+xx+(y+yy)*gs->gw] != a)
                return true;
        }
    }
    return false;
}

// @Yellow
static void overlay_draw_missed_pixels(int target, int *grid) {
    RenderColor(0, 200, 0, 255 * 0.5*(sin(SDL_GetTicks()/500.0)+1));
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            int i = x+y*gs->gw;

            if (gs->grid[i].type != grid[i]) {
                RenderPointRelative(target, x, y);
            }
        }
    }
}

static void overlay_draw_grid(int target, int *grid, f32 alpha_coeff) {
    f32 alpha;
    alpha = 255;
    alpha *= alpha_coeff;

    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!grid[x+y*gs->gw]) continue;

            Uint8 a = alpha;

            int t = grid[x+y*gs->gw];
            
            SDL_Color c = {
                type_to_outline_color[t*4+1],
                type_to_outline_color[t*4+2],
                type_to_outline_color[t*4+3],
                255
            };

            if (!int_array_any_neighbours_not_same(grid, x, y)) {
                f64 f = (sin(SDL_GetTicks()/750.0)+1)/2.0;
                if (gs->level_current+1 == 7) f *= 0.5;
                a = fmax(0, min(255, alpha - f*30));
            }

#if 0
            if (gs->level_current+1 != 7 && gs->overlay.current_material != -1 && t != gs->overlay.current_material) {
                a /= 4;
            }
            else if (gs->level_current+1 == 7) {
                if (gs->overlay.current_material != -1 && t != gs->overlay.current_material) {
                    a *= 0.67;
                }

                int add = 100;
                c.r = min(255, c.r+add);
                c.g = min(255, c.g+add);
                c.b = min(255, c.b+add);
            }
#endif

            RenderColor(c.r, c.g, c.b, a);

            RenderPointRelative(target, x, y);
        }
    }
}

static void overlay_draw(int target) {
    Overlay *overlay = &gs->overlay;

    if (!overlay->show)
        return;

    if (overlay->changes.index+1 < overlay->changes.count) {
        Assert(overlay->changes.alpha < 1);
        overlay_draw_grid(target,
                          overlay->grid, 1.0f - overlay->changes.alpha);
        overlay_draw_grid(target,
                          overlay->changes.grids[overlay->changes.index+1],
                          overlay->changes.alpha);
    } else {
        overlay_draw_grid(target, overlay->grid, 0.8f);
    }

    if (compare_cells_to_int_count(gs->grid, overlay->grid) <= 15) {
        overlay_draw_missed_pixels(target, overlay->grid);
    }
}
