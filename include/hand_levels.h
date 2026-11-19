/**
 * @file hand_levels.h
 *
 * @brief Rules of the hand levels raised by Planet cards.
 *
 * Only arithmetic lives here so it can be unit tested on the host. Hand types are passed as plain
 * numbers, with the values of enum HandType.
 */
#ifndef HAND_LEVELS_H
#define HAND_LEVELS_H

#include <stdbool.h>
#include <stdint.h>

// Values of enum HandType this module needs to know
#define HAND_LEVELS_NO_HAND           0
#define HAND_LEVELS_LOWEST_HAND       1
#define HAND_LEVELS_STRAIGHT_FLUSH    9
#define HAND_LEVELS_ROYAL_FLUSH       10
#define HAND_LEVELS_FIRST_SECRET_HAND 11
#define HAND_LEVELS_HIGHEST_HAND      13

/**
 * @brief Highest level a hand can reach.
 */
#define HAND_LEVELS_MAX_LEVEL 99

/**
 * @brief Get the hand type whose level a hand type uses.
 *
 * @return @p hand_type itself, except for the Royal Flush which levels with the Straight Flush.
 *         HAND_LEVELS_NO_HAND if @p hand_type is not a hand.
 */
int hand_levels_get_owner(int hand_type);

/**
 * @brief Add one level up to a number of level ups, without going past the highest level.
 *
 * @param nb_level_ups number of times the hand was leveled up so far, 0 for a level 1 hand
 *
 * @return the new number of level ups
 */
uint32_t hand_levels_add_level_up(uint32_t nb_level_ups);

/**
 * @brief Tell whether a Planet card can be offered for a hand type.
 *
 * @param hand_type the hand type the Planet card would level up
 * @param nb_times_played how many times the hand type was played during the run
 *
 * @return false for the Royal Flush, which has no Planet card of its own, and for the secret hands
 *         that were never played. true otherwise.
 */
bool hand_levels_has_planet(int hand_type, uint32_t nb_times_played);

/**
 * @brief Number of frames each part of a level up stays on screen.
 */
#define HAND_LEVELS_SHOWN_PART_FRAMES 45

/**
 * @brief Tell which part of a level up is on screen.
 *
 * The name of the hand and its level do not fit together where they are shown, so they take
 * turns.
 *
 * @param nb_frames number of frames since the level up started to be shown
 *
 * @return true for the name of the hand, which comes first, false for its level
 */
bool hand_levels_is_name_shown(uint32_t nb_frames);

#endif // HAND_LEVELS_H
