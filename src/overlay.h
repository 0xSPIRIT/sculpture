#define MAX_GRID_CHANGES 64

#define OVERLAY_ALPHA_1 0.8f
#define OVERLAY_ALPHA_2 0.3f

enum Overlay_Tool {
    OVERLAY_TOOL_BRUSH,
    OVERLAY_TOOL_LINE,
    OVERLAY_TOOL_SPLINE,
    OVERLAY_TOOL_RECTANGLE,
    OVERLAY_TOOL_BUCKET,
    OVERLAY_TOOL_ERASER_BRUSH,
    OVERLAY_TOOL_ERASER_RECTANGLE,
};

typedef struct Overlay_Changes {
    int *grids[MAX_GRID_CHANGES];
    int count, index;

    f32 temp;
    bool was_grid_none; // Was the games grid completely zeroed out at some point?
    bool is_grid_none_previous; // For the previous frame

    f32 alpha;
} Overlay_Changes;

typedef struct Overlay {
    int *grid;
    int *temp_grid;
    enum Overlay_Tool tool;

    bool temp;

    int current_material;

    Overlay_Changes changes;

    f32 alpha_coefficient;

    SDL_Rect r;
    f32 size;

    bool eraser_mode;
    bool show; // Why isn't this called active?

    int temp_x, temp_y;
} Overlay;
