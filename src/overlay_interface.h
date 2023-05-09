#define OVERLAY_INTERFACE_BUTTONS 7
#define OVERLAY_INTERFACE_ERASER_MODE 5 // Index into interface->buttons
#define OVERLAY_INTERFACE_CLEAR_ALL 6 // Index into interface->buttons

typedef struct Button Button;

typedef struct Overlay_Interface {
    Button *buttons[OVERLAY_INTERFACE_BUTTONS];
} Overlay_Interface;

// Function prototypes so that gui.c can find it.
// (overlay_interface.c is included before gui.c
//  so we must do this.)

void click_overlay_interface(void *ptr);
void overlay_interface_init(void);
void overlay_interface_draw(void);
void overlay_interface_tick(void);
