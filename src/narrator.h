#define MAX_LINES 32

     ////////////////////////////////////////////////////////////////
#define NARRATION_LEVEL_1 \
    "You inch towards the block of stone,\n"\
    "accompanied by an assortment of chisels.\n"\
    "You were always fascinated by them- the sculptures.\n"\
    "Worlds of detail tucked behind the stone,\n"\
    "waiting to be revealed.\n"\
    "Your eyes widen like never before,\n"\
    "a flame previously dormant sparking in you.\n"\

#define NARRATION_LEVEL_3 \
    "The single flame grows, blossoming into blazing fire.\n"\
    "What whispers of inspiration do you hear?\n"\
    "Which shapes and patterns beg to be sculpted?\n"\
    "Wondrous minutes went by, daydreaming fantastical possibilities.\n"\
    "You can fully picture it.\n"\
    "A beautiful symbolic piece, with endless complex details.\n"\
    "... No.\n"\
    "Only when you've sharpened your skills,\n"\
    "will you dare to unleash your full potential.\n"\
    "...\n"\
    "The flame flickers for a moment.\n"\

#define NARRATION_LEVEL_6 \
    "Surely by now you were ready, right?\n"\
    "To do something detailed- important...\n"\
    "You remember the sculptures that once danced in your mind's eye.\n"\
    "Hypnotized by an idea, in reverie, you snap back.\n"\
    "With your expectations tempered, you pick up your chisel.\n"\

#define NARRATION_LEVEL_7 \
    "At the end you should be proud of it, but how could you?\n"\
    "It's a sudden jolt back to reality-\n"\
    "to the detached and dispassionate.\n"\
    "With compromise after compromise,\n"\
    "this is all you could settle for?\n"\
    "A tiny fraction of what it could have been-\n"\
    "No, SHOULD have been.\n"\
    "It's like it was all for nothing.\n"\
    "\"I'll never make this mistake again,\" you say,\n"\
    "\"I'll never settle for anything less than what is expected.\"\n"\

#define NARRATION_LEVEL_9 \
    "A thought captures you once again,\n"\
    "condensing wildly into many ideas,\n"\
    "but this time, you're ready for it.\n"\
    "Lucid white radiates from your mind's eye,\n"\
    "reflecting an object with layers of diamond and ice.\n"\
    "The prize taunts you, aching for you to reveal it.\n"\
    "You can do this.\n"\
    "You know you can do this.\n"\

#define NARRATION_LEVEL_10 \
    "Armed with your honed artistry, you continue onwards.\n"\
    "You'd sculpt an intricate, sophisticated piece.\n"\
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
    "And it'll feel good.\n"\

struct Narrator {
    char lines[MAX_LINES][256];
    int line_curr, line_count;
    int char_curr;
    size_t curr_len;
    
    int time;
    
    bool black;
    bool off;
};
