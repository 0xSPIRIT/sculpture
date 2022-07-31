#include "converter.h"

#include <SDL2/SDL_image.h>

#include "util.h"
#include "globals.h"
#include "gui.h"
#include "placer.h"

struct Converter converter;

static int converter_can_overheat() {
    return (converter.heat_state == HEAT_HOT || converter.heat_state == HEAT_OVERHEATING || converter.heat_state == HEAT_TOO_HOT) && converter.placer_top && converter.placer_top->contains_type == CELL_COAL;
}

static struct Placer *get_current_placer() {
    if (current_placer == -1 || current_tool != TOOL_PLACER) return NULL;
    return placers[current_placer];
}

static int is_holding_placer() {
    struct Placer *placer = get_current_placer();
    if (!placer) return 0;
    return placer != converter.placer_top && placer != converter.placer_bottom;
}

void converter_init() {
    {
        SDL_Surface *surf = IMG_Load("../res/converter_off.png");
        converter.off_w = surf->w;
        converter.off_h = surf->h;
        converter.off_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    {
        SDL_Surface *surf = IMG_Load("../res/converter_on.png");
        converter.on_w = surf->w;
        converter.on_h = surf->h;
        converter.on_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    {
        SDL_Surface *surf = IMG_Load("../res/converter_output.png");
        converter.output_w = surf->w;
        converter.output_h = surf->h;
        converter.output_texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
    converter.w = converter.on_w;
    converter.h = converter.on_h;
    converter.texture = converter.off_texture;
    converter.state = STATE_OFF;
    converter.placer_top = converter.placer_bottom = NULL;
    converter.heat_time_max = 240; // Frames
    converter.heat_state = HEAT_HOT;
    converter.cooldown_time_max = 160;
    converter.cooldown_time_current = -1;
}

void converter_deinit() {
    SDL_DestroyTexture(converter.texture);
}

void converter_tick() {
    struct Placer *placer = get_current_placer();
    
    // Fixing the placers in place which are connected to the converter.
    if (converter.placer_top) {
        converter.placer_top->x = 15;
        converter.placer_top->y = gui.popup_y + 10 + 1;
    }
    if (converter.placer_bottom) {
        converter.placer_bottom->x = 20;
        converter.placer_bottom->y = gui.popup_y + 10 + converter.h + converter.placer_bottom->h - 1;
    } else if (!converter.placer_top && placer) {
        placer->x = mx;
        placer->y = my;
    }

    // Checking if the mouse is over input placer, output placer, or the converter, and
    // setting the overlay accordingly.
    if (converter.placer_top && is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.placer_top->x-converter.placer_top->w/2, converter.placer_top->y-converter.placer_top->h, converter.placer_top->w, converter.placer_top->h})) {
        struct Placer *p = converter.placer_top;
        gui.overlay = (struct Overlay){
            (float)real_mx/S, (float)real_my/S - GUI_H/S
        };
        char string[256] = {0};
        overlay_get_string(p->contains_type, p->contains_amount, string);
        strcpy(gui.overlay.str[0], "Input");
        strcpy(gui.overlay.str[1], string);
        SDL_ShowCursor(0);
    } else if (converter.placer_bottom && is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.placer_bottom->x-converter.placer_bottom->w/2, converter.placer_bottom->y-converter.placer_bottom->h, converter.placer_bottom->w, converter.placer_bottom->h})) {
        struct Placer *p = converter.placer_bottom;
        gui.overlay = (struct Overlay){
            (float)real_mx/S, (float)real_my/S - GUI_H/S
        };
        char string[256] = {0};
        overlay_get_string(p->contains_type, p->contains_amount, string);
        strcpy(gui.overlay.str[0], "Output");
        strcpy(gui.overlay.str[1], string);
        SDL_ShowCursor(0);
    } else if (is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.x, converter.y, converter.w, converter.w})) {
        char string[256] = {0};
        char str_b[256] = {0};
        overlay_get_string(converter.contains_type, converter.contains_amount, string);
        gui.overlay = (struct Overlay){
            (float)real_mx/S, (float)real_my/S - GUI_H/S
        };
        strcpy(gui.overlay.str[0], "Converter");
        switch (converter.heat_state) {
        case HEAT_TOO_HOT:
            sprintf(str_b, "Mode: [TOO HOT] %.2f seconds left", (float)converter.cooldown_time_current/60.0);
            strcpy(gui.overlay.str[1], str_b);
            break;
        case HEAT_OVERHEATING:;
            sprintf(str_b, "Mode: [OVERHEATING] %.2f seconds left", (float)converter.cooldown_time_current/60.0);
            strcpy(gui.overlay.str[1], str_b);
            break;
        case HEAT_HOT:
            strcpy(gui.overlay.str[1], "Mode: [HOT]");
            break;
        case HEAT_COLD:
            strcpy(gui.overlay.str[1], "Mode: [COLD]");
            break;
        }
        strcpy(gui.overlay.str[2], string);
        SDL_ShowCursor(0);
    }

    // Note: we're taking care of this in placer.c now.

    /*  else if (is_holding_placer(converter)) { */
    /*     // We have a placer that we're holding that is not on the top / bottom. */
    /*     // Show the stats of it. */
    /*     gui.overlay = (struct Overlay){ */
    /*         (float)real_mx/S, (float)real_my/S */
    /*     }; */
    /*     char string[256] = {0}; */
    /*     overlay_get_string(placer->contains_type, placer->contains_amount, string); */
    /*     strcpy(gui.overlay.str[0], "Placer"); */
    /*     strcpy(gui.overlay.str[1], string); */
    /*     SDL_ShowCursor(0); */
    /* }  */ else {
        gui.overlay.x = -1;
        gui.overlay.y = -1;
        SDL_ShowCursor(1);
    }

    if (is_holding_placer(converter) && placer) {
        // We have a placer that we're holding that is not on the top / bottom.
        placer_tick(placer);
    }

    // Code that assigns the placers to the converter.
    static int mouse_pressed = 0;
    if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (!mouse_pressed) {
            if (current_tool == TOOL_PLACER) {
                const int give = 5; // px of leeway
                if (is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.x-give, gui.popup_y, converter.w+give, gh-gui.popup_y})) {
                    if (my < (gui.popup_y + 10)+converter.h/2) {
                        if (placer && !converter.placer_top) {
                            converter.placer_top = placer;
                            printf("happened\n"); fflush(stdout);
                        } else if (is_holding_placer(converter)) {
                            // Swap the holding placer and the one in the converter.
                            int i = converter.placer_top->index;
                            converter.placer_top = get_current_placer();
                            current_placer = i;
                        } else if (converter.placer_top) {
                            current_placer = converter.placer_top->index;
                            converter.placer_top = NULL;
                        }
                    } else {
                        if (placer && !converter.placer_bottom) {
                            // Set the bottom one if the conditions are right.
                            if (placer->contains_amount == 0 || placer->contains_type == 0 || placer->contains_type == converter.contains_type) {
                                converter.placer_bottom = placer;
                            }
                        } else if (is_holding_placer(converter) && converter.placer_bottom) {
                            // Swap 'em
                            int i = converter.placer_bottom->index;
                            converter.placer_bottom = get_current_placer();
                            current_placer = i;
                        } else if (converter.placer_bottom) {
                            // Otherwise we take the one from the bottom.
                            current_placer = converter.placer_bottom->index;
                            converter.placer_bottom = NULL;
                        }
                    }
                } else if (is_holding_placer(converter)) { // If we're still holding a placer, but clicking outside...
                    // Unset the holding placer.
                    current_placer = -1;
                }
            }
            mouse_pressed = 1;
        }
    } else {
        mouse_pressed = 0;
    }

    static int mrp = 0;
    if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        if (!mrp) {
            if (converter.state == STATE_OFF && converter.placer_top && converter.heat_state == HEAT_HOT) {
                converter.state = STATE_ON;
            } else {
                converter.state = STATE_OFF;
            }
            mrp = 1;
        }
    } else {
        mrp = 0;
    }

    static int outp = 0;
    if (mouse & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
        if (!outp) {
            if (converter.state != STATE_OUT && converter.contains_amount > 0 && converter.placer_bottom && (converter.placer_bottom->contains_type == converter.contains_type || converter.placer_bottom->contains_amount == 0)) {
                converter.state = STATE_OUT;
                converter.placer_bottom->contains_type = converter.contains_type;
            } else {
                converter.state = STATE_OFF;
            }
            outp = 1;
        }
    } else {
        outp = 0;
    }

    struct Placer *top = converter.placer_top;

    if (converter.state == STATE_ON && !top) {
        converter.state = STATE_OFF;
    }

    float amount_add;
    if (converter.heat_state == HEAT_HOT) {
        amount_add = 1;
    } else if (converter.heat_state == HEAT_OVERHEATING) {
        amount_add = 0.25;
    }

    if (converter.state == STATE_ON && top->contains_amount > 0) {
        int output = converter_convert(top->contains_type);
        if (converter.contains_type != output && converter.contains_amount > 0) {
            converter.state = STATE_OFF; // We cannot continue since we can't output two types.
        } else {
            converter.contains_type = output;
            converter.contains_amount += amount_add;
            top->contains_amount -= amount_add;
            if (top->contains_amount <= 0) {
                converter.state = STATE_OFF;
                top->contains_type = 0;
            }
        }
        if (converter_can_overheat(converter)) {
            converter.heat_time_current -= 1;
            if (converter.heat_state == HEAT_HOT && converter.heat_time_current <= converter.heat_time_max/2) {
                converter.heat_state = HEAT_OVERHEATING;
                if (converter.cooldown_time_current == -1) {
                    converter.cooldown_time_current = converter.cooldown_time_max;
                }
            } else if (converter.heat_state == HEAT_OVERHEATING && converter.heat_time_current <= 0) {
                converter.heat_state = HEAT_TOO_HOT;
                converter.state = STATE_OFF;
                converter.cooldown_time_current = converter.cooldown_time_max;
            }    
        }
    } else if (converter.state == STATE_OUT && converter.contains_amount > 0) {
        if (!converter.placer_bottom) converter.state = STATE_OFF;
        if (converter.placer_bottom->contains_type == 0) {
            converter.placer_bottom->contains_amount = 0;
            converter.placer_bottom->contains_type = converter.contains_type;
        } else if (converter.contains_type != converter.placer_bottom->contains_type) {
            converter.state = STATE_OFF;
        }

        converter.contains_amount -= amount_add;
        converter.placer_bottom->contains_amount += amount_add;

        if (converter.contains_amount <= 0) {
            converter.state = STATE_OFF;
            converter.contains_type = 0;
        }
    }

    if (converter_can_overheat(converter) && converter.cooldown_time_current > 0) {
        converter.cooldown_time_current--;
        if (converter.cooldown_time_current == 0) {
            switch (converter.heat_state) {
            case HEAT_OVERHEATING:
                converter.heat_state = HEAT_HOT;
                converter.cooldown_time_current = -1;
                break; 
            case HEAT_TOO_HOT:
                converter.heat_state = HEAT_OVERHEATING;
                converter.cooldown_time_current = converter.cooldown_time_max;
                break; 
            }
        }
    }

    if (converter.state == STATE_ON) {
        converter.texture = converter.on_texture;
        converter.w = converter.on_w;
        converter.h = converter.on_h;
    } else if (converter.state == STATE_OFF) {
        converter.texture = converter.off_texture;
        converter.w = converter.off_w;
        converter.h = converter.off_h;
        if (converter_can_overheat(converter))
            converter.heat_time_current = converter.heat_time_max;
    } else if (converter.state == STATE_OUT) {
        converter.texture = converter.output_texture;
        converter.w = converter.output_w;
        converter.h = converter.output_h;
    }
}

