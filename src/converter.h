#ifndef CONVERTER_H
#define CONVERTER_H

#include <SDL2/SDL.h>

#include "grid.h"

enum {
    STATE_OFF,
    STATE_ON,
    STATE_OUT
};

enum {
    HEAT_COLD,        // Used for cooling down material
    HEAT_HOT,         // Used for heating up
    HEAT_OVERHEATING, // Indicates overheating (slows output)
    HEAT_TOO_HOT      // Unable to convert.
};

struct Converter {
    int x, y;
    int w, h;

    SDL_Texture *texture;

    SDL_Texture *off_texture, *on_texture, *output_texture;
    int off_w, off_h, on_w, on_h, output_w, output_h;

    int state;

    struct Placer *placer_top, *placer_bottom; // The placers attached at top & bottom

    int heat_state;
    int heat_time_current, heat_time_max;
    int cooldown_time_current, cooldown_time_max;

    int contains_type;
    float contains_amount;
};

extern struct Converter converter;

void converter_init();
void converter_deinit();
void converter_tick();
void converter_draw();
int converter_convert(int input);

int set_current_converter();

#endif  /* CONVERTER_H */
