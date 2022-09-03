#ifndef CONVERTER_H
#define CONVERTER_H

#define MAX_CONVERTERS 5

#include <SDL2/SDL.h>

#include "grid.h"

enum {
    STATE_OFF,
    STATE_ON,
    STATE_OUT
};

enum {
    CONVERTER_FURNACE,
    CONVERTER_BLAST_FURNACE,
    CONVERTER_WOOD_PROCESSOR,
    CONVERTER_STONECUTTER,
    CONVERTER_COOLER,
    CONVERTER_COUNT
};

enum {
    PLACER_INPUT,
    PLACER_OUTPUT,
    PLACER_FUEL
};

struct Converter {
    int type;

    int x, y;
    int w, h;

    SDL_Texture *texture;

    SDL_Texture *off_texture, *on_texture, *working_texture;
    int off_w, off_h, on_w, on_h, output_w, output_h;

    int state;

    struct Placer **placers;
    SDL_Point *rel_placer_attachment;
    int placer_count;
    
    int speed;

    int contains_type;
    float contains_amount;
    int (*convert_hook)(int, int);
};

extern struct Converter *converters[MAX_CONVERTERS];
extern struct Converter *furnace,
                        *blast_furnace,
                        *stonecutter,
                        *wood_processor,
                        *cooler;

struct Placer *get_current_placer();
void all_converters_init();
void all_converters_reset();
void converters_deinit();
struct Converter *converter_init();
void converter_deinit(struct Converter *converter);
void converter_tick(struct Converter *converter);
void converter_draw(struct Converter *converter);

int furnace_convert(int unused_state, int input);
int blast_furnace_convert(int unused_state, int input);
int cooler_convert(int unused_state, int input);

int set_current_converter();

#endif  /* CONVERTER_H */
