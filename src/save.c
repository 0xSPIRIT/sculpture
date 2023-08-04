#define SAVE_PATH "save.file"

static void save_game(void) {
    FILE *fp = fopen(SAVE_PATH, "w");
    if (!fp) return;
    fprintf(fp, "%d", gs->level_current);
    fclose(fp);
}

static void load_game(void) {
    FILE *fp = fopen(SAVE_PATH, "r");
    if (!fp) return;
    fscanf(fp, "%d", &gs->level_current);
    fclose(fp);

    if (gs->level_current < 0 || gs->level_current > 11) gs->level_current = 0;
}
