#define PLACER_COUNT 5
#define MAX_PLACE_SOLID_TIME 45 // Frames until cut off for solids.

enum {
    PLACER_PLACE_RECT_MODE,
    PLACER_PLACE_CIRCLE_MODE,
    PLACER_SUCK_MODE,
};

struct Placer {
    int index;

    int state;

    SDL_Rect rect;
    
    int x, y, px, py;
    int w, h;
    SDL_Texture *texture;

    int radius;

    int object_index; // What object index to set the cells to.
    int did_click;    // Did we actually place down any material?
    int did_set_new;

    struct Item *contains;

    int placing_solid_time;
};

struct Placer *get_current_placer();