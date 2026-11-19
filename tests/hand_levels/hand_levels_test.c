#include <assert.h>
#include <hand_levels.h>
#include <stdio.h>

// Same values as enum HandType in the game
#define NONE            0
#define HIGH_CARD       1
#define PAIR            2
#define FLUSH           6
#define STRAIGHT_FLUSH  9
#define ROYAL_FLUSH     10
#define FIVE_OF_A_KIND  11
#define FLUSH_HOUSE     12
#define FLUSH_FIVE      13

void test_every_hand_owns_its_level_except_the_royal_flush(void)
{
    assert(hand_levels_get_owner(HIGH_CARD) == HIGH_CARD);
    assert(hand_levels_get_owner(PAIR) == PAIR);
    assert(hand_levels_get_owner(STRAIGHT_FLUSH) == STRAIGHT_FLUSH);
    assert(hand_levels_get_owner(FLUSH_FIVE) == FLUSH_FIVE);

    assert(hand_levels_get_owner(ROYAL_FLUSH) == STRAIGHT_FLUSH);
}

void test_values_that_are_not_hands_own_no_level(void)
{
    assert(hand_levels_get_owner(NONE) == HAND_LEVELS_NO_HAND);
    assert(hand_levels_get_owner(-1) == HAND_LEVELS_NO_HAND);
    assert(hand_levels_get_owner(FLUSH_FIVE + 1) == HAND_LEVELS_NO_HAND);
}

void test_level_ups_stop_at_the_highest_level(void)
{
    assert(hand_levels_add_level_up(0) == 1);
    assert(hand_levels_add_level_up(41) == 42);

    // A level 1 hand has 0 level ups, so the highest level is one less in level ups
    uint32_t nb_level_ups_at_max = HAND_LEVELS_MAX_LEVEL - 1;
    assert(hand_levels_add_level_up(nb_level_ups_at_max - 1) == nb_level_ups_at_max);
    assert(hand_levels_add_level_up(nb_level_ups_at_max) == nb_level_ups_at_max);
    assert(hand_levels_add_level_up(nb_level_ups_at_max + 50) == nb_level_ups_at_max);
}

void test_regular_hands_always_have_a_planet(void)
{
    for (int hand_type = HIGH_CARD; hand_type <= STRAIGHT_FLUSH; hand_type++)
    {
        assert(hand_levels_has_planet(hand_type, 0));
        assert(hand_levels_has_planet(hand_type, 5));
    }
}

void test_royal_flush_has_no_planet_of_its_own(void)
{
    assert(!hand_levels_has_planet(ROYAL_FLUSH, 0));
    assert(!hand_levels_has_planet(ROYAL_FLUSH, 3));
}

void test_secret_hands_have_a_planet_once_played(void)
{
    assert(!hand_levels_has_planet(FIVE_OF_A_KIND, 0));
    assert(!hand_levels_has_planet(FLUSH_HOUSE, 0));
    assert(!hand_levels_has_planet(FLUSH_FIVE, 0));

    assert(hand_levels_has_planet(FIVE_OF_A_KIND, 1));
    assert(hand_levels_has_planet(FLUSH_HOUSE, 2));
    assert(hand_levels_has_planet(FLUSH_FIVE, 7));
}

void test_values_that_are_not_hands_have_no_planet(void)
{
    assert(!hand_levels_has_planet(NONE, 9));
    assert(!hand_levels_has_planet(-1, 9));
    assert(!hand_levels_has_planet(FLUSH_FIVE + 1, 9));
}

int main()
{
    printf("Testing hand levels.\n");
    test_every_hand_owns_its_level_except_the_royal_flush();
    test_values_that_are_not_hands_own_no_level();
    test_level_ups_stop_at_the_highest_level();
    test_regular_hands_always_have_a_planet();
    test_royal_flush_has_no_planet_of_its_own();
    test_secret_hands_have_a_planet_once_played();
    test_values_that_are_not_hands_have_no_planet();

    printf("-------------------------------------------------------------------------------\n");
    printf("Hand Levels Tests Passed :)\n");
    printf("-------------------------------------------------------------------------------\n");

    return 0;
}
