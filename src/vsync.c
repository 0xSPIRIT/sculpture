// Simulated vsync for when the screen's refresh rate is not 60.
// (In that case, we just use SDL's vsync which is smoother)

void precise_sleep(f64 seconds) {
    if (seconds < 0) return;
    
    static f64 estimate = 0.005;
    static f64 mean = 0.005;
    static f64 m2 = 0;
    static i64 count = 1;
    
    f64 freq = SDL_GetPerformanceFrequency();
    
    // Some sleeps
    while (seconds > estimate) {
        u64 start = SDL_GetPerformanceCounter();
        SDL_Delay(1);
        u64 end = SDL_GetPerformanceCounter();
        
        f64 observed = ((f64)(end-start)) / freq;
        seconds -= observed;
        
        count++;
        f64 delta = (observed - mean);
        mean += delta / count;
        m2   += delta * (observed - mean);
        f64 std_dev = sqrt(m2 / (count-1));
        estimate = mean + std_dev;
    }
    
    // Spin lock
    u64 start = SDL_GetPerformanceCounter();
    while (((f64)SDL_GetPerformanceCounter()-start)/freq < seconds);
}
