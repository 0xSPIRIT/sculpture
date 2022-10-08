struct Text_Field {
    char description[256];
    char text[256];
    bool active;

    void (*on_return)(const char *);
};
