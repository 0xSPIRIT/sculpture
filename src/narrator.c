void narrator_init(char **lines, int count) {
    memset(gs->narrator, 0, sizeof(struct Narrator));
    
    for (int i = 0; i < count; i++) {
        strcpy(gs->narrator.lines[i], lines[i]);
    }
    
    gs->narrator.line_count = count;
}

void narrator_run() {
    //draw_text(...)
}