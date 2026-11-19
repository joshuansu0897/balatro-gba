/**
 * @file hand_levels.c
 *
 * @brief Hand level rules implementation.
 */
#include "hand_levels.h"

// A level 1 hand has no level up
#define MAX_LEVEL_UPS (HAND_LEVELS_MAX_LEVEL - 1)

static bool is_hand(int hand_type)
{
    return hand_type >= HAND_LEVELS_LOWEST_HAND && hand_type <= HAND_LEVELS_HIGHEST_HAND;
}

int hand_levels_get_owner(int hand_type)
{
    if (!is_hand(hand_type))
        return HAND_LEVELS_NO_HAND;

    // WHY: a Royal Flush is a Straight Flush by another name, they level up together
    if (hand_type == HAND_LEVELS_ROYAL_FLUSH)
        return HAND_LEVELS_STRAIGHT_FLUSH;

    return hand_type;
}

uint32_t hand_levels_add_level_up(uint32_t nb_level_ups)
{
    if (nb_level_ups >= MAX_LEVEL_UPS)
        return MAX_LEVEL_UPS;

    return nb_level_ups + 1;
}

bool hand_levels_has_planet(int hand_type, uint32_t nb_times_played)
{
    if (!is_hand(hand_type) || hand_type == HAND_LEVELS_ROYAL_FLUSH)
        return false;

    // WHY: the secret hands stay hidden until the player finds them by playing one
    bool is_secret = (hand_type >= HAND_LEVELS_FIRST_SECRET_HAND);

    return !is_secret || nb_times_played > 0;
}

bool hand_levels_is_name_shown(uint32_t nb_frames)
{
    return (nb_frames / HAND_LEVELS_SHOWN_PART_FRAMES) % 2 == 0;
}
