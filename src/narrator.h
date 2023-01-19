#define MAX_LINES 32

     //////////////////////////////////////////////////////////////
#define NARRATION_LEVEL_1 \
    "Nothing is created in a vacuum.\n"\
    "The remains of other things in our heads generate new things.\n"\
    "At first, a hazy fog of inspiration, nudging us forward.\n"\
    "Then, precipitating into nuggets of ideas.\n"\
    "Finally, the ideas themselves coalesce, hardening into reality.\n"\
    "You chisel away at the blockiness, imperfections, and unknowns.\n"\
    "Something sparks inside you.\n"\
    "You always were fascinated by them- the sculptures.\n"\
    "Worlds of detail contained within the same block,\n"\
    "waiting to be revealed.\n"\
    "You inch towards the odd thing.\n"\
    "Your eyes widen, like they've never before.\n"

#define NARRATION_LEVEL_3 \
    "Filled with simple joy, you quickly begin thinking.\n"\
    "What should you create next? What shapes call to you?\n"\
    "Of course, the first thing that came to mind was a human figure.\n"\
    "Wondrous minutes went by, daydreaming fantastical possibilities.\n"\
    "Yes, you can fully picture it.\n"\
    "A beautiful symbolic piece, with endless complex details.\n"
    "No, you should temper your expectations, and your ambitions.\n"\
    "Only when you're at a higher level of skill will you attempt.\n"

#define NARRATION_LEVEL_6 \
    "Surely by now you were ready, right?\n"\
    "To do something important. Detailed. Great.\n"\
    "You thought about the sculptures you used to love.\n"\
    "Hypnotized by an idea in reverie, you snap back.\n"\
    "With your expectations tempered, she'll be one of your greats!\n"\
    "Although she'd be more complex than your previous works,\n"\
    "the result certainly will pay for the extra work!\n"

#define NARRATION_LEVEL_7 \
    "She went on and on,\n"\
    "but slowly morphed into someone else completely.\n"\
    "Self-deluding into thinking you can finish it.\n"\
    "At the end, you should be proud, but how could you?\n"\
    "With compromise after compromise,\n"\
    "this is all you could settle for?\n"\
    "A tiny fraction of what she could have been--\n"\
    "no, SHOULD have been.\n"\
    "It's like it was all for nothing.\n"\
    "\"I'll never make this mistake again.\"\n"\
    "\"I will never settle for anything less than what is expected.\"\n"

#define NARRATION_LEVEL_9 \
    "You've been getting pretty good at this, huh?\n"\
    "A thought captures you once again,\n"\
    "condensing wildly into many ideas,\n"\
    "but this time, you're ready for it.\n"\
    "Your eyes glow with bright white,\n"\
    "reflecting an orb with layers of diamond and ice.\n"\
    "It's right in front of you when you close your eyes.\n"\
    "The prize taunts you in your head, aching for you to reveal it.\n"\
    "You can do this.\n"\
    "You know you can do this.\n"

#define NARRATION_LEVEL_10 \
    "Armed with your honed skills, you continue forth.\n"\
    "You'd create a piece with depth, and sophistication-\n"\
    "much like some of your favorite sculptures, in fact.\n"\
    "Your expertise should clean this one up nicely.\n"\
    "You've earned a break.\n"\

#define NARRATION_END \
    "You realize the self-delusion had never really stopped.\n"\
    "Was any of your accomplishments really worth anything?\n"\
    "What have you really created that allows you believe\n"\
    "you actually had what it takes?\n"\
    "The bare essentials?\n"\
    "Some pretentiousness?\n"\
    "Some piece bastardized by compromise?\n"\
    "Or the showpiece, with the vile insides?\n"\
    "...\n"\
    "Perhaps right now you can't create a truly complex masterpiece.\n"\
    "But maybe that's not such a bad thing.\n"\
    "Maybe it doesn't really matter.\n"\
    "...\n"\
    "Let's move to Alaska.\n"\
    "We'll sculpt the little trees and birds.\n"\
    "And it'll feel good.\n"

struct Narrator {
    char lines[MAX_LINES][256];
    int line_curr, line_count;
    int char_curr;
    size_t curr_len;
    
    int time;
    
    bool black;
};
