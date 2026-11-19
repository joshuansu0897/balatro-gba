/**
 * @file wiki_entries.c
 *
 * @brief Wiki entries implementation.
 */
#include "game/wiki_entries.h"

#include "boss_blind_effects.h"
#include "util.h"

// clang-format off
// WHY: the game has no name or description for its Skip Tags, these are written from what each
// one does in skip_tag_effects.c
static const WikiTagEntry WIKI_TAGS[] = {
    {SKIP_TAG_TYPE_INVESTMENT, "Investment Tag", "Gain $25 after you defeat the Boss Blind"      },
    {SKIP_TAG_TYPE_BOSS,       "Boss Tag",       "Rerolls the Boss Blind"                        },
    {SKIP_TAG_TYPE_HANDY,      "Handy Tag",      "Gives $1 for each hand played this run"        },
    {SKIP_TAG_TYPE_GARBAGE,    "Garbage Tag",    "Gives $1 for each unused discard this run"     },
    {SKIP_TAG_TYPE_DOUBLE,     "Double Tag",     "Gives a copy of the next Tag you get"          },
    {SKIP_TAG_TYPE_JUGGLE,     "Juggle Tag",     "+3 hand size for the next Round"               },
    {SKIP_TAG_TYPE_D6,         "D6 Tag",         "Rerolls start at $0 in the next Shop"          },
    {SKIP_TAG_TYPE_TOP_UP,     "Top-up Tag",     "Creates up to 2 Common Jokers if you have room"},
    {SKIP_TAG_TYPE_SPEED,      "Speed Tag",      "Gives $5 for each Blind skipped this run"      },
    {SKIP_TAG_TYPE_ECONOMY,    "Economy Tag",    "Doubles your money, up to $40 more"            },
};

static const enum BlindType WIKI_BOSSES[] = {
    BLIND_TYPE_HOOK,  BLIND_TYPE_OX,     BLIND_TYPE_PSYCHIC, BLIND_TYPE_WATER,
    BLIND_TYPE_EYE,   BLIND_TYPE_NEEDLE, BLIND_TYPE_TOOTH
};
// clang-format on

// WHY: the text The Ox shows in its Round names a hand that is only known once the Round starts
static const char WIKI_OX_TEXT[] = "Playing your most played hand sets your money to $0";

int wiki_entries_get_nb_tags(void)
{
    return NUM_ELEM_IN_ARR(WIKI_TAGS);
}

const WikiTagEntry* wiki_entries_get_tag(int tag_idx)
{
    if (tag_idx < 0 || tag_idx >= wiki_entries_get_nb_tags())
        return NULL;

    return &WIKI_TAGS[tag_idx];
}

int wiki_entries_get_nb_bosses(void)
{
    return NUM_ELEM_IN_ARR(WIKI_BOSSES);
}

enum BlindType wiki_entries_get_boss(int boss_idx)
{
    if (boss_idx < 0 || boss_idx >= wiki_entries_get_nb_bosses())
        return BLIND_TYPE_MAX;

    return WIKI_BOSSES[boss_idx];
}

const char* wiki_entries_get_boss_text(enum BlindType boss)
{
    if (boss_blind_is_ox_active(boss))
        return WIKI_OX_TEXT;

    return boss_blind_get_effect_text(boss);
}
