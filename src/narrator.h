#define MAX_LINES 32

     //////////////////////////////////////////////////////////////
#define NARRATION_LEVEL_1 \
    "A curious object spots you in the corner of your eye-\n"\
    "A square block of stone, and alongside were chisels.\n"\
    "Something sparked inside, igniting a flame previously dormant.\n"\
    "You always were fascinated by them- the sculptures.\n"\
    "Worlds of detail contained within the same block,\n"\
    "waiting to be revealed.\n"\
    "Interested, you inch towards the odd thing.\n"\
    "Your eyes widen, like they've never before.\n"

#define NARRATION_LEVEL_2 \
    "Your heart pulsing with simple joy, you quickly begin thinking.\n"\
    "What should you create next? What shapes call to you?\n"\
    "Of course, the first thing that came to mind was a human figure.\n"\
    "Wondrous minutes went by, daydreaming fantastical possibilities.\n"\
    "No, you should temper your expectations, and your ambitions.\n"\
    "Your eyes gaze to the fireplace, and it stares back, knowingly.\n"\

#define NARRATION_LEVEL_3 \
    "Nothing is created in a vacuum.\n"\
    "The remains of other things in our minds generate new things.\n"\
    "At first, a hazy fog of inspiration, nudging us forward.\n"\
    "Then, precipitating into nuggets of ideas.\n"\
    "Finally, the ideas themselves coalesce, hardening into reality.\n"\
    "You chisel away at the blockiness, imperfections, and unknowns.\n"\

struct Narrator {
    char lines[MAX_LINES][256];
    int line_curr, line_count;
    int char_curr;
    size_t curr_len;
    
    int time;
    
    bool black;
};
