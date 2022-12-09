// A rectangle containing text, and an "Okay" button.
struct Tutorial_Rect {
    TTF_Font *font;
    
    bool active;
    
    int margin;
    
    char lines[64][64];
    int line_count;
    
    SDL_Rect rect;
    
    struct Button *ok_button;
};

void tutorial_rect_close(void*);
