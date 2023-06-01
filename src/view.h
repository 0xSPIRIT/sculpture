typedef struct {
    f64 x, y; // Offset of the entire screen
    f64 w, h;
} View;

static void view_update(void);