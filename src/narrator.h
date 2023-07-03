#define MAX_LINES 32

#define NARRATOR_EPSILON 0.5
#define NARRATOR_ALPHA 10
#define NARRATOR_HANG_TIME 14 // Frames

#define SHOW_NARRATION ALASKA_RELEASE_MODE

////////////////////////////////////////////////////////////////
#define NARRATION_LEVEL_1 \
"Max inches towards three blocks of stone,\r"\
"accompanied by an array of chisels.\n"\
"His gaze intensifies at the marble curiously, longing for\r"\
"something hidden, locked away.\n"\
"A glimmer previously dormant ignites in him.\n"\

#define NARRATION_LEVEL_3 \
"Max's imagination fills with exotic new ideas\r"\
"as he acquainted himself with the basics.\n"\
"It scratched an itch previously unknown,\r"\
"daydreaming even outside of the workshop.\n"\
"His single flame glares in bliss.\n"\

#define NARRATION_LEVEL_4 \
"Which shapes beg to be sculpted?\n"\
"Wondrous minutes went by, with Max daydreaming\r"\
"fantastical possibilities.\n"\
"He could fully picture it: a symbolic piece with\r"\
"endless complex details.\n"\
"... No.\n"\
"Only when he'd sharpened his skills will he\r"\
"dare create something real.\n"\
"For a moment, his flame flickers.\n"\

// TODO: Do one for level 5.
// - Talk about taking in the processed remains
//   of other works, and converting it into something
//   new.

// This is in the wrong tense, but it sounds weird
// if you correct it to present tense!!!
// Should I change everything else to past tense?
#define NARRATION_LEVEL_7 \
"Surely by now he must be ready.\n"\
"To do something real.\rSomething important.\n"\
"Max recalled sculptures that he knew fondly.\r"\
"They swirled around his head in a moment of inspiration.\n"\
"Too, his mind spun with other hypnotic notions that\r"\
"would surely never come to pass--\n"\
"Though, he'd never tell anyone\r"\
"what he thought to be foolish fantasies.\n"\
"In reverie, he returned to the here and now.\n"\
"His expectations surely tempered, he picked up his chisel.\n"\

#define NARRATION_LEVEL_8 \
"At the end he should be proud, but how could he?\n"\
"Max turned away pointedly, his chisel clattering\r"\
"to the hardwood floor in contempt.\n"\
"His eyes flinched in an instinctual repulsion.\n"\
"He'd never thought that something like this would have\r"\
"affected him so deeply-- but it did.\n"\
"It's a sudden jolt back to the detached\r"\
"and dispassionate.\n"\
"He thought, with compromise after compromise,\r"\
"this is all he could have settled for?\n"\
"A bastardized fraction of what it could have been-- no,\r"\
"SHOULD have been?\n"\
"Max, in a fit of rage, clenches the single flower in hands\r"\
"and smashes it to the floor, chucking shards of marble\r"\
"and quartz across his workshop.\n"\
"\"I'll never make this mistake again,\" he declared.\n"\

#define NARRATION_LEVEL_10 \
"A thought captures him once again,\r"\
"crystallizing wildly into many ideas,\r"\
"but this time, he's ready.\n"\
"A glittering white radiates from Max's imagination,\r"\
"outlining an object with layers of diamond and ice.\n"\
"The prize taunts him, aching for him to reveal it.\n"\
"Max says,\r"\
"\"I can do this.\"\n"\
"\"I know I can do this.\"\n"\
"\"I have to.\"\n"\

#define NARRATION_LEVEL_11 \
"Max collapses into his chair, debilitated,\r"\
"the floorboards creaking in response.\n"\
"Little emotion seemed to shine through his demeanor,\r"\
"as he carefully places his finished piece next the others.\n"\
"With his seemingly perfected craftsmanship,\r"\
"without a moment's rest, nor hesitation,\r"\
"he continues on.\n"\
"He'd sculpt an intricate, sophisticated piece, yes.\r"\
"In fact, very similar to some of his favorites.\n"\
"His new expertise should clean this one nicely;\r"\
"he's earned a break.\n"\

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
"We'd sculpt the little birds and trees.\n"\
"And it'd feel good.\n"\

typedef struct Narrator {
    char current_lines[10][256];
    int current_line_count;

    char lines[MAX_LINES][256];
    int line_curr, line_count;
    size_t curr_len;

    int delay;
    bool update; // Should we update the narrations text surface? narrator_next_line

    bool fadeout; // This is for text

    f64 alpha;
    f64 hang_time; // The hang time between each line
    bool first_frame;

    f64 glitch_time;
    f64 target_time;
    bool red;

    bool off;
} Narrator;

static void narrator_next_line(bool init);
static int get_glitched_offset(void);
