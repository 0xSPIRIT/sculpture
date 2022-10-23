#define OVERLAY_INTERFACE_BUTTONS 7
#define OVERLAY_INTERFACE_ERASER_MODE 5 // Index into interface->buttons
#define OVERLAY_INTERFACE_CLEAR_ALL 6 // Index into interface->buttons

struct Overlay_Interface {
    struct Button *buttons[OVERLAY_INTERFACE_BUTTONS];
};

// Function prototypes so that gui.c can find it.
// (overlay_interface.c is included before gui.c
//  so we must do this.)

void click_overlay_interface(void *ptr);
void overlay_interface_init();
void overlay_interface_draw();
void overlay_interface_tick();
