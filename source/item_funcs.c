#include "item_funcs.h"

#include "card.h"
#include "game.h"
#include "joker.h"
#include "mgba_logger.h"
#include "pack.h"
#include "planet.h"
#include "util.h"

static bool item_always_can_acquire(Item* item);

// clang-format off
ItemFuncs item_func_table[] = {
    [ITEM_TYPE_JOKER] = {
        .roll_new = joker_object_roll_new,
        .get_buy_price = joker_object_get_buy_price,
        .get_sell_price = joker_object_get_sell_price,
        .get_name = joker_object_get_name,
        .get_subtype_info = joker_object_get_rarity_info,
        .acquire = joker_object_add_to_owned,
        .can_acquire = joker_object_can_acquire,
        .dispose = joker_object_dispose,
        .print_description = joker_object_print_description
    },
    
    /* Currently playing cards have partial implementations since Magic Trick is not implemented
     * and playing cards can't appear in the shop.
     * If the Magic Trick voucher is implemented, these need to be completed.
     * Playing cards must stay the only exception, the rest of the item types must implement
     * all the basic functions to appear in the shop.
     */
    [ITEM_TYPE_PLAYING_CARD] = {
        .roll_new = NULL,
        .get_buy_price = card_object_get_buy_price,
        .get_sell_price = NULL,
        .get_name = NULL,
        .get_subtype_info = NULL,
        .acquire = NULL,
        .can_acquire = item_always_can_acquire,
        .dispose = card_object_dispose,
        .print_description = NULL
    },

    // Planets are used when bought, so they are never owned and cannot be sold
    [ITEM_TYPE_PLANET] = {
        .roll_new = planet_object_roll_new,
        .get_buy_price = planet_object_get_buy_price,
        .get_sell_price = NULL,
        .get_name = planet_object_get_name,
        .get_subtype_info = planet_object_get_subtype_info,
        .acquire = planet_object_acquire,
        .can_acquire = planet_object_can_acquire,
        .dispose = planet_object_dispose,
        .print_description = planet_object_print_description
    },

    // Packs are used up when bought: the Shop opens them and offers their content
    [ITEM_TYPE_JOKER_PACK] = {
        .roll_new = joker_pack_object_roll_new,
        .get_buy_price = pack_object_get_buy_price,
        .get_sell_price = NULL,
        .get_name = pack_object_get_name,
        .get_subtype_info = pack_object_get_subtype_info,
        .acquire = pack_object_acquire,
        .can_acquire = pack_object_can_acquire,
        .dispose = pack_object_dispose,
        .print_description = pack_object_print_description
    },

    [ITEM_TYPE_PLANET_PACK] = {
        .roll_new = planet_pack_object_roll_new,
        .get_buy_price = pack_object_get_buy_price,
        .get_sell_price = NULL,
        .get_name = pack_object_get_name,
        .get_subtype_info = pack_object_get_subtype_info,
        .acquire = pack_object_acquire,
        .can_acquire = pack_object_can_acquire,
        .dispose = pack_object_dispose,
        .print_description = pack_object_print_description
    }
};
// clang-format on

ItemFuncs* get_item_type_funcs(enum ItemType type)
{
    if ((int)type < 0 || type >= NUM_ELEM_IN_ARR(item_func_table))
    {
        MGBA_FUNC_ERROR("Invalid type %d", type);
        return NULL;
    }

    return &item_func_table[type];
}

static bool item_always_can_acquire(Item* item)
{
    return true;
}
