bool seconds_elapsed(f64 dt, f64 rate) {
    gs->main_timer += dt;
    if (gs->main_timer >= rate) {
        gs->main_timer-= rate;
        return true;
    }
    return false;
}