#define PLACER_COUNT 3

#define MAX_PLACE_SOLID_TIME 45 // Frames until cut off for solids.

enum {
    PLACER_SUCK_MODE,
    PLACER_PLACE_RECT_MODE,
    PLACER_PLACE_CIRCLE_MODE
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

    int contains_type, contains_amount;

    int placing_solid_time;
};
