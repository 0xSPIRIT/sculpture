#include "chisel_blocker.h"

#include <math.h>

#include "grid.h"
#include "globals.h"
#include "util.h"

struct ChiselBlocker chisel_blocker;
int chisel_blocker_mode = 0;

void chisel_blocker_init() {
    chisel_blocker.control_points[0] = (SDL_Point){gw/4, gh/2};
    chisel_blocker.control_points[1] = (SDL_Point){gw/2, gh/2};
    chisel_blocker.control_points[2] = (SDL_Point){3*gw/4, gh/2};
    chisel_blocker.control_points[3] = (SDL_Point){10+3*gw/4, gh/2};
    chisel_blocker.point_count = 4;

    chisel_blocker.side = 1;

    chisel_blocker.render_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);
    SDL_SetTextureBlendMode(chisel_blocker.render_tex, SDL_BLENDMODE_BLEND);
    chisel_blocker.pixels = calloc(gw*gh, sizeof(Uint32));
}

void chisel_blocker_deinit() {
    free(chisel_blocker.pixels);
}

static void flood_fill(Uint32 *pixels, int x, int y, Uint32 value) {
    if (x < 0 || y < 0 || x >= gw || y >= gh || pixels[x+y*gw] != 0) {
        return;
    }

    pixels[x+y*gw] = value;

    flood_fill(pixels, x+1, y, value);
    flood_fill(pixels, x-1, y, value);
    flood_fill(pixels, x, y+1, value);
    flood_fill(pixels, x, y-1, value);
}

void chisel_blocker_tick() {
    static int b_pressed = 0;
    if (current_tool >= TOOL_CHISEL_SMALL && current_tool <= TOOL_CHISEL_MEDIUM && keys[SDL_SCANCODE_C]) {
        if (!b_pressed) {
            chisel_blocker_mode = !chisel_blocker_mode;
            b_pressed = 1;
        }
    } else {
        b_pressed = 0;
    }

    if (!chisel_blocker_mode) return;

    SDL_ShowCursor(1);
    SDL_SetCursor(grabber_cursor);

    static int rpressed = 0;
    if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        if (!rpressed) {
            chisel_blocker.side = (chisel_blocker.side == 1) ? 2 : 1;
            rpressed = 1;
        }
    } else {
        rpressed = 0;
    }

    static int pressed = 0;
    if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (!pressed) {
            pressed = 1;
            int found_point = 0;
            for (int i = 0; i < chisel_blocker.point_count; i++) {
                SDL_Point p = chisel_blocker.control_points[i];
                if (distance(p.x, p.y, mx, my) <= 3) {
                    chisel_blocker.current_point = i;
                    found_point = 1;
                }
            }

            if (!found_point) {
                chisel_blocker.control_points[0].x = mx;
                chisel_blocker.control_points[0].y = my;
                chisel_blocker.control_points[1] = chisel_blocker.control_points[0];
                chisel_blocker.current_point = -1;
            }
        }
        if (chisel_blocker.current_point != -1) {
            SDL_Point *p = &chisel_blocker.control_points[chisel_blocker.current_point];
            p->x = mx;
            p->y = my;
        } else {
            SDL_Point p0 = chisel_blocker.control_points[0];
            SDL_Point p1 = chisel_blocker.control_points[1];
            if (p0.x == p1.x && p0.y == p1.y) {
                int xx = mx, yy = my;
                /* float angle = atan2(xx-p0.x, yy-p0.y); */
                /* float dx = xx-p0.x; */
                /* float dy = yy-p0.y; */
                /* float len = sqrt(dx*dx + dy*dy); */

                /* angle = angle/(2*M_PI); */
                /* angle *= 360; */

                /* angle /= 45.; */
                /* angle = round(angle) * 45; */

                /* printf("Angle: %f\n", angle); fflush(stdout); */

                /* angle = 2*M_PI * angle/360.0; */
                
                /* float ux = sin(angle); */
                /* float uy = cos(angle); */

                /* xx = p0.x + ux * len; */
                /* yy = p0.y + uy * len; */

                chisel_blocker.control_points[chisel_blocker.point_count-1].x = xx;
                chisel_blocker.control_points[chisel_blocker.point_count-1].y = yy;
                chisel_blocker.control_points[chisel_blocker.point_count-2].x = xx;
                chisel_blocker.control_points[chisel_blocker.point_count-2].y = yy;
            }
        }
    } else {
        pressed = 0;
        chisel_blocker.current_point = -1;

        SDL_Point p0 = chisel_blocker.control_points[0];
        SDL_Point p1 = chisel_blocker.control_points[1];
        SDL_Point p3 = chisel_blocker.control_points[3];

        SDL_Point p = (SDL_Point){(p0.x+p3.x)/2, (p0.y+p3.y)/2};

        if (p0.x == p1.x && p0.y == p1.y) {
            chisel_blocker.control_points[1].x = p.x;
            chisel_blocker.control_points[1].y = p.y;
            chisel_blocker.control_points[2].x = p.x;
            chisel_blocker.control_points[2].y = p.y;
        }
    }
}

