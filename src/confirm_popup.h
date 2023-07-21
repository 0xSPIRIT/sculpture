typedef struct Button Button;
typedef struct {
    bool active;
    char text[128];
    SDL_Rect r;
    Button *a, *b;
} Popup_Confirm;

// Used as the parameter to the popup's run callback.
typedef struct {
    int text_width, text_height; // OUT
} Popup_Confirm_Run_Data;

static void popup_confirm_activate(Popup_Confirm *popup);

static Popup_Confirm popup_confirm_init(const char *string,
                                        int confirm_type,
                                        int cancel_type,
                                        void (*on_confirm)(void*),
                                        void (*on_exit)(void*));

// You must call this in your own update callback.
static void popup_confirm_base_tick_and_draw(Popup_Confirm_Run_Data *data, int target, Popup_Confirm *popup);

// End of level popup hooks
static void end_of_level_popup_confirm_run(int target);
static void end_of_level_popup_confirm_confirm(void*);
static void end_of_level_popup_confirm_cancel(void*);

// Restart level hooks
void restart_popup_confirm_run(int target);
void restart_popup_confirm_confirm(void *unused);
void restart_popup_confirm_cancel(void *unused);
