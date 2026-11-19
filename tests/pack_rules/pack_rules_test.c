#include <assert.h>
#include <pack_rules.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void test_joker_pack_needs_a_free_joker_slot(void)
{
    assert(pack_rules_can_open_joker_pack(0, 5));
    assert(pack_rules_can_open_joker_pack(4, 5));

    assert(!pack_rules_can_open_joker_pack(5, 5));
    assert(!pack_rules_can_open_joker_pack(6, 5));
}

void test_joker_pack_is_for_sale_in_one_shop_out_of_five(void)
{
    int nb_shops_with_joker_pack = 0;
    for (uint32_t roll = 0; roll < 1000; roll++)
    {
        if (pack_rules_is_joker_pack_for_sale(roll))
            nb_shops_with_joker_pack++;
    }

    assert(nb_shops_with_joker_pack == 200);
}

void test_first_choice_of_a_pack_is_always_new(void)
{
    assert(pack_rules_is_new_choice(NULL, 0, 7));
}

void test_choice_already_offered_is_not_new(void)
{
    const int offered_ids[] = {4, 9};

    assert(!pack_rules_is_new_choice(offered_ids, 2, 4));
    assert(!pack_rules_is_new_choice(offered_ids, 2, 9));
    assert(pack_rules_is_new_choice(offered_ids, 2, 5));

    // Only the choices offered so far count
    assert(pack_rules_is_new_choice(offered_ids, 1, 9));
}

void test_pack_with_nothing_to_offer_is_refunded(void)
{
    assert(pack_rules_get_refund(PACK_BUY_PRICE, 0) == PACK_BUY_PRICE);

    assert(pack_rules_get_refund(PACK_BUY_PRICE, 1) == 0);
    assert(pack_rules_get_refund(PACK_BUY_PRICE, PACK_NB_CHOICES) == 0);
}

void test_pack_shakes_three_times_then_pops(void)
{
    int nb_shakes = 0;
    int pop_frame = -1;

    for (int frame = 0; frame < 200 && pop_frame == -1; frame++)
    {
        enum PackOpeningStep step = pack_rules_get_opening_step(frame);
        if (step == PACK_OPENING_SHAKE)
            nb_shakes++;
        if (step == PACK_OPENING_POP)
            pop_frame = frame;
    }

    assert(nb_shakes == 3);
    assert(pop_frame > 0);
}

void test_pack_waits_before_its_first_shake(void)
{
    assert(pack_rules_get_opening_step(0) == PACK_OPENING_WAIT);
    assert(pack_rules_get_opening_step(1) == PACK_OPENING_WAIT);
}

void test_pack_stays_popped_once_it_popped(void)
{
    int pop_frame = 0;
    while (pack_rules_get_opening_step(pop_frame) != PACK_OPENING_POP)
        pop_frame++;

    assert(pack_rules_get_opening_step(pop_frame + 1) == PACK_OPENING_POP);
    assert(pack_rules_get_opening_step(pop_frame + 100) == PACK_OPENING_POP);
}

int main(void)
{
    printf("Testing pack rules.\n");
    test_joker_pack_needs_a_free_joker_slot();
    test_joker_pack_is_for_sale_in_one_shop_out_of_five();
    test_first_choice_of_a_pack_is_always_new();
    test_choice_already_offered_is_not_new();
    test_pack_with_nothing_to_offer_is_refunded();
    test_pack_shakes_three_times_then_pops();
    test_pack_waits_before_its_first_shake();
    test_pack_stays_popped_once_it_popped();

    printf("-------------------------------------------------------------------------------\n");
    printf("Pack Rules Tests Passed :)\n");
    printf("-------------------------------------------------------------------------------\n");

    return 0;
}
