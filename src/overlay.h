enum Overlay_Tool {
    OVERLAY_TOOL_BRUSH,
    OVERLAY_TOOL_LINE,
    OVERLAY_TOOL_RECTANGLE,
    OVERLAY_TOOL_ERASER_BRUSH,
    OVERLAY_TOOL_ERASER_RECTANGLE,
    OVERLAY_TOOL_BUCKET,
};

struct Overlay {
    int *grid;
    int *temp_grid;
    enum Overlay_Tool tool;

    SDL_Rect r;
    int size;

    int temp_x, temp_y;
};
