#define OVERLAY_INTERFACE_BUTTONS 5
#define OVERLAY_INTERFACE_ERASER_MODE 4 // Index into interface->buttons

struct Overlay_Interface {
    struct Button *buttons[OVERLAY_INTERFACE_BUTTONS];
};

// Function prototypes so that gui.c can find it.
// (overlay_interface.c is included before gui.c
//  so we must do this.)
void overlay_interface_init();
void overlay_interface_draw();
void overlay_interface_tick();
