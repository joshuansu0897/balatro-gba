/**
 * @file pack.c
 *
 * @brief Packs implementation.
 */
#include "pack.h"

#include "game.h"
#include "game/common_ui.h"
#include "graphic_utils.h"
#include "joker.h"
#include "list.h"
#include "pack_rules.h"
#include "planet.h"
#include "random.h"
#include "sprite.h"
#include "util.h"

#include <tonc.h>

// Drawn in pack_gfx.c with the colors of the playing cards
extern const u32 pack_gfxTiles[];
extern const u32 planet_pack_gfxTiles[];

// Number of u32 words of one pack sprite
#define PACK_GFX_WORDS (TILE_SIZE * JOKER_SPRITE_TILES)

#define PACK_SUBTYPE_NAME "Pack"

typedef struct
{
    enum ItemType content_type;
    const u32* tiles;
    const char* name;
    const char* description;
    u16 main_color;
    u16 shadow_color;
} PackInfo;

// WHY: one pack of each kind is for sale at a time, so each kind has its own object and sprite
enum PackSlot
{
    PACK_SLOT_JOKER,
    PACK_SLOT_PLANET,
    PACK_NB_SLOTS
};

// clang-format off
static const PackInfo PACK_INFO[PACK_NB_SLOTS] = {
    [PACK_SLOT_JOKER] = {
        .content_type = ITEM_TYPE_JOKER,
        .tiles = pack_gfxTiles,
        .name = "Joker Pack",
        .description = TTE_BLACK_TAG "Choose " TTE_DARK_BLUE_TAG "1 " TTE_BLACK_TAG "of "
                       TTE_DARK_BLUE_TAG "2 " TTE_BLACK_TAG "Jokers for free",
        .main_color = 0x029F,
        .shadow_color = 0x01B6
    },
    [PACK_SLOT_PLANET] = {
        .content_type = ITEM_TYPE_PLANET,
        .tiles = planet_pack_gfxTiles,
        .name = "Planet Pack",
        .description = TTE_BLACK_TAG "Choose " TTE_DARK_BLUE_TAG "1 " TTE_BLACK_TAG "of "
                       TTE_DARK_BLUE_TAG "2 " TTE_BLACK_TAG "Planets for free",
        .main_color = 0x7E88,
        .shadow_color = 0x5180
    }
};
// clang-format on

_Static_assert(PACK_NB_CHOICES == 2, "The pack descriptions say 2 choices");
_Static_assert(PACK_NB_SLOTS == MAX_PACKS_FOR_SALE, "Each pack for sale needs a sprite slot");

static EWRAM_BSS PackObject s_pack_objects[PACK_NB_SLOTS];

bool item_is_pack(const Item* item)
{
    return item != NULL &&
           (item->type == ITEM_TYPE_JOKER_PACK || item->type == ITEM_TYPE_PLANET_PACK);
}

static enum PackSlot pack_get_slot(enum ItemType pack_type)
{
    return (pack_type == ITEM_TYPE_PLANET_PACK) ? PACK_SLOT_PLANET : PACK_SLOT_JOKER;
}

static const PackInfo* pack_get_info(const Item* pack_object)
{
    return &PACK_INFO[pack_get_slot(pack_object->type)];
}

Item* pack_object_new(enum ItemType pack_type)
{
    enum PackSlot slot = pack_get_slot(pack_type);
    PackObject* pack = &s_pack_objects[slot];
    if (pack->in_use)
        return NULL;

    // Set up the item
    sprite_object_init((SpriteObject*)pack);
    pack->type = pack_type;
    pack->is_owned = false;
    pack->in_use = true;

    // Load its graphics
    int tile_index = sprite_get_tid(PACK_SPRITE, slot);
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        PACK_INFO[slot].tiles,
        PACK_GFX_WORDS
    );

    sprite_object_set_sprite(
        (SpriteObject*)pack,
        sprite_new(
            ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
            ATTR1_SIZE_32,
            tile_index,
            CARD_PB,
            sprite_get_starting_layer(PACK_SPRITE) + slot
        )
    );

    return (Item*)pack;
}

static void pack_object_destroy(PackObject* pack)
{
    if (!pack->in_use)
        return;

    sprite_object_destroy((SpriteObject*)pack);
    pack->in_use = false;
}

