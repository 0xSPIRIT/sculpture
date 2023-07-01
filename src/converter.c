static Converter *converter_init(int type, bool allocated) {
    Converter *converter = NULL;

    if (!allocated) {
        converter = PushSize(gs->persistent_memory, sizeof(Converter));
    } else {
        switch (type) {
            case CONVERTER_FUEL:     converter = gs->fuel_converter;     break;
            case CONVERTER_MATERIAL: converter = gs->material_converter; break;
        }
    }

    converter->type = type;
    converter->w = (f32) (gs->window_width/2);
    converter->h = GUI_POPUP_H;

    converter->timer_max = 1;
    converter->timer_current = 0;

    if (type == CONVERTER_FUEL && gs->level_current+1 <= 5) {
        converter->state = CONVERTER_INACTIVE;
    } else {
        converter->state = CONVERTER_OFF;
    }

    switch (type) {
        case CONVERTER_MATERIAL: {
            converter->slot_count = 4;

            if (!allocated) {
                converter->slots = PushArray(gs->persistent_memory, converter->slot_count, sizeof(Slot));
            } else {
                memset(&converter->slots[SLOT_INPUT1], 0, sizeof(Slot));
                memset(&converter->slots[SLOT_INPUT2], 0, sizeof(Slot));
                memset(&converter->slots[SLOT_FUEL],   0, sizeof(Slot));
                memset(&converter->slots[SLOT_OUTPUT], 0, sizeof(Slot));
            }

            // NOTE: Converter slot positions are in global space,
            //       but the converter itself isn't. Yes, it's
            //       unnecessarily confusing.

            converter_setup_position(converter);

            strcpy(converter->name, "Material Converter");

            auto_set_material_converter_slots(converter);
            break;
        }

        case CONVERTER_FUEL: {
            converter->slot_count = 3;

            if (!allocated) {
                converter->slots = PushArray(gs->persistent_memory, converter->slot_count, sizeof(Slot));
            } else {
                memset(&converter->slots[SLOT_INPUT1], 0, sizeof(Slot));
                memset(&converter->slots[SLOT_INPUT2], 0, sizeof(Slot));
                memset(&converter->slots[SLOT_OUTPUT], 0, sizeof(Slot));
            }

            converter_setup_position(converter);

            strcpy(converter->name, "Fuel Converter");
            break;
        }
    }

    return converter;
}

static void all_converters_init(void) {
    bool allocated = gs->material_converter != NULL || gs->fuel_converter != NULL;
    gs->material_converter = converter_init(CONVERTER_MATERIAL, allocated);
    gs->fuel_converter = converter_init(CONVERTER_FUEL, allocated);

}

static void converter_set_state(Converter *converter, enum Converter_State state) {
    converter->state = state;
    if (state == CONVERTER_OFF) {
        converter->state = CONVERTER_OFF;
        RenderTextureColorMod(converter->arrow.texture, 255, 255, 255);
    }
}

static void converter_draw(int target, Converter *converter) {
    if (converter->state == CONVERTER_INACTIVE)
        return;

    if (converter->y >= gs->window_height-GUI_H)
        return;

    converter->w = (f32) (gs->window_width/2);
    converter->h = GUI_POPUP_H;

    RenderColor(Red(CONVERTER_LINE_COLOR),
                Green(CONVERTER_LINE_COLOR),
                Blue(CONVERTER_LINE_COLOR),
                255);
    RenderLine(target,
               converter->x+converter->w,
               converter->y+GUI_H, converter->x+converter->w,
               converter->y+GUI_H+converter->h);

    for (int i = 0; i < converter->slot_count; i++) {
        slot_draw(target, &converter->slots[i], converter->x, converter->y);
    }

    SDL_Rect arrow_dst = {
        (int) (converter->x + converter->arrow.x - Scale(converter->arrow.w) / 2.0),
        (int) (converter->y + converter->arrow.y + Scale(converter->arrow.h) / 2.0),
        Scale(converter->arrow.w),
        Scale(converter->arrow.h)
    };

    // Flashing the arrow itself.
    if (converter->state == CONVERTER_ON) {
        const int period = 500; // Milliseconds.
        Uint8 a = (SDL_GetTicks()/period) % 2 == 0;
        a = a ? 255 : 190;

        RenderTextureColorMod(converter->arrow.texture, a, a, a);
    }
    // Since we use the same texture for both converters,
    // we need to reset it every time as well.
    else {
        RenderTextureColorMod(converter->arrow.texture, 255, 255, 255);
    }

    RenderTexture(target,
                  converter->arrow.texture,
                  NULL,
                  &arrow_dst);

    // TODO:
    // Draw converter->name using gs->fonts.font_courier,
    // with CONVERTER_NAME_COLOR,  at
    //  (int) (converter->x + margin),
    //  (int) (converter->y + margin + GUI_H),

    char identifier[64] = {0};
    sprintf(identifier, "%p", converter);
    int margin = 8;
    RenderDrawTextQuick(RENDER_TARGET_MASTER,
                        identifier,
                        gs->fonts.font_courier,
                        converter->name,
                        ColorFromInt(CONVERTER_NAME_COLOR),
                        255,
                        (int) (converter->x + margin),
                        (int) (converter->y + margin + GUI_H),
                        NULL,
                        NULL,
                        false);

    button_draw(target, converter->go_button);
}

