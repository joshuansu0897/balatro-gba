/**
 * @file shop_rules.h
 *
 * @brief Switches for what the Shop sells, set on the Rules page of the Options menu.
 *
 * Everything is sold until it is switched off. Only bookkeeping lives here so it can be unit
 * tested on the host.
 */
#ifndef SHOP_RULES_H
#define SHOP_RULES_H

#include <stdbool.h>
#include <stdint.h>

enum ShopRule
{
    /** Planet cards among the items for sale */
    SHOP_RULE_PLANET_CARDS,
    SHOP_RULE_PLANET_PACK,
    SHOP_RULE_JOKER_PACK,
    SHOP_RULE_MAX
};

/**
 * @brief Values a switch is saved as. Anything else is read as on, see shop_rules_set_saved().
 */
#define SHOP_RULE_SAVED_OFF 0
#define SHOP_RULE_SAVED_ON  1

/**
 * @brief Tell whether the Shop sells what a rule stands for.
 *
 * @return false if it is switched off, or if @p rule is not a rule
 */
bool shop_rules_is_on(enum ShopRule rule);

/**
 * @brief Switch a rule on or off.
 *
 * The Planet Pack needs the Planet cards: switching the pack on switches the cards on, and
 * switching the cards off switches the pack off.
 */
void shop_rules_set(enum ShopRule rule, bool is_on);

/**
 * @brief Get the value to save a switch as.
 */
uint8_t shop_rules_get_saved(enum ShopRule rule);

/**
 * @brief Set a switch from its saved value.
 *
 * WHY: a save made before the switches existed holds neither value. It is read as on, so that
 * such a save keeps selling everything.
 */
void shop_rules_set_saved(enum ShopRule rule, uint8_t saved_value);

#endif // SHOP_RULES_H