// Finds tangent at point in cubic bezier curve.
// abcd = P0, P1, P2, P3
static SDL_FPoint bezier_tangent(float t, SDL_Point a, SDL_Point b, SDL_Point c, SDL_Point d) {
    // To do this, we find the derivative.

    // Original bezier:

    // A = lerp(P0, P1, t)
    // B = lerp(P1, P2, t)
    // C = lerp(P2, P3, t)
    // D = lerp(A, B, t)
    // E = lerp(B, C, t)
    // P = lerp(D, E, t)

    // Replacing using math notation:

    // A = (1-t)P0 + P1t
    // B = (1-t)P1 + P2t
    // C = (1-t)P2 + P3t
    // D = (1-t)A + Bt
    // E = (1-t)B + Ct
    // P = (1-t)D + Et

    // Expand:

    // P(t) = P0(-t^3 + 3t^2 - 3t + 1) + P1(3t^3 - 6t^2 + 3t) + P2(-3t^3 + 3t^2) + P3(t^3)

    // Then, find derivative to get eqn. of tangent.
    // P'(t) = P0(-3t^2 + 6t - 3) + P1(9t^2 - 12t + 3) + P2(-9t^2 + 6t) + P3(3t^2)
    // replace with abcd:
    // P'(t) = A(-3t^2 + 6t - 3) + b(9t^2 - 12t + 3) + c(-9t^2 + 6t) + d(3t^2)

    float xp = a.x * (-3 * t*t + 6*t - 3) + b.x * (9*t*t - 12*t + 3) + c.x * (-9*t*t + 6*t) + d.x*(3*t*t);
    float yp = a.y * (-3 * t*t + 6*t - 3) + b.y * (9*t*t - 12*t + 3) + c.y * (-9*t*t + 6*t) + d.y*(3*t*t);

    SDL_FPoint p = {xp/gw, yp/gh};

    float len = sqrt(p.x*p.x + p.y*p.y);
    p.x /= len;
    p.y /= len;

    return p;
}

