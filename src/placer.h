#define PLACER_COUNT 5
#define MAX_PLACE_SOLID_TIME 45 // Frames until cut off for solids.
#define PLACER_MINIMUM_AREA 5

enum {
    PLACER_PLACE_RECT_MODE,
    PLACER_PLACE_CIRCLE_MODE,
    PLACER_SUCK_MODE,
};

typedef struct Placer {
    int index;
    
    int state;
    bool escape_rect; // A flag for if we escape out of the setting of rectangle.
    
    bool did_place_this_frame;
    bool was_placing;
    int suck_audio_state;

    SDL_Rect rect;

    int place_width, place_height; // The size of the rectangle
    f64 place_aspect;

    int x, y, px, py;
    //int w, h;
    //Texture *texture;

    int radius;

    int object_index; // What object index to set the cells to.
    int did_click;    // Did we actually place down any material?
    int did_set_new;

    Item *contains;

    int placing_solid_time;
} Placer;

static Placer *get_current_placer(void);