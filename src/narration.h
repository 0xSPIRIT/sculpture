#define MAX_LINES 32

#define NARRATOR_EPSILON 0.5
#define NARRATOR_ALPHA 10
#define NARRATOR_HANG_TIME 14 // Frames

#ifdef ALASKA_RELEASE_MODE
  #define SHOW_NARRATION 1
#else
  #define SHOW_NARRATION 0
#endif

////////////////////////////////////////////////////////////////
#define NARRATION_LEVEL_1 \
"Max inched towards three blocks of stone,\r"\
"accompanied by an array of chisels.\n"\
"His gaze intensified at the marble curiously,\r"\
"longing for something hidden, locked away.\n"\
"A glimmer previously dormant ignited in him.\n"\

#define NARRATION_LEVEL_3 \
"Max's imagination filled with exotic new ideas\r"\
"as he acquainted himself with the basics.\n"\
"It scratched an itch previously unknown,\r"\
"daydreaming even outside of the workshop.\n"\
"His single flame glared in bliss.\n"\

#define NARRATION_LEVEL_4 \
"Wondrous minutes went by with Max daydreaming\r"\
"fantastical possibilities, with one most alluring.\n"\
"He could fully picture it: a symbolic piece with\r"\
"endless complex details.\n"\
"... No.\n"\
"Only once he sharpened his skills would he\r"\
"dare create something actually real.\n"\
"Max thought to himself,\r"\
"\"These sculptures are nothing other than\r"\
"fodder for future projects.\"\n"\
"For a moment, his flame flickered.\n"\

#define NARRATION_LEVEL_5 \
"When he was younger, Max\r"\
"frequently admired popular artwork.\n"\
"Over the years, dozens etched into his head,\r"\
"combining into a soup of likes and dislikes-\n"\
"Flowing streams, interlaced with notions of\r"\
"greatness, waiting to be.\n"\
"Now, the liquid that had formed precipitated\r"\
"intensely from Max's mind.\n"\

#define NARRATION_LEVEL_7 \
"Surely by then he must've been ready...?\n"\
"He understood his previous sculptures only served as\r"\
"stepping stones to something greater,\r"\
"but it's been too long.\n"\
"He must move forward to do something real.\r"\
"Something noteworthy.\n"\
"Max fantasized sculptures that he admired-\r"\
"They swirled around in his head in a moment of inspiration.\n"\
"Too, his mind spun with other hypnotic ideas that\r"\
"would surely never come to pass.\n"\
"In reverie, he returned to the here and now.\n"\
"He thought of an idea that must've been realistic-\r"\
"that his expectations were surely tempered.\n"\
"With no hesitation, Max picked up his chisel.\n"\

#define NARRATION_LEVEL_8 \
"At the end he should've been proud, but how could he?\n"\
"Max turned away pointedly, his chisel clattering\r"\
"to the hardwood floor in contempt.\n"\
"His eyes flinched in an instinctual repulsion.\n"\
"He didn't think that something like this would have\r"\
"affected him so deeply, but it did.\n"\
"It's a sudden jolt back to the detached,\r"\
"and dispassionate.\n"\
"He thought, with compromise after compromise,\r"\
"this is all he could have settled for?\n"\
"A bastardized fraction of what it could have been--\r"\
"No, SHOULD have been?\n"\
"Max, in a fit of rage, clenched the sculpture in his hands\r"\
"and smashes it to the floor, chucking shards of stone\r"\
"all across his workshop.\n"\
"\"I'll never make this mistake again,\"\r"\
"he declared.\n"\

#define NARRATION_LEVEL_10 \
"A thought captured him once again,\r"\
"crystallizing wildly into many ideas,\r"\
"but this time, he was ready.\n"\
"A glittering white radiated from Max's imagination,\r"\
"outlining an object with layers of diamond and ice.\n"\
"The prize taunted Max, aching to reveal it.\n"\
"He said,\r"\
"\"I can do this.\"\n"\
"\"I know I can- I have to.\"\n"\

#define NARRATION_LEVEL_11 \
"Max collapsed into his chair, debilitated,\r"\
"the floorboards creaking in response.\n"\
"Little emotion shone through him as he\r"\
"carefully placed his finished piece next the others.\n"\
"With his seemingly perfected craftsmanship,\r"\
"without a moment's rest nor hesitation,\r"\
"he continued on.\n"\
"He'd sculpt an intricate, sophisticated piece, yes.\r"\
"In fact, very similar to some of his favorites.\n"\
"His new expertise should clean this one up nicely.\n"\

#define NARRATION_END \
"Fixed in place, Max stares out his window.\n"\
"He watches as the people go by.\n"\
"Life goes on, but he is simply not there.\n"\
"With the true depth of his work in sight,\r"\
"he realizes he can't do what they did;\n"\
"yearning to masquerade around as some auteur.\n"\
"Maybe that's not such a bad thing.\n"\
"...\n"\
"Let's move to Alaska.\n"\
"We'll sculpt the little birds and trees.\n"\
"And it'll feel good.\n"


typedef struct Narrator {
    char current_lines[10][256];
    int current_line_count;

    char lines[MAX_LINES][256];
    int line_curr, line_count;

    int delay;
    bool update; // Should we update the narrations text surface? narrator_next_line

    bool fadeout; // This is for text

    f64 alpha;
    f64 hang_time; // The hang time between each line
    bool first_frame;

    f64 target_time;
    bool red;

    bool off;
} Narrator;