void converter_draw() {
    converter.x = 10;
    converter.y = gui.popup_y + 10;

    SDL_Rect rect = {
        converter.x, converter.y,
        converter.w, converter.h
    };

    if (converter.heat_state == HEAT_TOO_HOT) {
        SDL_SetTextureColorMod(converter.texture, 255, 0, 0);
    } else if (converter.heat_state == HEAT_OVERHEATING) {
        SDL_SetTextureColorMod(converter.texture, 255, 200, 200);
    } else if (converter.heat_state == HEAT_HOT) {
        SDL_SetTextureColorMod(converter.texture, 255, 255, 255);
    } else if (converter.heat_state == HEAT_COLD) {
        SDL_SetTextureColorMod(converter.texture, 0, 0, 255);
    }
    SDL_RenderCopy(renderer, converter.texture, NULL, &rect);

    if (current_tool == TOOL_PLACER) {
        if (converter.placer_top)
            placer_draw(converter.placer_top);
        if (converter.placer_bottom)
            placer_draw(converter.placer_bottom);

        if (get_current_placer() && get_current_placer() != converter.placer_top && get_current_placer() != converter.placer_bottom)
            placer_draw(get_current_placer());
    }
}

// Converts cell type "input" to another babsed on the converter's current
// heat level.
int converter_convert(int input) {
    if (converter.heat_state == HEAT_HOT || converter.heat_state == HEAT_OVERHEATING) { // Heat
        switch (input) {
        case CELL_SAND:   return CELL_GLASS;
        case CELL_GLASS:  return CELL_MARBLE;
        case CELL_MARBLE: return CELL_QUARTZ;
        case CELL_QUARTZ: return CELL_SAND;

        case CELL_DIRT:   return CELL_COAL;
        case CELL_COAL:   return CELL_DIAMOND;

        // TODO: Remove this from the converter and make a wood converter.
        case CELL_WOOD_LOG:   return CELL_WOOD_PLANK;
        case CELL_WOOD_PLANK: return CELL_COAL;

        case CELL_ICE:   return CELL_WATER;
        case CELL_WATER: return CELL_STEAM;
        }
    } else if (converter.heat_state == HEAT_COLD) { // Cold
        switch (input) {
        case CELL_QUARTZ: return CELL_MARBLE;
        case CELL_MARBLE: return CELL_GLASS;
        case CELL_GLASS:  return CELL_SAND;
        case CELL_SAND:   return CELL_DIRT;

        case CELL_COAL:   return CELL_DIRT;

        case CELL_STEAM:  return CELL_WATER;
        case CELL_WATER:  return CELL_ICE;
        }
    }
    return input;
}
