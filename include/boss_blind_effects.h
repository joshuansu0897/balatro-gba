/**
 * @file boss_blind_effects.h
 *
 * @brief Rules of the Boss Blinds that have an effect.
 *
 * Only arithmetic and bookkeeping live here so they can be unit tested on the host. The active
 * Boss Blind is the current Blind of the game, which callers pass to every function. Hand types
 * are passed as plain numbers, with the values of enum HandType.
 *
 * Boss Blinds with an effect:
 *   The Needle  : Only 1 hand allowed per Round.
 *   The Water   : Start the Round with 0 discards.
 *   The Hook    : After each hand played, 2 held cards are discarded.
 *   The Psychic : A hand must be made of exactly 5 cards to score.
 *   The Tooth   : Lose $1 for each card played.
 *   The Eye     : Each hand type only scores once per Round.
 *   The Ox      : Playing the most played hand of the run sets money to $0.
 */
#ifndef BOSS_BLIND_EFFECTS_H
#define BOSS_BLIND_EFFECTS_H

#include "blind.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Number of values of enum HandType, NONE included.
 */
#define BOSS_BLIND_MAX_HAND_TYPES 14

/**
 * @brief Value of the lowest hand type, HIGH_CARD in enum HandType.
 */
#define BOSS_BLIND_LOWEST_HAND_TYPE 1

/**
 * @brief Size of a buffer that can hold any text describing a Boss Blind effect.
 */
#define BOSS_BLIND_EFFECT_TEXT_SIZE 32

/**
 * @brief Turn the effects of every Boss Blind on or off. They are off until turned on.
 *
 * While they are off, every function of this module answers as for a Blind without effect.
 */
void boss_blind_set_enabled(bool is_enabled);

/**
 * @brief Tell whether the effects of the Boss Blinds are on.
 */
bool boss_blind_is_enabled(void);

/**
 * @brief Tell whether a Blind has one of the effects of this module.
 */
bool boss_blind_has_effect(enum BlindType id);

/**
 * @brief Change the hands, discards and hand size a Round starts with.
 *
 * @param id the Blind of the Round
 * @param hands number of hands of the Round, updated in place
 * @param discards number of discards of the Round, updated in place
 * @param hand_size size of the hand for the Round, updated in place
 */
void boss_blind_apply_round_start(enum BlindType id, int* hands, int* discards, int* hand_size);

/**
 * @brief Tell whether the Blind lets a hand score.
 *
 * @param id the Blind of the Round
 * @param hand_selections number of cards in the hand
 * @param hand_type type of the hand
 *
 * @return false if the hand would be played for nothing, true otherwise
 */
bool boss_blind_validate_play(enum BlindType id, int hand_selections, int hand_type);

/**
 * @brief Get how many held cards the Blind discards after each hand played.
 */
int boss_blind_get_hook_count(enum BlindType id);

/**
 * @brief Get how much money the Blind takes for a hand of @p cards_played cards.
 */
int boss_blind_get_tooth_penalty(enum BlindType id, int cards_played);

/**
 * @brief Tell whether the Blind is the one that takes all the money for a certain hand type.
 */
bool boss_blind_is_ox_active(enum BlindType id);

/**
 * @brief Set the hand type The Ox punishes, the most played one when the Round starts.
 */
void boss_blind_set_ox_hand(int hand_type);

/**
 * @brief Get the hand type The Ox punishes.
 */
int boss_blind_get_ox_hand(void);

/**
 * @brief Tell whether playing a hand of type @p hand_type sets the money to $0.
 */
bool boss_blind_is_ox_triggered(enum BlindType id, int hand_type);

/**
 * @brief Remember that a hand type was played this Round, for The Eye.
 */
void boss_blind_register_hand(enum BlindType id, int hand_type);

/**
 * @brief Forget the hand types played, to call when a Round starts.
 */
void boss_blind_reset(void);

/**
 * @brief Find the hand type played the most times.
 *
 * @param nb_played_hands number of times each hand type was played, starting at the lowest hand
 *                        type (there is no entry for NONE)
 * @param nb_hand_types number of entries in @p nb_played_hands
 *
 * @return the hand type played the most, the lowest one if several are tied or none was played
 */
int boss_blind_find_most_played_hand(const uint32_t nb_played_hands[], int nb_hand_types);

/**
 * @brief Get the name of a Boss Blind that has one of the effects of this module.
 *
 * @return the name, whether the effects are on or off, or NULL for any other Blind
 */
const char* boss_blind_get_name(enum BlindType id);

/**
 * @brief Get a short text describing the effect of a Blind.
 *
 * @return the text, or NULL if the Blind has no effect or if its text depends on the run (The Ox,
 *         see boss_blind_format_ox_text())
 */
const char* boss_blind_get_effect_text(enum BlindType id);

/**
 * @brief Write the text describing the effect of The Ox.
 *
 * @param out where the text is written, BOSS_BLIND_EFFECT_TEXT_SIZE bytes are enough
 * @param out_size size of @p out
 * @param hand_name displayed name of the hand type The Ox punishes
 */
void boss_blind_format_ox_text(char* out, size_t out_size, const char* hand_name);

#endif // BOSS_BLIND_EFFECTS_H
