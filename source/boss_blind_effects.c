/**
 * @file boss_blind_effects.c
 *
 * @brief Boss Blind effects implementation.
 */
#include "boss_blind_effects.h"

#include <stdio.h>
#include <string.h>

#define NEEDLE_NB_HANDS       1
#define WATER_NB_DISCARDS     0
#define HOOK_NB_CARDS         2
#define PSYCHIC_NB_CARDS      5
#define TOOTH_MONEY_PER_CARD  1

static bool s_eye_played_types[BOSS_BLIND_MAX_HAND_TYPES];
static int s_ox_hand_type = BOSS_BLIND_LOWEST_HAND_TYPE;

static bool is_hand_type_valid(int hand_type)
{
    return hand_type >= 0 && hand_type < BOSS_BLIND_MAX_HAND_TYPES;
}

bool boss_blind_has_effect(enum BlindType id)
{
    switch (id)
    {
        case BLIND_TYPE_HOOK:
        case BLIND_TYPE_OX:
        case BLIND_TYPE_PSYCHIC:
        case BLIND_TYPE_WATER:
        case BLIND_TYPE_EYE:
        case BLIND_TYPE_NEEDLE:
        case BLIND_TYPE_TOOTH:
            return true;
        default:
            return false;
    }
}

void boss_blind_apply_round_start(enum BlindType id, int* hands, int* discards, int* hand_size)
{
    (void)hand_size;
    switch (id)
    {
        case BLIND_TYPE_NEEDLE:
            *hands = NEEDLE_NB_HANDS;
            break;
        case BLIND_TYPE_WATER:
            *discards = WATER_NB_DISCARDS;
            break;
        default:
            break;
    }
}

bool boss_blind_validate_play(enum BlindType id, int hand_selections, int hand_type)
{
    switch (id)
    {
        case BLIND_TYPE_PSYCHIC:
            return hand_selections == PSYCHIC_NB_CARDS;
        case BLIND_TYPE_EYE:
            return !is_hand_type_valid(hand_type) || !s_eye_played_types[hand_type];
        default:
            return true;
    }
}

int boss_blind_get_hook_count(enum BlindType id)
{
    return (id == BLIND_TYPE_HOOK) ? HOOK_NB_CARDS : 0;
}

int boss_blind_get_tooth_penalty(enum BlindType id, int cards_played)
{
    if (id != BLIND_TYPE_TOOTH || cards_played <= 0)
        return 0;

    return cards_played * TOOTH_MONEY_PER_CARD;
}

bool boss_blind_is_ox_active(enum BlindType id)
{
    return id == BLIND_TYPE_OX;
}

void boss_blind_set_ox_hand(int hand_type)
{
    s_ox_hand_type = hand_type;
}

int boss_blind_get_ox_hand(void)
{
    return s_ox_hand_type;
}

bool boss_blind_is_ox_triggered(enum BlindType id, int hand_type)
{
    return boss_blind_is_ox_active(id) && hand_type == s_ox_hand_type;
}

void boss_blind_register_hand(enum BlindType id, int hand_type)
{
    if (id == BLIND_TYPE_EYE && is_hand_type_valid(hand_type))
        s_eye_played_types[hand_type] = true;
}

void boss_blind_reset(void)
{
    for (int i = 0; i < BOSS_BLIND_MAX_HAND_TYPES; i++)
        s_eye_played_types[i] = false;
}

int boss_blind_find_most_played_hand(const uint32_t nb_played_hands[], int nb_hand_types)
{
    int most_played_idx = 0;

    // WHY: only a strictly higher count replaces the current one, so the lowest hand wins a tie
    for (int idx = 1; idx < nb_hand_types; idx++)
    {
        if (nb_played_hands[idx] > nb_played_hands[most_played_idx])
            most_played_idx = idx;
    }

    return most_played_idx + BOSS_BLIND_LOWEST_HAND_TYPE;
}

// WHY: on-screen text is centered on a grid of whole characters, which is only exact for an even
// number of them. Every text below has an even length, the unit tests check it.
const char* boss_blind_get_effect_text(enum BlindType id)
{
    switch (id)
    {
        case BLIND_TYPE_HOOK:
            return "Discards 2 cards each hand";
        case BLIND_TYPE_PSYCHIC:
            return "You must play five cards";
        case BLIND_TYPE_WATER:
            return "Start with zero discards";
        case BLIND_TYPE_EYE:
            return "No repeat hand types";
        case BLIND_TYPE_NEEDLE:
            return "Play only 1 hand";
        case BLIND_TYPE_TOOTH:
            return "Lose $1 for each card played";
        default:
            return NULL;
    }
}

void boss_blind_format_ox_text(char* out, size_t out_size, const char* hand_name)
{
    // WHY: the wording follows the length of the hand name so the whole text keeps an even
    // number of characters and can be centered exactly
    bool is_name_length_even = (strlen(hand_name) % 2 == 0);
    const char* effect = is_name_length_even ? " sets your money to $0" : " sets money to $0";

    snprintf(out, out_size, "%s%s", hand_name, effect);
}
