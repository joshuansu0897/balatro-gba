#include <assert.h>
#include <boss_blind_effects.h>
#include <stdio.h>
#include <string.h>

// Same values as enum HandType in the game
#define HIGH_CARD       1
#define PAIR            2
#define TWO_PAIR        3
#define THREE_OF_A_KIND 4
#define STRAIGHT        5
#define FLUSH           6
#define NB_HAND_TYPES   13

// The screen is 30 characters wide
#define SCREEN_WIDTH_IN_CHARS 30

static const enum BlindType BLINDS_WITHOUT_EFFECT[] = {
    BLIND_TYPE_SMALL,
    BLIND_TYPE_BIG,
    BLIND_TYPE_WALL,
    BLIND_TYPE_MANACLE,
    BLIND_TYPE_ACORN
};
#define NB_BLINDS_WITHOUT_EFFECT (sizeof(BLINDS_WITHOUT_EFFECT) / sizeof(BLINDS_WITHOUT_EFFECT[0]))

void test_blinds_with_an_effect(void)
{
    assert(boss_blind_has_effect(BLIND_TYPE_HOOK));
    assert(boss_blind_has_effect(BLIND_TYPE_OX));
    assert(boss_blind_has_effect(BLIND_TYPE_PSYCHIC));
    assert(boss_blind_has_effect(BLIND_TYPE_WATER));
    assert(boss_blind_has_effect(BLIND_TYPE_EYE));
    assert(boss_blind_has_effect(BLIND_TYPE_NEEDLE));
    assert(boss_blind_has_effect(BLIND_TYPE_TOOTH));

    for (size_t i = 0; i < NB_BLINDS_WITHOUT_EFFECT; i++)
        assert(!boss_blind_has_effect(BLINDS_WITHOUT_EFFECT[i]));
}

void test_needle_leaves_one_hand(void)
{
    int hands = 4, discards = 4, hand_size = 8;
    boss_blind_apply_round_start(BLIND_TYPE_NEEDLE, &hands, &discards, &hand_size);
    assert(hands == 1 && discards == 4 && hand_size == 8);
}

void test_water_leaves_no_discard(void)
{
    int hands = 4, discards = 4, hand_size = 8;
    boss_blind_apply_round_start(BLIND_TYPE_WATER, &hands, &discards, &hand_size);
    assert(hands == 4 && discards == 0 && hand_size == 8);
}

void test_other_blinds_start_the_round_unchanged(void)
{
    int hands = 4, discards = 4, hand_size = 8;
    boss_blind_apply_round_start(BLIND_TYPE_HOOK, &hands, &discards, &hand_size);
    boss_blind_apply_round_start(BLIND_TYPE_PSYCHIC, &hands, &discards, &hand_size);
    boss_blind_apply_round_start(BLIND_TYPE_TOOTH, &hands, &discards, &hand_size);
    boss_blind_apply_round_start(BLIND_TYPE_EYE, &hands, &discards, &hand_size);
    boss_blind_apply_round_start(BLIND_TYPE_OX, &hands, &discards, &hand_size);
    for (size_t i = 0; i < NB_BLINDS_WITHOUT_EFFECT; i++)
        boss_blind_apply_round_start(BLINDS_WITHOUT_EFFECT[i], &hands, &discards, &hand_size);

    assert(hands == 4 && discards == 4 && hand_size == 8);
}

void test_psychic_only_lets_five_cards_score(void)
{
    assert(!boss_blind_validate_play(BLIND_TYPE_PSYCHIC, 1, HIGH_CARD));
    assert(!boss_blind_validate_play(BLIND_TYPE_PSYCHIC, 4, TWO_PAIR));
    assert(boss_blind_validate_play(BLIND_TYPE_PSYCHIC, 5, STRAIGHT));
    assert(boss_blind_validate_play(BLIND_TYPE_PSYCHIC, 5, HIGH_CARD));
}

void test_eye_lets_each_hand_type_score_once_per_round(void)
{
    boss_blind_reset();
    assert(boss_blind_validate_play(BLIND_TYPE_EYE, 1, HIGH_CARD));

    boss_blind_register_hand(BLIND_TYPE_EYE, HIGH_CARD);
    assert(!boss_blind_validate_play(BLIND_TYPE_EYE, 1, HIGH_CARD));
    assert(boss_blind_validate_play(BLIND_TYPE_EYE, 2, PAIR));

    // A new Round starts
    boss_blind_reset();
    assert(boss_blind_validate_play(BLIND_TYPE_EYE, 1, HIGH_CARD));
}

