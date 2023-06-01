typedef struct Text_Field {
    char description[256];
    char text[256];
    bool active;

    void (*on_return)(const char *);
} Text_Field;

static void set_text_field(const char *description, const char *initial_text, void (*on_return)(const char *));
