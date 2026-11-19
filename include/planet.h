/**
 * @file planet.h
 *
 * @brief Planet cards: Shop items that are used when bought and level up one poker hand.
 */
#ifndef PLANET_H
#define PLANET_H

#include "hand.h"
#include "item.h"

#define PLANET_BUY_PRICE 3

typedef struct PlanetObject
{
    Item; // First member struct inheritance

    /**
     * @brief The hand type the Planet levels up
     */
    enum HandType hand_type;

    /**
     * @brief Which of the Planet sprite slots the Planet is drawn with
     */
    s8 slot;

    bool in_use;
} PlanetObject;

// ItemFuncs implementations, see item.h

Item* planet_object_roll_new(enum RngSequence key);
int planet_object_get_buy_price(Item* planet_object);
const char* planet_object_get_name(Item* planet_object);
ItemDescSubtypeInfo planet_object_get_subtype_info(Item* planet_object);
bool planet_object_can_acquire(Item* planet_object);

/**
 * @brief Use the Planet: its hand gains a level and the Planet is destroyed.
 */
void planet_object_acquire(Item* planet_object);
void planet_object_dispose(Item** planet_object);
int planet_object_print_description(Item* planet_object, Rect dest_rect);

#endif // PLANET_H