void test_eye_only_remembers_hands_when_it_is_the_blind(void)
{
    boss_blind_reset();
    boss_blind_register_hand(BLIND_TYPE_HOOK, PAIR);
    assert(boss_blind_validate_play(BLIND_TYPE_EYE, 2, PAIR));

    // Hand types out of range are ignored instead of written out of bounds
    boss_blind_register_hand(BLIND_TYPE_EYE, -1);
    boss_blind_register_hand(BLIND_TYPE_EYE, BOSS_BLIND_MAX_HAND_TYPES);
    assert(boss_blind_validate_play(BLIND_TYPE_EYE, 5, BOSS_BLIND_MAX_HAND_TYPES));
}

void test_other_blinds_let_every_hand_score(void)
{
    boss_blind_reset();
    boss_blind_register_hand(BLIND_TYPE_EYE, FLUSH);

    assert(boss_blind_validate_play(BLIND_TYPE_NEEDLE, 3, FLUSH));
    assert(boss_blind_validate_play(BLIND_TYPE_WATER, 3, FLUSH));
    assert(boss_blind_validate_play(BLIND_TYPE_HOOK, 3, FLUSH));
    assert(boss_blind_validate_play(BLIND_TYPE_TOOTH, 3, FLUSH));
    assert(boss_blind_validate_play(BLIND_TYPE_OX, 3, FLUSH));
    for (size_t i = 0; i < NB_BLINDS_WITHOUT_EFFECT; i++)
        assert(boss_blind_validate_play(BLINDS_WITHOUT_EFFECT[i], 3, FLUSH));
}

void test_only_the_hook_discards_held_cards(void)
{
    assert(boss_blind_get_hook_count(BLIND_TYPE_HOOK) == 2);
    assert(boss_blind_get_hook_count(BLIND_TYPE_NEEDLE) == 0);
    assert(boss_blind_get_hook_count(BLIND_TYPE_OX) == 0);
    for (size_t i = 0; i < NB_BLINDS_WITHOUT_EFFECT; i++)
        assert(boss_blind_get_hook_count(BLINDS_WITHOUT_EFFECT[i]) == 0);
}

void test_tooth_takes_one_dollar_per_card_played(void)
{
    assert(boss_blind_get_tooth_penalty(BLIND_TYPE_TOOTH, 1) == 1);
    assert(boss_blind_get_tooth_penalty(BLIND_TYPE_TOOTH, 5) == 5);
    assert(boss_blind_get_tooth_penalty(BLIND_TYPE_TOOTH, 0) == 0);
    assert(boss_blind_get_tooth_penalty(BLIND_TYPE_TOOTH, -1) == 0);
    assert(boss_blind_get_tooth_penalty(BLIND_TYPE_HOOK, 3) == 0);
    assert(boss_blind_get_tooth_penalty(BLIND_TYPE_OX, 3) == 0);
}

void test_ox_only_punishes_the_most_played_hand(void)
{
    assert(boss_blind_is_ox_active(BLIND_TYPE_OX));
    assert(!boss_blind_is_ox_active(BLIND_TYPE_TOOTH));

    boss_blind_set_ox_hand(PAIR);
    assert(boss_blind_get_ox_hand() == PAIR);
    assert(boss_blind_is_ox_triggered(BLIND_TYPE_OX, PAIR));
    assert(!boss_blind_is_ox_triggered(BLIND_TYPE_OX, FLUSH));

    // Another Blind never takes the money, even for that hand
    assert(!boss_blind_is_ox_triggered(BLIND_TYPE_TOOTH, PAIR));
    assert(!boss_blind_is_ox_triggered(BLIND_TYPE_SMALL, PAIR));
}

