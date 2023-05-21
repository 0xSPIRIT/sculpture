#define MAX_TOOLTIP_LEN 128
#define MAX_TOOLTIP_LINE_LEN 128

typedef struct Tooltip {
    enum Tooltip_Type type;
    
    bool set_this_frame;
    
    f32 x, y;
    char str[MAX_TOOLTIP_LEN][MAX_TOOLTIP_LINE_LEN];
    int w, h;
    
    Preview *preview;
} Tooltip;

void tooltip_reset(Tooltip *tooltip);
void tooltip_set_position_to_cursor(Tooltip *tooltip, int type);
void tooltip_set_position(Tooltip *tooltip, int x, int y, int type);
void tooltip_draw_box(Tooltip *tooltip, int w, int h);
void tooltip_get_string(int type, int amt, char *out_str);
void tooltip_draw(Tooltip *tooltip);

