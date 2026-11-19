/**
 * @file pack_rules.c
 *
 * @brief Rules of the packs sold in the Shop, implementation.
 */
#include "pack_rules.h"

// Frames of the opening of a pack
#define OPENING_FIRST_SHAKE_FRAME 24
#define OPENING_SHAKE_INTERVAL    10
#define OPENING_NB_SHAKES         3
#define OPENING_POP_FRAME         56

bool pack_rules_is_joker_pack_for_sale(uint32_t roll)
{
    return roll % PACK_JOKER_PACK_ODDS == 0;
}

bool pack_rules_can_open_joker_pack(int nb_jokers_held, int nb_joker_slots)
{
    return nb_jokers_held < nb_joker_slots;
}

bool pack_rules_is_new_choice(const int offered_ids[], int nb_offered, int candidate_id)
{
    for (int i = 0; i < nb_offered; i++)
    {
        if (offered_ids[i] == candidate_id)
            return false;
    }

    return true;
}

int pack_rules_get_refund(int price_paid, int nb_offered)
{
    return (nb_offered == 0) ? price_paid : 0;
}

enum PackOpeningStep pack_rules_get_opening_step(int nb_frames)
{
    if (nb_frames >= OPENING_POP_FRAME)
        return PACK_OPENING_POP;

    int nb_frames_shaking = nb_frames - OPENING_FIRST_SHAKE_FRAME;
    if (nb_frames_shaking < 0 || nb_frames_shaking % OPENING_SHAKE_INTERVAL != 0)
        return PACK_OPENING_WAIT;

    bool is_shake_left = (nb_frames_shaking / OPENING_SHAKE_INTERVAL < OPENING_NB_SHAKES);
    return is_shake_left ? PACK_OPENING_SHAKE : PACK_OPENING_WAIT;
}
