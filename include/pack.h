/**
 * @file pack.h
 *
 * @brief Packs: Shop items that open when bought and offer a few items, one of which can be
 *        taken for free.
 *
 * A pack is only the item for sale. The Shop opens it and offers its content, see
 * game/shop_packs.h. There are Joker Packs (ITEM_TYPE_JOKER_PACK) and Planet Packs
 * (ITEM_TYPE_PLANET_PACK).
 */
#ifndef PACK_H
#define PACK_H

#include "item.h"
#include "list.h"

typedef struct PackObject
{
    Item; // First member struct inheritance

    bool in_use;
} PackObject;

/**
 * @brief Tell whether an item is a pack of any kind.
 */
bool item_is_pack(const Item* item);

/**
 * @brief Create a pack of a given type, whatever the odds of finding one in a Shop.
 *
 * @param pack_type ITEM_TYPE_JOKER_PACK or ITEM_TYPE_PLANET_PACK
 *
 * @return the pack, or NULL if the pack of that type already exists
 */
Item* pack_object_new(enum ItemType pack_type);

/**
 * @brief Roll the items a pack offers. An item is never offered twice.
 *
 * @param pack_object the pack
 * @param choices list the items are added to
 *
 * @return the number of items added, 0 if there is nothing left to offer
 */
int pack_object_roll_content(const Item* pack_object, List* choices);

// ItemFuncs implementations, see item.h

Item* joker_pack_object_roll_new(enum RngSequence key);
Item* planet_pack_object_roll_new(enum RngSequence key);
int pack_object_get_buy_price(Item* pack_object);
const char* pack_object_get_name(Item* pack_object);
ItemDescSubtypeInfo pack_object_get_subtype_info(Item* pack_object);

/**
 * @brief Tell whether the pack can be opened. A Joker Pack needs a free Joker slot.
 */
bool pack_object_can_acquire(Item* pack_object);

/**
 * @brief Use up the pack. Offering its content is the job of the Shop.
 */
void pack_object_acquire(Item* pack_object);
void pack_object_dispose(Item** pack_object);
int pack_object_print_description(Item* pack_object, Rect dest_rect);

#endif // PACK_H