void test_most_played_hand(void)
{
    uint32_t nb_played_hands[NB_HAND_TYPES] = {0};

    // Nothing played yet, both Blinds before the Boss can be skipped
    assert(boss_blind_find_most_played_hand(nb_played_hands, NB_HAND_TYPES) == HIGH_CARD);

    nb_played_hands[PAIR - 1] = 3;
    nb_played_hands[FLUSH - 1] = 7;
    nb_played_hands[HIGH_CARD - 1] = 1;
    assert(boss_blind_find_most_played_hand(nb_played_hands, NB_HAND_TYPES) == FLUSH);

    // The lowest hand type wins a tie
    nb_played_hands[PAIR - 1] = 7;
    assert(boss_blind_find_most_played_hand(nb_played_hands, NB_HAND_TYPES) == PAIR);

    // The last entry is read too
    nb_played_hands[NB_HAND_TYPES - 1] = 8;
    assert(boss_blind_find_most_played_hand(nb_played_hands, NB_HAND_TYPES) == NB_HAND_TYPES);
}

/**
 * Text is centered on a grid of whole characters, which is only exact for an even number of them.
 */
static void assert_text_can_be_centered(const char* text)
{
    assert(text != NULL);
    assert(strlen(text) > 0);
    assert(strlen(text) % 2 == 0);
    assert(strlen(text) <= SCREEN_WIDTH_IN_CHARS);
    assert(strlen(text) < BOSS_BLIND_EFFECT_TEXT_SIZE);
}

void test_effect_texts(void)
{
    assert_text_can_be_centered(boss_blind_get_effect_text(BLIND_TYPE_HOOK));
    assert_text_can_be_centered(boss_blind_get_effect_text(BLIND_TYPE_PSYCHIC));
    assert_text_can_be_centered(boss_blind_get_effect_text(BLIND_TYPE_WATER));
    assert_text_can_be_centered(boss_blind_get_effect_text(BLIND_TYPE_EYE));
    assert_text_can_be_centered(boss_blind_get_effect_text(BLIND_TYPE_NEEDLE));
    assert_text_can_be_centered(boss_blind_get_effect_text(BLIND_TYPE_TOOTH));

    // The text of The Ox names a hand, it is built by boss_blind_format_ox_text()
    assert(boss_blind_get_effect_text(BLIND_TYPE_OX) == NULL);
    for (size_t i = 0; i < NB_BLINDS_WITHOUT_EFFECT; i++)
        assert(boss_blind_get_effect_text(BLINDS_WITHOUT_EFFECT[i]) == NULL);
}

void test_ox_text(void)
{
    // Every hand name shown by the game
    static const char* const HAND_NAMES[] = {
        "Hi-Card", "Pair",   "2 Pair",  "3 OAK", "Strt",    "Flush",  "Full H",
        "4 OAK",   "Strt F", "Royal F", "5 OAK", "Flush H", "Flush 5"
    };
    char text[BOSS_BLIND_EFFECT_TEXT_SIZE];

    for (size_t i = 0; i < sizeof(HAND_NAMES) / sizeof(HAND_NAMES[0]); i++)
    {
        boss_blind_format_ox_text(text, sizeof(text), HAND_NAMES[i]);

        assert_text_can_be_centered(text);
        assert(strncmp(text, HAND_NAMES[i], strlen(HAND_NAMES[i])) == 0);
        assert(strstr(text, "$0") != NULL);
    }

    // A buffer too small never overflows
    char small_text[8];
    boss_blind_format_ox_text(small_text, sizeof(small_text), "Flush 5");
    assert(strlen(small_text) == sizeof(small_text) - 1);
}

int main()
{
    printf("Testing Boss Blind effects.\n");
    test_blinds_with_an_effect();
    test_needle_leaves_one_hand();
    test_water_leaves_no_discard();
    test_other_blinds_start_the_round_unchanged();
    test_psychic_only_lets_five_cards_score();
    test_eye_lets_each_hand_type_score_once_per_round();
    test_eye_only_remembers_hands_when_it_is_the_blind();
    test_other_blinds_let_every_hand_score();
    test_only_the_hook_discards_held_cards();
    test_tooth_takes_one_dollar_per_card_played();
    test_ox_only_punishes_the_most_played_hand();
    test_most_played_hand();
    test_effect_texts();
    test_ox_text();

    printf("-------------------------------------------------------------------------------\n");
    printf("Boss Blind Effects Tests Passed :)\n");
    printf("-------------------------------------------------------------------------------\n");

    return 0;
}