static void all_converters_draw(int target) {
    converter_draw(target, gs->material_converter);
    converter_draw(target, gs->fuel_converter);
}

static bool converter_is_layout_valid(Converter *converter) {
    bool is_empty = true;
    for (int i = 0; i < converter->slot_count; i++) {
        if (converter->slots[i].item.type) {
            is_empty = false;
            break;
        }
    }

    if (is_empty) return false;

    return true;
}

static void converter_begin_converting(void *converter_ptr) {
    Converter *converter = (Converter *) converter_ptr;

    if (!converter_is_layout_valid(converter))
        return;

    if (converter->state == CONVERTER_OFF) {
        save_state_to_next();
    }

    converter_set_state(converter, converter->state == CONVERTER_ON ? CONVERTER_OFF : CONVERTER_ON);
}

static void converter_gui_setup_rectangle(bool really_update) {
    if (really_update) {
    } else if (!gs->resized) return;

    Conversions *c = &gs->converter;

    c->r = (SDL_Rect){
        gs->window_width - Scale(64+400),
        Scale(64),
        Scale(360), 0
    };

    c->r.h = gs->fonts.font_small->char_height*c->line_count - Scale(90);
}

static void converter_gui_init(void) {
    Conversions *c = &gs->converter;

    FILE *f = fopen(RES_DIR "layout_converter.txt", "r");

    char buf[128] = {0};

    while (fgets(buf, 128, f)) {
        const size_t len = strlen(buf);
        for (size_t i = 0; i < len; i++) {
            if (buf[i] == '\n' || buf[i] == '\r') buf[i] = 0;
        }
        strcpy(c->lines[c->line_count++], buf);
    }

    fclose(f);

    converter_gui_setup_rectangle(true);
}

static void converter_gui_tick(void) {
    Conversions *c = &gs->converter;

    if (!gs->text_field.active && gs->input.keys_pressed[SDL_SCANCODE_I]) {
        c->active = !c->active;
    }
}

static void converter_gui_draw(void) {
    Conversions *c = &gs->converter;

    const SDL_Color bg = ColorFromIntRGBA(CONVERSION_PANEL_COLOR);
    const int target = RENDER_TARGET_CONVERSION_PANEL;

    c->r.x = 0;
    c->r.y = 0;
    c->r.w = gs->window_width;
    c->r.h = gs->window_height-GUI_H;

    const bool update = false;

    if (!c->calculated_render_target || update || gs->resized) {
        c->calculated_render_target = true;
        int cum = 0;

        RenderColor(0, 0, 0, 0);
        RenderClear(target);

        RenderColor(bg.r, bg.g, bg.b, bg.a);
        RenderFillRect(target, c->r);

        RenderColor(128, 128, 128, 255);
        RenderDrawRect(target, c->r);

        int count = 0;

        for (int i = 0; i < c->line_count; i++) {
            count++;

            Render_Text_Data text_data = {0};
            text_data.font = gs->fonts.font_converter_gui;
            strcpy(text_data.str, c->lines[i]);
            text_data.foreground = (SDL_Color){255, 255, 255, 255};
            text_data.alignment = ALIGNMENT_TOP_LEFT;
            text_data.x = c->r.x + Scale(140);
            text_data.y = c->r.y + Scale(16) + cum;
            text_data.alpha = 255;

            RenderDrawText(target, &text_data);
            cum += text_data.texture.height;
        }

        {
            RenderColor(0, 0, 0, 0);
            SDL_Rect a = {
                0, c->r.y + c->r.h,
                gs->window_width, gs->window_height - (c->r.y + c->r.h+GUI_H)
            };
            RenderFillRect(target, a);

            SDL_Rect b = {
                c->r.x+c->r.w, 0,
                gs->window_width - (c->r.x + c->r.w), gs->window_height-GUI_H
            };
            RenderFillRect(target, b);
        }
    }

    if (!c->active) return;

    SDL_Rect src = {
        0, 0,
        gs->window_width, gs->window_height-GUI_H
    };
    SDL_Rect dst = {
        0, GUI_H,
        gs->window_width, gs->window_height-GUI_H
    };

    RenderTexture(RENDER_TARGET_MASTER,
                  &RenderTarget(target)->texture,
                  &src,
                  &dst);
}

