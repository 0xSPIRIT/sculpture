#define MAX_LINES 32

#define NARRATOR_EPSILON 0.5
#define NARRATOR_ALPHA 10
#define NARRATOR_HANG_TIME 14 // Frames

     ////////////////////////////////////////////////////////////////
#define NARRATION_LEVEL_1 \
"You inch towards three blocks of stone,\n"\
"accompanied by an array of chisels.\n"\
"As your gaze intensifies,"
"a spark previously dormant ignites in you.\n"\

#define NARRATION_LEVEL_3 \
"As you acquaint yourself with the basics,\n"\
"your imagination fills with exotic new ideas.\n"\
"It scratches an itch you never knew you had,\n"\
"daydreaming even outside of the workshop.\n"\
"Your single flame glares with bliss.\n"\

#define NARRATION_LEVEL_4 \
"Which shapes and patterns beg to be sculpted?\n"\
"Wondrous minutes went by,\n"\
"daydreaming fantastical possibilities.\n"\
"You can fully picture it:\n"\
"A symbolic piece with endless complex details.\n"\
"... No.\n"\
"Only when you've sharpened your skills,\n"\
"will you dare to create something real.\n"\
"\n"\
"The flame flickers for a moment.\n"\

#define NARRATION_LEVEL_7 \
"Surely by now you were ready, right?\n"\
"To do something detailed- important.\n"\
"You recall the sculptures that you once loved.\n"\
"Hypnotized by an idea, you return.\n"\
"Your expectations surely tempered, you pick up your chisel.\n"\

#define NARRATION_LEVEL_8 \
"At the end you should be proud, but how could you?\n"\
"It's a sudden jolt back to reality-\n"\
"To the detached, and dispassionate.\n"\
"With compromise after compromise,\n"\
"this is all you could settle for?\n"\
"A bastardized fraction of what it should have been?\n"\
"It was all for nothing.\n"\
"\"I'll never make this mistake again,\" you say,\n"\
"\"I'll never settle for anything less than what is expected.\"\n"\

#define NARRATION_LEVEL_10 \
"A thought captures you once again,\n"\
"liquifying boldly into many ideas,\n"\
"but this time, you're ready.\n"\
"Glaring white radiates from your imagination,\n"\
"outlining an object with layers of diamond and ice.\n"\
"The prize taunts you. It aches for you to reveal it.\n"\
"You can do this.\n"\
"You know you can.\n"\
"You have to.\n"\

#define NARRATION_LEVEL_11 \
"With your perfected craftsmanship, you continue on.\n"\
"You'd sculpt an intricate, sophisticated piece-\n"\
"In fact, very similar to several of your favorites.\n"\
"Your expertise should clean this one up nicely.\n"\
"You've earned a break.\n"\

#define NARRATION_END \
"Fixed in place, you stare out your window.\n"\
"You watch as the people go by.\n"\
"Life goes on, but you are simply not there.\n"\
"With the true depth of your work in sight,\n"\
"you realize you can't do what they did;\n"\
"yearning to masquerade around as some auteur.\n"\
"Maybe that's not such a bad thing.\n"\
"...\n"\
"Let's move to Alaska.\n"\
"We'd sculpt the little birds and trees.\n"\
"And it'd feel good.\n"\

struct Narrator {
    char lines[MAX_LINES][256];
    int line_curr, line_count;
    size_t curr_len;
    
    int delay;
    
    bool fadeout; // This is for text
    
    f64 alpha;
    f64 hang_time; // The hang time between each line
    bool first_frame;
    
    bool off;
};