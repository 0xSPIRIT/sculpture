static void click_overlay_interface(void *ptr) {
    int button_index = *(int*)ptr;

    Overlay *overlay = &gs->overlay;
    Overlay_Interface *interf = &gs->gui.overlay_interface;

    if (button_index == OVERLAY_INTERFACE_CLEAR_ALL) {
        for (int i = 0; i < gs->gw*gs->gh; i++) {
            overlay->grid[i] = 0;
        }
    }

    switch (button_index) {
        case OVERLAY_TOOL_BRUSH: {
            overlay->tool = OVERLAY_TOOL_BRUSH;
            break;
        }
        case OVERLAY_TOOL_LINE: {
            overlay->tool = OVERLAY_TOOL_LINE;
            break;
        }
        case OVERLAY_TOOL_SPLINE: {
            overlay->tool = OVERLAY_TOOL_SPLINE;
            break;
        }
        case OVERLAY_TOOL_RECTANGLE: {
            overlay->tool = OVERLAY_TOOL_RECTANGLE;
            break;
        }
        case OVERLAY_TOOL_BUCKET: {
            overlay->tool = OVERLAY_TOOL_BUCKET;
            break;
        }
        case OVERLAY_INTERFACE_ERASER_MODE: {
            overlay->eraser_mode = !overlay->eraser_mode;
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

    if (button_index == OVERLAY_INTERFACE_ERASER_MODE) {
        interf->buttons[OVERLAY_INTERFACE_ERASER_MODE]->active = overlay->eraser_mode;
        return;
    }

    interf->buttons[button_index]->active = true;

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        if (i == OVERLAY_INTERFACE_ERASER_MODE) continue;

        if (i != button_index) {
            interf->buttons[i]->active = false;
        }
    }
}

static void overlay_interface_init(void) {
    Overlay_Interface *interf = &gs->gui.overlay_interface;

    const char overlay_interface_names[OVERLAY_INTERFACE_BUTTONS][64] = {
        "Brush Tool",
        "Line Tool",
        "Spline Tool (Under Construction!)",
        "Rectangle Tool",
        "Bucket Tool",
        "Eraser Mode",
        "Clear All",
    };

    int cum = 0;
    int ypad = 10;

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        Button *b = 0;

        b = button_allocate(BUTTON_TYPE_OVERLAY_INTERFACE,
                            &GetTexture(TEXTURE_TOOL_BUTTONS + TOOL_GRABBER),
                            overlay_interface_names[i],
                            click_overlay_interface);

        b->x = 20;
        b->y = 110 + cum;
        b->index = i;

        cum += b->w + ypad;

        interf->buttons[i] = b;
    }

    interf->buttons[0]->active = true;
}

static void overlay_interface_tick(void) {
    Overlay_Interface *interf = &gs->gui.overlay_interface;

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        button_tick(interf->buttons[i], (void*)&i);
    }
}

static void overlay_interface_draw(int target) {
    Overlay_Interface *interf = &gs->gui.overlay_interface;

    for (int i = 0; i < OVERLAY_INTERFACE_BUTTONS; i++) {
        button_draw(target, interf->buttons[i]);
    }
}