static void auto_set_material_converter_slots(Converter *converter) {
    int level = gs->level_current;

    switch (level+1) {
        case 4: {
            converter->slots[SLOT_FUEL].item = (Item)
            {
                .type = CELL_UNREFINED_COAL,
                .amount = 179
            };
            break;
        }
        case 10: {
            converter->slots[SLOT_FUEL].item = (Item)
            {
                .type = CELL_UNREFINED_COAL,
                .amount = 2000
            };
        }
        default: {
            break;
        }
    }
}

static void converter_setup_position(Converter *converter) {
    converter->slots[SLOT_INPUT1].x = converter->w/3.f;
    converter->slots[SLOT_INPUT1].y = GUI_H + converter->h/4.f;
    strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");

    converter->slots[SLOT_INPUT2].x = 2.f*converter->w/3.f;
    converter->slots[SLOT_INPUT2].y = GUI_H + converter->h/4.f;
    strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");

    if (converter->type == CONVERTER_MATERIAL) {
        converter->slots[SLOT_FUEL].x = 3.f*converter->w/4.f;
        converter->slots[SLOT_FUEL].y = GUI_H + converter->h/2.f;
        strcpy(converter->slots[SLOT_FUEL].name, "Fuel");
    }

    converter->slots[SLOT_OUTPUT].x = converter->w/2.f;
    converter->slots[SLOT_OUTPUT].y = GUI_H + 4.f*converter->h/5.f;
    strcpy(converter->slots[SLOT_OUTPUT].name, "Output");

    // Indices line up with Slot_Type enum
    // even though slot_count is variable.
    for (int i = 0; i < converter->slot_count; i++) {
        converter->slots[i].type = i;
        converter->slots[i].w = ITEM_SIZE;
        converter->slots[i].h = ITEM_SIZE;
        converter->slots[i].converter = converter;
        converter->slots[i].inventory_index = -1;
    }

    converter->arrow.texture = &GetTexture(TEXTURE_CONVERTER_ARROW);

    converter->arrow.w = converter->arrow.texture->width;
    converter->arrow.h = converter->arrow.texture->height;

    converter->arrow.x = (int) (converter->w/2);
    converter->arrow.y = (int) (converter->h/2 + 24);
    converter->speed = 8;

    // Both X and Y-coordinates are updated in converter_tick.
    if (converter->go_button == NULL) {
        converter->go_button = button_allocate(BUTTON_CONVERTER,
                                               &GetTexture(TEXTURE_CONVERT_BUTTON),
                                               "Convert",
                                               converter_begin_converting);
    }
    converter->go_button->w = Scale(48);
    converter->go_button->h = Scale(48);
}

static int get_number_unique_inputs(Item *input1, Item *input2) {
    int number_inputs = (input1->type != 0) + (input2->type != 0);
    int number_unique_inputs = 0;

    if (number_inputs == 1) {
        number_unique_inputs = 1;
    } else if (number_inputs == 2) {
        if (input1->type != input2->type) {
            number_unique_inputs = 2;
        } else {
            number_unique_inputs = 1;
        }
    }

    return number_unique_inputs;
}

static Converter_Checker converter_checker(Item *input1, Item *input2) {
    Assert(input1);
    Assert(input2);

    return (Converter_Checker) {
        input1, input2, 0
    };
}

static bool is_either_input_type(Converter_Checker *checker, int type, bool restart) {
    Assert(checker->input1);
    Assert(checker->input2);

    if (restart) {
        checker->current = 0;
    }

    if (checker->current == 0) {
        if (checker->input1->type == type) {
            checker->current = 2;
        } else if (checker->input2->type == type) {
            checker->current = 1;
        } else {
            return false;
        }
    } else {
        if ((checker->current == 2 && checker->input2->type != type) ||
            (checker->current == 1 && checker->input1->type != type)) {
            return false;
        }
    }

    return true;
}

