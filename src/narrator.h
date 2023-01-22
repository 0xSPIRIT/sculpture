#define MAX_LINES 32

     //////////////////////////////////////////////////////////////
#define NARRATION_LEVEL_1 \
    "Nothing is created in a vacuum.\n"\
    "Other things in our heads generate new things.\n"\
    "At first, a hazy fog of inspiration, nudging us forward.\n"\
    "Then, it precipitates into nuggets of ideas.\n"\
    "Finally, the ideas themselves coalesce, hardening into reality.\n"\
    "You chisel away at the blockiness, imperfections, and unknowns.\n"\
    "Worlds of detail contained within the same block,\n"\
    "waiting to be revealed.\n"\
    "You inch towards the block of stone.\n"\
    "Something sparks inside you.\n"\

#define NARRATION_LEVEL_3 \
    "What should you create next? What shapes call to you?\n"\
    "Wondrous minutes went by, daydreaming fantastical possibilities.\n"\
    "Yes, you can fully picture it.\n"\
    "A beautiful symbolic piece, with endless complex details.\n"\
    "... No, you should temper your expectations, and your ambitions.\n"\
    "Only when you're good enough will you try something real.\n"\

#define NARRATION_LEVEL_6 \
    "Surely by now you were ready, right?\n"\
    "To do something detailed- important.\n"\
    "You thought about the sculptures you used to love.\n"\
    "Hypnotized by an idea, in reverie, you snap back.\n"\
    "With your expectations tempered, you pick up your chisel.\n"\

#define NARRATION_LEVEL_7 \
    "It went on and on,\n"\
    "but morphed into something else completely.\n"\
    "Deluding yourself into thinking it's possible.\n"\
    "At the end, you should be proud, but how could you?\n"\
    "With compromise after compromise,\n"\
    "this is all you could settle for?\n"\
    "A tiny fraction of what she could have been-\n"\
    "no, SHOULD have been.\n"\
    "It's like it was all for nothing.\n"\
    "\"I'll never make this mistake again,\" you say,\n"\
    "\"I'll never settle for anything less than what is expected.\"\n"\

#define NARRATION_LEVEL_9 \
    "A thought captures you once again,\n"\
    "condensing wildly into many ideas,\n"\
    "but this time, you're ready for it.\n"\
    "Beaming white shines from your eyes,\n"\
    "reflecting an object with layers of diamond and ice.\n"\
    "When you close your eyes, it's there.\n"\
    "The prize taunts you, aching for you to reveal it.\n"\
    "You can do this.\n"\
    "You know you can do this.\n"\

#define NARRATION_LEVEL_10 \
    "Armed with your honed artistry, you continue onwards.\n"\
    "You'd sculpt an intricate, sophisticated piece.\n"\
    "In fact, very similar to several of your favorites.\n"\
    "Your expertise should clean this one nicely.\n"\
    "You've earned a break.\n"\

#define NARRATION_END \
    "Fixed in place, you stare out your window.\n"\
    "You watch as the people go by.\n"\
    "Life goes on, but you are simply not there.\n"\
    "With the true thickness of your work in sight,\n"\
    "you realize you can't create a complex masterpiece;\n"\
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
