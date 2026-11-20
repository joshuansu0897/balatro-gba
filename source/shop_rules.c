/**
 * @file shop_rules.c
 *
 * @brief Switches for what the Shop sells, implementation.
 */
#include "shop_rules.h"

// WHY: kept as "off" so that the memory of a game that just started, all zeros, means that
// everything is sold
static bool s_is_off[SHOP_RULE_MAX];

static bool is_rule(enum ShopRule rule)
{
    return (int)rule >= 0 && rule < SHOP_RULE_MAX;
}

bool shop_rules_is_on(enum ShopRule rule)
{
    return is_rule(rule) && !s_is_off[rule];
}

void shop_rules_set(enum ShopRule rule, bool is_on)
{
    if (!is_rule(rule))
        return;

    s_is_off[rule] = !is_on;

    // WHY: a Planet Pack is a way to get Planet cards. Selling it while they are switched off
    // would bring them back through the side door, so the pack goes with the cards.
    if (rule == SHOP_RULE_PLANET_PACK && is_on)
        s_is_off[SHOP_RULE_PLANET_CARDS] = false;
    if (rule == SHOP_RULE_PLANET_CARDS && !is_on)
        s_is_off[SHOP_RULE_PLANET_PACK] = true;
}

uint8_t shop_rules_get_saved(enum ShopRule rule)
{
    return shop_rules_is_on(rule) ? SHOP_RULE_SAVED_ON : SHOP_RULE_SAVED_OFF;
}

void shop_rules_set_saved(enum ShopRule rule, uint8_t saved_value)
{
    shop_rules_set(rule, saved_value != SHOP_RULE_SAVED_OFF);
}
