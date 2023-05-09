#define MAX_CONVERSION_LINES 100

typedef struct Conversions {
    bool active; // Is the panel active?
    
    char *string;
    char lines[MAX_CONVERSION_LINES][100];
    int line_count;
    
    bool calculated_render_target;
    
    SDL_Rect r; // The panel position and size.
} Conversions;