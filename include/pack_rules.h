/**
 * @file pack_rules.h
 *
 * @brief Rules of the packs sold in the Shop.
 *
 * A pack is bought, opens, and offers a few items of which one can be taken for free. Only
 * arithmetic lives here so it can be unit tested on the host. Items are passed as plain numbers.
 */
#ifndef PACK_RULES_H
#define PACK_RULES_H

#include <stdbool.h>
#include <stdint.h>

#define PACK_BUY_PRICE  4
#define PACK_NB_CHOICES 2

/**
 * @brief One Shop out of this many sells a Joker Pack.
 */
#define PACK_JOKER_PACK_ODDS 5

/**
 * @brief How many times an item the pack already offers is rolled again before it is given up.
 */
#define PACK_MAX_ROLL_TRIES 8

/**
 * @brief What a pack does on a frame of its opening.
 */
enum PackOpeningStep
{
    PACK_OPENING_WAIT,
    PACK_OPENING_SHAKE,
    PACK_OPENING_POP
};

/**
 * @brief Tell whether a Shop sells a Joker Pack.
 *
 * A Joker Pack is a rare find: one in every Shop made Jokers too cheap.
 *
 * @param roll a random number rolled for the Shop
 */
bool pack_rules_is_joker_pack_for_sale(uint32_t roll);

/**
 * @brief Tell whether a Joker Pack can be opened.
 *
 * @return false when there is no free slot for the Joker it would give
 */
bool pack_rules_can_open_joker_pack(int nb_jokers_held, int nb_joker_slots);

/**
 * @brief Tell whether an item is different from every item a pack already offers.
 *
 * @param offered_ids the items offered so far, may be NULL when there is none
 * @param nb_offered number of entries in @p offered_ids
 * @param candidate_id the item rolled as the next choice
 */
bool pack_rules_is_new_choice(const int offered_ids[], int nb_offered, int candidate_id);

/**
 * @brief Get the money given back for a pack once it is known what it offers.
 *
 * @return @p price_paid if the pack had nothing to offer, 0 otherwise
 */
int pack_rules_get_refund(int price_paid, int nb_offered);

/**
 * @brief Get what a pack does on a frame of its opening: it shakes a few times, then pops.
 *
 * @param nb_frames number of frames since the pack started to open
 *
 * @return PACK_OPENING_POP from the frame the pack pops on
 */
enum PackOpeningStep pack_rules_get_opening_step(int nb_frames);

#endif // PACK_RULES_H
