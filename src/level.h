#ifndef LEVEL_H_
#define LEVEL_H_

#define MAX_LEVELS 16
#define POPUP_TIME 45

enum {
    LEVEL_STATE_INTRO,
    LEVEL_STATE_PLAY,
    LEVEL_STATE_OUTRO
};

struct Source_Cell {
    int x, y;
    int type;
};

struct Level {
    int state;
    int index;
    char name[256];
    int effect_type;
    struct Cell *desired_grid; // What the inspiration is
    struct Cell *initial_grid; // Starting state of grid
    struct Source_Cell source_cell[256];
    int source_cell_count;
    int w, h;
    int popup_time_current, popup_time_max;
};

void levels_setup();
void goto_level_string_hook(const char *string);
void goto_level(int lvl);
void levels_deinit();
void level_tick();
void level_draw();
void level_draw_intro();
void level_get_cells_from_image(char *path, struct Cell **out, struct Source_Cell *source_cells, int *out_source_cell_count, int *out_w, int *out_h);
void level_output_to_png(const char *output_file);

#endif