static bool is_either_input_tier(Converter_Checker *checker, int tier, bool is_fuel, bool restart) {
    Assert(checker->input1);
    Assert(checker->input2);

    if (restart) {
        checker->current = 0;
    }

    if (checker->current == 0) {
        if (get_cell_tier(checker->input1->type) == tier) {
            checker->current = 2;
        } else if (get_cell_tier(checker->input2->type) == tier) {
            checker->current = 1;
        }
    } else {
        if (is_fuel) {
            return ((checker->current == 2 && is_cell_fuel(checker->input2->type) && tier == get_cell_tier(checker->input2->type)) ||
                    (checker->current == 1 && is_cell_fuel(checker->input1->type) && tier == get_cell_tier(checker->input1->type)));
        } else {
            return ((checker->current == 2 && !is_cell_fuel(checker->input2->type) && tier == get_cell_tier(checker->input2->type)) ||
                    (checker->current == 1 && !is_cell_fuel(checker->input1->type) && tier == get_cell_tier(checker->input1->type)));
        }
    }

    return true;
}

static bool is_either_input_stone(Converter_Checker *checker, bool restart) {
    Assert(checker->input1);
    Assert(checker->input2);

    if (restart) {
        checker->current = 0;
    }

    if (checker->current == 0) {
        if (is_cell_stone(checker->input1->type)) {
            checker->current = 2;
        } else if (is_cell_stone(checker->input2->type)) {
            checker->current = 1;
        }
    } else {
        if ((checker->current == 2 && !is_cell_stone(checker->input2->type)) ||
            (checker->current == 1 && !is_cell_stone(checker->input1->type))) {
            return false;
        }
    }

    return true;
}

static int fuel_converter_convert(Item *input1, Item *input2) {
    int result_type = 0;
    int number_inputs = (input1->type != 0) + (input2->type != 0);
    int number_unique_inputs = 0;

    Item *input = NULL;

    // We simply don't allow this.
    if (input1->type == input2->type) {
        return 0;
    }

    if (number_inputs == 1) {
        input = input1->type ? input1 : input2;
    } else if (number_inputs == 2) {
        input = input1;
    }

    number_unique_inputs = get_number_unique_inputs(input1, input2);

    if (number_unique_inputs == 1) {
        if (input->type == CELL_DIRT ||
            input->type == CELL_SAND ||
            !is_cell_fuel(input->type) && get_cell_tier(input->type) == 1)
        {
            result_type = CELL_UNREFINED_COAL;
        }
    }
    else if (number_unique_inputs == 2) {
        Converter_Checker checker = converter_checker(input1, input2);

        if (is_either_input_type(&checker, CELL_UNREFINED_COAL, false) &&
            is_either_input_tier(&checker, 2, false, false))
        {
            result_type = CELL_REFINED_COAL;
        }

        else if (is_either_input_type(&checker, CELL_UNREFINED_COAL, true) &&
                 is_either_input_type(&checker, CELL_LAVA, false))
        {
            result_type = CELL_REFINED_COAL;
        }

        else if (is_either_input_type(&checker, CELL_REFINED_COAL, true) &&
                 is_either_input_stone(&checker, false))
        {
            result_type = CELL_LAVA;
        }
    }

    return result_type;
}

