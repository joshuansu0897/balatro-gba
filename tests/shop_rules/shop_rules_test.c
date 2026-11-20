#include <assert.h>
#include <shop_rules.h>
#include <stdio.h>

// What a save made before the switches existed holds in their place
#define SAVED_BEFORE_THE_RULES_EXISTED 0xFF

static void switch_everything_on(void)
{
    for (int rule = 0; rule < SHOP_RULE_MAX; rule++)
        shop_rules_set(rule, true);
}

void test_everything_is_sold_until_switched_off(void)
{
    // Nothing was set yet: this is the state of a game that just started
    for (int rule = 0; rule < SHOP_RULE_MAX; rule++)
        assert(shop_rules_is_on(rule));
}

void test_rule_can_be_switched_off_without_touching_the_others(void)
{
    switch_everything_on();

    shop_rules_set(SHOP_RULE_JOKER_PACK, false);

    assert(!shop_rules_is_on(SHOP_RULE_JOKER_PACK));
    assert(shop_rules_is_on(SHOP_RULE_PLANET_CARDS));
    assert(shop_rules_is_on(SHOP_RULE_PLANET_PACK));
}

void test_rule_can_be_switched_back_on(void)
{
    switch_everything_on();

    shop_rules_set(SHOP_RULE_PLANET_CARDS, false);
    shop_rules_set(SHOP_RULE_PLANET_CARDS, true);

    assert(shop_rules_is_on(SHOP_RULE_PLANET_CARDS));
}

void test_switching_the_planet_pack_on_switches_planet_cards_on(void)
{
    switch_everything_on();
    shop_rules_set(SHOP_RULE_PLANET_CARDS, false);

    shop_rules_set(SHOP_RULE_PLANET_PACK, true);

    assert(shop_rules_is_on(SHOP_RULE_PLANET_PACK));
    assert(shop_rules_is_on(SHOP_RULE_PLANET_CARDS));
}

void test_switching_planet_cards_off_switches_the_planet_pack_off(void)
{
    switch_everything_on();

    shop_rules_set(SHOP_RULE_PLANET_CARDS, false);

    assert(!shop_rules_is_on(SHOP_RULE_PLANET_CARDS));
    assert(!shop_rules_is_on(SHOP_RULE_PLANET_PACK));
}

void test_planet_cards_can_be_sold_without_the_planet_pack(void)
{
    switch_everything_on();

    shop_rules_set(SHOP_RULE_PLANET_PACK, false);

    assert(shop_rules_is_on(SHOP_RULE_PLANET_CARDS));
    assert(!shop_rules_is_on(SHOP_RULE_PLANET_PACK));
}

void test_joker_pack_does_not_depend_on_the_planets(void)
{
    switch_everything_on();

    shop_rules_set(SHOP_RULE_PLANET_CARDS, false);

    assert(shop_rules_is_on(SHOP_RULE_JOKER_PACK));
}

void test_save_with_the_pack_on_and_the_cards_off_loads_with_both_on(void)
{
    // WHY: such a save could be made before the pack needed the cards
    for (int rule = 0; rule < SHOP_RULE_MAX; rule++)
    {
        uint8_t saved = (rule == SHOP_RULE_PLANET_CARDS) ? SHOP_RULE_SAVED_OFF : SHOP_RULE_SAVED_ON;
        shop_rules_set_saved(rule, saved);
    }

    assert(shop_rules_is_on(SHOP_RULE_PLANET_PACK));
    assert(shop_rules_is_on(SHOP_RULE_PLANET_CARDS));
}

void test_saved_value_brings_a_switch_back(void)
{
    switch_everything_on();
    shop_rules_set(SHOP_RULE_PLANET_PACK, false);
    uint8_t saved_off = shop_rules_get_saved(SHOP_RULE_PLANET_PACK);
    uint8_t saved_on = shop_rules_get_saved(SHOP_RULE_JOKER_PACK);

    // Load them the other way around
    shop_rules_set_saved(SHOP_RULE_PLANET_PACK, saved_on);
    shop_rules_set_saved(SHOP_RULE_JOKER_PACK, saved_off);

    assert(saved_off == SHOP_RULE_SAVED_OFF);
    assert(saved_on == SHOP_RULE_SAVED_ON);
    assert(shop_rules_is_on(SHOP_RULE_PLANET_PACK));
    assert(!shop_rules_is_on(SHOP_RULE_JOKER_PACK));
}

void test_save_made_before_the_rules_existed_sells_everything(void)
{
    for (int rule = 0; rule < SHOP_RULE_MAX; rule++)
    {
        shop_rules_set(rule, false);
        shop_rules_set_saved(rule, SAVED_BEFORE_THE_RULES_EXISTED);

        assert(shop_rules_is_on(rule));
    }
}

void test_value_that_is_not_a_rule_is_off_and_ignored(void)
{
    switch_everything_on();

    shop_rules_set(SHOP_RULE_MAX, false);
    shop_rules_set(-1, false);
    shop_rules_set_saved(SHOP_RULE_MAX, SHOP_RULE_SAVED_OFF);

    assert(!shop_rules_is_on(SHOP_RULE_MAX));
    assert(!shop_rules_is_on(-1));
    assert(shop_rules_get_saved(SHOP_RULE_MAX) == SHOP_RULE_SAVED_OFF);
    for (int rule = 0; rule < SHOP_RULE_MAX; rule++)
        assert(shop_rules_is_on(rule));
}

int main(void)
{
    printf("Testing shop rules.\n");
    test_everything_is_sold_until_switched_off();
    test_rule_can_be_switched_off_without_touching_the_others();
    test_rule_can_be_switched_back_on();
    test_switching_the_planet_pack_on_switches_planet_cards_on();
    test_switching_planet_cards_off_switches_the_planet_pack_off();
    test_planet_cards_can_be_sold_without_the_planet_pack();
    test_joker_pack_does_not_depend_on_the_planets();
    test_save_with_the_pack_on_and_the_cards_off_loads_with_both_on();
    test_saved_value_brings_a_switch_back();
    test_save_made_before_the_rules_existed_sells_everything();
    test_value_that_is_not_a_rule_is_off_and_ignored();

    printf("-------------------------------------------------------------------------------\n");
    printf("Shop Rules Tests Passed :)\n");
    printf("-------------------------------------------------------------------------------\n");

    return 0;
}
