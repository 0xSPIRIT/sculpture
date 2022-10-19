#define OVERLAY_INTERFACE_ERASER_MODE 4 // Index into interface->buttons

void click_overlay_interface(void *ptr) {
    int button_index = *(int*)ptr;

    struct Overlay *overlay = &gs->overlay;
    struct Overlay_Interface *interface = &gs->gui.overlay_interface;

    switch (button_index) {
    case OVERLAY_TOOL_BRUSH: {
        overlay->tool = OVERLAY_TOOL_BRUSH;
        gui_message_stack_push("Overlay Tool: Brush");
        break;
    }
    case OVERLAY_TOOL_LINE: {
        overlay->tool = OVERLAY_TOOL_LINE;
        gui_message_stack_push("Overlay Tool: Line");
        break;
    }
    case OVERLAY_TOOL_RECTANGLE: {
        overlay->tool = OVERLAY_TOOL_RECTANGLE;
        gui_message_stack_push("Overlay Tool: Line");
        break;
    }
    case OVERLAY_TOOL_BUCKET: {
        overlay->tool = OVERLAY_TOOL_BUCKET;
        gui_message_stack_push("Overlay Tool: Bucket");
        break;
    }
    case OVERLAY_INTERFACE_ERASER_MODE: {
        overlay->eraser_mode = !overlay->eraser_mode;
        if (overlay->eraser_mode) {
            gui_message_stack_push("Eraser Mode: On");
        } else {
            gui_message_stack_push("Eraser Mode: Off");
        }
        break;
    }
    }

    if (overlay->tool == OVERLAY_TOOL_BRUSH || overlay->tool == OVERLAY_TOOL_ERASER_BRUSH) {
        if (overlay->eraser_mode) {
            overlay->tool = OVERLAY_TOOL_ERASER_BRUSH;
        } else {
            overlay->tool = OVERLAY_TOOL_BRUSH;
        }
    } else if (overlay->tool == OVERLAY_TOOL_RECTANGLE || overlay->tool == OVERLAY_TOOL_ERASER_RECTANGLE) {
        if (overlay->eraser_mode) {
            overlay->tool = OVERLAY_TOOL_ERASER_RECTANGLE;
        } else {
            overlay->tool = OVERLAY_TOOL_RECTANGLE;
        }
    }

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        if (i == OVERLAY_INTERFACE_ERASER_MODE) {
            interface->buttons[i]->activated = overlay->eraser_mode;
        } else if (i != button_index) {
            interface->buttons[i]->activated = false;
        }
    }
}

void overlay_interface_init() {
    struct Overlay_Interface *interface = &gs->gui.overlay_interface;

    const char overlay_interface_names[OVERLAY_INTERFACE_BUTTONS][64] = {
        "Brush Tool",
        "Line Tool",
        "Rectangle Tool",
        "Bucket Tool",
        "Eraser Mode",
    };

    int cum = 0;
    int ypad = 10;

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        struct Button *b = 0;

        SDL_Texture *texture = gs->textures.tool_buttons[0];

        b = button_allocate(
            BUTTON_TYPE_OVERLAY_INTERFACE,
            texture,
            overlay_interface_names[i],
            click_overlay_interface
        );

        b->x = 20;
        b->y = 110 + cum;
        b->index = i;

        cum += b->w + ypad;

        interface->buttons[i] = b;
    }
}

void overlay_interface_tick() {
    struct Overlay_Interface *interface = &gs->gui.overlay_interface;

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        button_tick(interface->buttons[i], (void*)&i);
    }
}

void overlay_interface_draw() {
    struct Overlay_Interface *interface = &gs->gui.overlay_interface;

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        button_draw(interface->buttons[i]);
    }
}
