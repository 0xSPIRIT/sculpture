#define MAX_GRID_CHANGES 64

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
    
    int temp;
    bool was_grid_none; // Was the games grid completely zeroed out at some point?
    
    f32 alpha;
} Overlay_Changes;

typedef struct Overlay {
    int *grid;
    int *temp_grid;
    enum Overlay_Tool tool;
    
    int current_material;
    
    Overlay_Changes changes;
    
    SDL_Rect r;
    f32 size;
    
    bool eraser_mode;
    bool show;
    
    int temp_x, temp_y;
} Overlay;