static int material_converter_convert(Item *input1, Item *input2, Item *fuel) {
    Assert(input1);
    Assert(input2);

    int result = 0;
    int number_inputs = (input1->type != 0) + (input2->type != 0);

    // We simply don't allow this.
    if (input1->type == input2->type) {
        return 0;
    }

    Item *input = NULL;

    if (number_inputs == 1) {
        input = input1->type ? input1 : input2;
        if (input->type == 0 || input->amount == 0)
            return 0;
    } else if (number_inputs == 2) {
        input = input1;
    }

    switch (fuel->type) {
        case CELL_NONE: {
            if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_WOOD_LOG: result = CELL_WOOD_PLANK; break;
                    case CELL_STEAM:    result = CELL_WATER;      break;
                    case CELL_WATER:    result = CELL_ICE;        break;
                }
            }
            break;
        }
        case CELL_UNREFINED_COAL: {
            if (number_inputs == 2) {
                Converter_Checker checker = converter_checker(input1, input2);

                if (is_either_input_type(&checker, CELL_COBBLESTONE, true) &&
                    is_either_input_type(&checker, CELL_SAND, false))
                {
                    result = CELL_SANDSTONE;
                }
            } else if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_SAND:        result = CELL_GLASS;       break;
                    case CELL_DIRT:        result = CELL_COBBLESTONE; break;
                    case CELL_COBBLESTONE: result = CELL_MARBLE;      break;
                    case CELL_ICE:         result = CELL_WATER;       break;
                    case CELL_WATER:       result = CELL_STEAM;       break;
                }
            }
            break;
        }
        case CELL_REFINED_COAL: {
            if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_DIRT:        result = CELL_COBBLESTONE; break;
                    case CELL_ICE:         result = CELL_STEAM;       break;
                    case CELL_COBBLESTONE: result = CELL_MARBLE;      break;
                }
            } else if (number_inputs == 2) {
                Converter_Checker checker = converter_checker(input1, input2);

                if (is_either_input_type(&checker, CELL_SANDSTONE, true) &&
                    is_either_input_type(&checker, CELL_MARBLE, false))
                {
                    result = CELL_QUARTZ;
                }
                else if (is_either_input_type(&checker, CELL_WATER, true) &&
                         is_either_input_type(&checker, CELL_COBBLESTONE, false))
                {
                    result = CELL_CEMENT;
                }
            }

            break;
        }
        case CELL_LAVA: {
            if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_REFINED_COAL: case CELL_UNREFINED_COAL: {
                        result = CELL_BASALT;
                        break;
                    }
                    case CELL_COBBLESTONE: {
                        result = CELL_MARBLE;
                        break;
                    }
                }
            } else if (number_inputs == 2) {
                Converter_Checker checker = converter_checker(input1, input2);

                if (is_either_input_type(&checker, CELL_QUARTZ, true) &&
                    is_either_input_type(&checker, CELL_MARBLE, false))
                {
                    result = CELL_GRANITE;
                }
                else if (is_either_input_type(&checker, CELL_BASALT, true) &&
                         is_either_input_type(&checker, CELL_GRANITE, false))
                {
                    result = CELL_DIAMOND;
                }
            }
            break;
        }
    }

    return result;
}

// Returns if the conversion went succesfully.
static bool converter_convert(Converter *converter) {
    bool did_convert = false;

    int temp_output_type = 0;

    Item *input1 = &converter->slots[SLOT_INPUT1].item;
    Item *input2 = &converter->slots[SLOT_INPUT2].item;
    Item *output = &converter->slots[SLOT_OUTPUT].item;
    Item *fuel = NULL;

    if (converter->type == CONVERTER_MATERIAL) {
        fuel = &converter->slots[SLOT_FUEL].item;
    }

    int number_inputs = (input1->type != 0) + (input2->type != 0);

    if (!number_inputs) return false;
    if (input1->type == input2->type) return false;

    /* if (fuel && (!fuel->type || fuel->amount == 0)) return false; */

    if (converter->type == CONVERTER_FUEL) {
        temp_output_type = fuel_converter_convert(input1, input2);
    } else if (converter->type == CONVERTER_MATERIAL) {
        temp_output_type = material_converter_convert(input1, input2, fuel);
    }

    if (!temp_output_type) return false;

    bool final_conversion = false;

    // Actually remove amounts from the inputs and increase
    // or set the output.
    if (temp_output_type == output->type || output->type == 0) {
        output->type = temp_output_type;

        int amount = converter->speed;

        // Check if any input, when reduced by the amount,
        // gives negative amount. If so, lock the amount
        // we're reducing to by the minimum amount.
        if (input1->type && input1->amount-amount < 0) {
            final_conversion = true;
        }
        if (input2->type && input2->amount-amount < 0) {
            final_conversion = true;
        }

        // Same for fuel
        if (fuel && fuel->type && fuel->amount-1 < 0) {
            final_conversion = true;
        }

        if (final_conversion) {
            // Find the lowest value from the inputs and fuel.
            amount = 999999999;
            for (int i = SLOT_INPUT1; i <= SLOT_FUEL; i++) {
                if (i == SLOT_OUTPUT) continue;
                if (fuel && i == SLOT_FUEL) continue;

                if (converter->slots[i].item.type && converter->slots[i].item.amount < amount) {
                    amount = converter->slots[i].item.amount;
                }
            }
        }

        if (input1->type) {
            input1->amount -= amount;
        }
        if (input2->type) {
            input2->amount -= amount;
        }

        if (fuel && fuel->type) {
            fuel->amount--;
        }
        output->amount += amount;

        did_convert = true;
    }

    return !final_conversion || !did_convert;
}