/**
 * @brief Get a number that tells two items of the same type apart.
 */
static int pack_get_choice_id(Item* choice)
{
    if (choice->type == ITEM_TYPE_PLANET)
        return ((PlanetObject*)choice)->hand_type;

    return ((JokerObject*)choice)->joker->id;
}

/**
 * @brief Roll an item the pack does not offer yet.
 *
 * @return the item, or NULL if there is none left to offer
 */
static Item* pack_roll_choice(enum ItemType type, const int offered_ids[], int nb_offered)
{
    for (int nb_tries = 0; nb_tries < PACK_MAX_ROLL_TRIES; nb_tries++)
    {
        Item* choice = item_roll_new(type, RNG_SEQ_SHOP_ITEMS);
        if (choice == NULL)
            return NULL;

        if (pack_rules_is_new_choice(offered_ids, nb_offered, pack_get_choice_id(choice)))
            return choice;

        item_dispose(&choice);
    }

    return NULL;
}

int pack_object_roll_content(const Item* pack_object, List* choices)
{
    if (!item_is_pack(pack_object))
        return 0;

    enum ItemType content_type = pack_get_info(pack_object)->content_type;
    int offered_ids[PACK_NB_CHOICES];
    int nb_offered = 0;

    while (nb_offered < PACK_NB_CHOICES)
    {
        Item* choice = pack_roll_choice(content_type, offered_ids, nb_offered);
        if (choice == NULL)
            break;

        offered_ids[nb_offered++] = pack_get_choice_id(choice);
        list_push_back(choices, choice);
    }

    return nb_offered;
}

Item* joker_pack_object_roll_new(enum RngSequence key)
{
    // WHY: with a Joker Pack in every Shop, Jokers were too cheap. It is a rare find instead.
    if (!pack_rules_is_joker_pack_for_sale(rng_get_u32(key)))
        return NULL;

    return pack_object_new(ITEM_TYPE_JOKER_PACK);
}

Item* planet_pack_object_roll_new(enum RngSequence key)
{
    return pack_object_new(ITEM_TYPE_PLANET_PACK);
}

int pack_object_get_buy_price(Item* pack_object)
{
    GBAL_RETURN_IF_NULL(pack_object, UNDEFINED);

    return PACK_BUY_PRICE;
}

const char* pack_object_get_name(Item* pack_object)
{
    if (!item_is_pack(pack_object))
        return ITEM_NAME_UNDEFINED;

    return pack_get_info(pack_object)->name;
}

ItemDescSubtypeInfo pack_object_get_subtype_info(Item* pack_object)
{
    ItemDescSubtypeInfo subtype_info = ITEM_SUBTYPE_INFO_DEFAULT;
    if (!item_is_pack(pack_object))
        return subtype_info;

    subtype_info.main_color = pack_get_info(pack_object)->main_color;
    subtype_info.shadow_color = pack_get_info(pack_object)->shadow_color;
    subtype_info.name_str = PACK_SUBTYPE_NAME;

    return subtype_info;
}

bool pack_object_can_acquire(Item* pack_object)
{
    if (!item_is_pack(pack_object))
        return false;

    if (pack_object->type == ITEM_TYPE_PLANET_PACK)
        return true;

    return pack_rules_can_open_joker_pack(list_get_len(get_jokers_list()), game_get_joker_slots());
}

void pack_object_acquire(Item* pack_object)
{
    if (!item_is_pack(pack_object))
        return;

    pack_object_destroy((PackObject*)pack_object);
}

void pack_object_dispose(Item** pack_object)
{
    GBAL_RETURN_IF_NULL(pack_object, RET_NONE);
    if (!item_is_pack(*pack_object))
        return;

    pack_object_destroy((PackObject*)(*pack_object));
    *pack_object = NULL;
}

int pack_object_print_description(Item* pack_object, Rect dest_rect)
{
    if (!item_is_pack(pack_object))
        return 0;

    return tte_printf_justified_in_rect(
        pack_get_info(pack_object)->description,
        dest_rect,
        JUSTIFY_CENTER,
        SCREEN_LEFT,
        true
    );
}