void chisel_blocker_draw() {
    if (!(current_tool >= TOOL_CHISEL_SMALL && current_tool <= TOOL_CHISEL_MEDIUM))
        return;
    if (chisel_blocker_mode) {
        SDL_SetRenderDrawColor(renderer, 64, 64, 64, 64);
        SDL_RenderFillRect(renderer, NULL);
    }

    SDL_Texture *prev_target = SDL_GetRenderTarget(renderer);
    SDL_SetTextureBlendMode(chisel_blocker.render_tex, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderTarget(renderer, chisel_blocker.render_tex);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    float x, y;
    float px, py;

    // Bezier calculation.
    const float step = 0.001;
    for (float t = 0.0; t < 1.0; t += step) {
        float m1x = lerp(chisel_blocker.control_points[0].x, chisel_blocker.control_points[1].x, t);
        float m1y = lerp(chisel_blocker.control_points[0].y, chisel_blocker.control_points[1].y, t);

        float m2x = lerp(chisel_blocker.control_points[1].x, chisel_blocker.control_points[2].x, t);
        float m2y = lerp(chisel_blocker.control_points[1].y, chisel_blocker.control_points[2].y, t);

        float m3x = lerp(chisel_blocker.control_points[2].x, chisel_blocker.control_points[3].x, t);
        float m3y = lerp(chisel_blocker.control_points[2].y, chisel_blocker.control_points[3].y, t);

        float pax = lerp(m1x, m2x, t);
        float pay = lerp(m1y, m2y, t);
        float pbx = lerp(m2x, m3x, t);
        float pby = lerp(m2y, m3y, t);

        if (t != 0.0) {
            px = x;
            py = y;
        }
        x = lerp(pax, pbx, t);
        y = lerp(pay, pby, t);

        if (t == 0.0) {
            px = x;
            py = y;
        }

        if ((int)x == (int)px && (int)y == (int)py) continue;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        /* SDL_RenderDrawPoint(renderer, (int)x, (int)y); */
        SDL_RenderDrawLine(renderer, (int)px, (int)py, (int)x, (int)y);
    }

    SDL_Point p0 = chisel_blocker.control_points[0];
    SDL_Point p1 = chisel_blocker.control_points[1];
    if (!(p0.x == p1.x && p0.y == p1.y)) {
        {
            SDL_FPoint start = bezier_tangent(0, chisel_blocker.control_points[0], chisel_blocker.control_points[1], chisel_blocker.control_points[2], chisel_blocker.control_points[3]);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Point point = chisel_blocker.control_points[0];
            chisel_blocker.lines[0] = (struct Line){ point.x - start.x, point.y - start.y, point.x - gw*start.x, point.y - gw*start.y };
        }
    
        {
            SDL_FPoint end = bezier_tangent(1, chisel_blocker.control_points[0], chisel_blocker.control_points[1], chisel_blocker.control_points[2], chisel_blocker.control_points[3]);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Point point = chisel_blocker.control_points[3];
            chisel_blocker.lines[1] = (struct Line){ point.x, point.y, point.x + gw*end.x, point.y + gw*end.y };
        }
        
        for (int i = 0; i < 2; i++) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            struct Line l = chisel_blocker.lines[i];
            SDL_RenderDrawLine(renderer, l.x1, l.y1, l.x2, l.y2);
        }
    }

    SDL_RenderReadPixels(renderer, NULL, 0, chisel_blocker.pixels, 4*gw);
    SDL_SetRenderTarget(renderer, prev_target);

    int value = 1;
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (chisel_blocker.pixels[x+y*gw] == 0) {
                flood_fill(chisel_blocker.pixels, x, y, value);
                value++;
            }
        }
    }

    SDL_SetTextureBlendMode(chisel_blocker.render_tex, SDL_BLENDMODE_BLEND);
    if (chisel_blocker_mode)
        SDL_SetTextureAlphaMod(chisel_blocker.render_tex, 255);
    else
        SDL_SetTextureAlphaMod(chisel_blocker.render_tex, 128);

    SDL_RenderCopy(renderer, chisel_blocker.render_tex, NULL, NULL);
    
    for (int i = 0; i < gw*gh; i++) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 16);
        if (chisel_blocker.pixels[i] != chisel_blocker.side) {
            SDL_RenderDrawPoint(renderer, i%gw, i/gw);
        }
    }

    if (!chisel_blocker_mode) return;

    for (int i = 0; i < chisel_blocker.point_count; i++) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawPoint(renderer, chisel_blocker.control_points[i].x, chisel_blocker.control_points[i].y);
        SDL_RenderDrawPoint(renderer, chisel_blocker.control_points[i].x-1, chisel_blocker.control_points[i].y);
        SDL_RenderDrawPoint(renderer, chisel_blocker.control_points[i].x+1, chisel_blocker.control_points[i].y);
        SDL_RenderDrawPoint(renderer, chisel_blocker.control_points[i].x, chisel_blocker.control_points[i].y+1);
        SDL_RenderDrawPoint(renderer, chisel_blocker.control_points[i].x, chisel_blocker.control_points[i].y-1);
    }
}