static void converter_tick(Converter *converter) {
    converter->arrow.y = (int) (converter->h/2 + 18);

    converter_setup_position(converter);

    switch (converter->type) {
        case CONVERTER_MATERIAL: {
            converter->x = 0;
            converter->y = gs->gui.popup_y;
            break;
        }
        case CONVERTER_FUEL: {
            converter->x = (f32) (gs->S*gs->gw/2);
            converter->y = (f32) (gs->gui.popup_y);
            break;
        }
    }

    converter->go_button->x = (int) (converter->x + converter->arrow.x - Scale(128));
    converter->go_button->y = (int) (gs->gui.popup_y + Scale(250));

    if (converter->state == CONVERTER_INACTIVE)
        return;

    for (int i = 0; i < converter->slot_count; i++) {
        slot_tick(&converter->slots[i]);
    }

    button_tick(converter->go_button, converter);

    if (converter->state == CONVERTER_ON) {
        if (converter->timer_current < converter->timer_max) {
            converter->timer_current += 1;
        } else {
            converter->timer_current = 0;
            switch (converter->type) {
                case CONVERTER_MATERIAL: {
                    bool did_convert = converter_convert(gs->material_converter);
                    if (!did_convert) {
                        converter_set_state(gs->material_converter, CONVERTER_OFF);
                    }
                    break;
                }
                case CONVERTER_FUEL: {
                    bool did_convert = converter_convert(gs->fuel_converter);
                    if (!did_convert) {
                        converter_set_state(gs->fuel_converter, CONVERTER_OFF);
                    }
                    break;
                }
            }
        }
    }

    if (!gs->gui.popup) return;
}

static void all_converters_tick(void) {
    // Grey out the buttons for each converter
    // if no converter are possible right now.

    if (0 == material_converter_convert(&gs->material_converter->slots[SLOT_INPUT1].item,
                                        &gs->material_converter->slots[SLOT_INPUT2].item,
                                        &gs->material_converter->slots[SLOT_FUEL].item)) {
        gs->material_converter->go_button->disabled = true;
    } else {
        gs->material_converter->go_button->disabled = false;
    }

    if (0 == fuel_converter_convert(&gs->fuel_converter->slots[SLOT_INPUT1].item,
                                    &gs->fuel_converter->slots[SLOT_INPUT2].item))
    {
        gs->fuel_converter->go_button->disabled = true;
    } else {
        gs->fuel_converter->go_button->disabled = false;
    }

    converter_tick(gs->material_converter);
    converter_tick(gs->fuel_converter);

    for (int i = 0; i < 2; i++) {
        Converter *c = i == 0 ? gs->material_converter : gs->fuel_converter;

        for (int j = 0; j < c->slot_count; j++) {
            if (!c->slots[j].item.type) continue;

            if (is_mouse_in_slot(&c->slots[j])) {
                tooltip_set_position_to_cursor(&gs->gui.tooltip, TOOLTIP_TYPE_ITEM);

                char type[64] = {0};
                char type_name[64] = {0};
                char amount[64] = {0};

                get_name_from_type(c->slots[j].item.type, type_name);
                sprintf(type, "Type: %s", type_name);
                sprintf(amount, "Amount: %d", c->slots[j].item.amount);

                strcpy(gs->gui.tooltip.str[0], type);
                strcpy(gs->gui.tooltip.str[1], amount);

                gs->gui.tooltip.set_this_frame = true;
            } else if (!gs->gui.tooltip.set_this_frame && gs->gui.tooltip.type == TOOLTIP_TYPE_ITEM) {
                tooltip_reset(&gs->gui.tooltip);
            }
        }
    }
}

static void setup_item_indices() {
    // Set up the item indices.

    int mat_slot_count = gs->material_converter->slot_count;
    int fuel_slot_count = gs->fuel_converter->slot_count;

    gs->item_holding.index = 0;
    for (int i = 0; i < mat_slot_count; i++) {
        gs->material_converter->slots[i].item.index = i+1;
    }
    for (int i = 0; i < fuel_slot_count; i++) {
        gs->material_converter->slots[i].item.index = mat_slot_count+i+1;
    }
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        gs->inventory.slots[i].item.index = mat_slot_count+fuel_slot_count+i+1;
    }
}