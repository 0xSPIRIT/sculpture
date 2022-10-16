enum Overlay_Tool {
    OVERLAY_TOOL_NONE,
    OVERLAY_TOOL_BRUSH,
    OVERLAY_TOOL_LINE,
    OVERLAY_TOOL_ERASER,
};

struct Overlay {
    int *grid;
    enum Overlay_Tool tool;
};
