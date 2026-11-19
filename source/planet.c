/**
 * @file planet.c
 *
 * @brief Planet cards implementation.
 */
#include "planet.h"

#include "audio_utils.h"
#include "game.h"
#include "game/common_ui.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "hand_levels.h"
#include "random.h"
#include "soundbank.h"
#include "sprite.h"
#include "util.h"

#include <stdio.h>
#include <tonc.h>

// Drawn in planet_gfx.c
extern const u32 planet_gfxTiles[];
extern const u16 planet_gfxPal[];

// Number of u32 words of one Planet sprite in planet_gfxTiles
#define PLANET_GFX_WORDS (TILE_SIZE * JOKER_SPRITE_TILES)

// Colors and name shown on the description panel
#define PLANET_SUBTYPE_MAIN_COLOR   0x7E88
#define PLANET_SUBTYPE_SHADOW_COLOR 0x5180
#define PLANET_SUBTYPE_NAME         "Planet"

#define PLANET_DESC_MAX_LENGTH 160

typedef struct
{
    const char* name;
    u8 gfx_idx; // Index of the sprite in planet_gfxTiles
} PlanetInfo;

// clang-format off
// Indexed by enum HandType. The Royal Flush has no Planet of its own, it levels up with the
// Straight Flush.
static const PlanetInfo PLANET_INFO[HAND_TYPE_MAX + 1] = {
    [NONE]            = {NULL,       0 },
    [HIGH_CARD]       = {"Pluto",    0 },
    [PAIR]            = {"Mercury",  1 },
    [TWO_PAIR]        = {"Uranus",   2 },
    [THREE_OF_A_KIND] = {"Venus",    3 },
    [STRAIGHT]        = {"Saturn",   4 },
    [FLUSH]           = {"Jupiter",  5 },
    [FULL_HOUSE]      = {"Earth",    6 },
    [FOUR_OF_A_KIND]  = {"Mars",     7 },
    [STRAIGHT_FLUSH]  = {"Neptune",  8 },
    [ROYAL_FLUSH]     = {"Neptune",  8 },
    [FIVE_OF_A_KIND]  = {"Planet X", 9 },
    [FLUSH_HOUSE]     = {"Ceres",    10},
    [FLUSH_FIVE]      = {"Eris",     11},
};
// clang-format on

// WHY: a Planet only exists while it is for sale, in the Shop or in an open Planet Pack.
// They are kept out of the fast memory, which the stack needs.
static EWRAM_BSS PlanetObject s_planet_objects[MAX_PLANETS_FOR_SALE];

static bool is_planet(const Item* item)
{
    return item != NULL && item->type == ITEM_TYPE_PLANET;
}

static int planet_find_free_slot(void)
{
    for (int slot = 0; slot < MAX_PLANETS_FOR_SALE; slot++)
    {
        if (!s_planet_objects[slot].in_use)
            return slot;
    }

    return UNDEFINED;
}

static PlanetObject* planet_object_new(enum HandType hand_type)
{
    int slot = planet_find_free_slot();
    if (slot == UNDEFINED)
        return NULL;

    // Set up the item
    PlanetObject* planet = &s_planet_objects[slot];
    sprite_object_init((SpriteObject*)planet);
    planet->type = ITEM_TYPE_PLANET;
    planet->is_owned = false;
    planet->hand_type = hand_type;
    planet->slot = slot;
    planet->in_use = true;

    // Load its graphics
    int tile_index = sprite_get_tid(PLANET_SPRITE, slot);
    memcpy32(
        &tile_mem[TILE_MEM_OBJ_CHARBLOCK0_IDX][tile_index],
        &planet_gfxTiles[PLANET_INFO[hand_type].gfx_idx * PLANET_GFX_WORDS],
        PLANET_GFX_WORDS
    );
    memcpy16(pal_obj_bank[PLANET_PB], planet_gfxPal, PAL_ROW_LEN);

    sprite_object_set_sprite(
        (SpriteObject*)planet,
        sprite_new(
            ATTR0_SQUARE | ATTR0_4BPP | ATTR0_AFF,
            ATTR1_SIZE_32,
            tile_index,
            PLANET_PB,
            sprite_get_starting_layer(PLANET_SPRITE) + slot
        )
    );

    return planet;
}

static void planet_object_destroy(PlanetObject* planet)
{
    if (planet == NULL || !planet->in_use)
        return;

    sprite_object_destroy((SpriteObject*)planet);
    planet->in_use = false;
}

Item* planet_object_roll_new(enum RngSequence key)
{
    // List the hands a Planet can be offered for
    enum HandType candidates[HAND_TYPE_MAX];
    int nb_candidates = 0;

    for (int hand_type = HAND_TYPE_MIN; hand_type <= HAND_TYPE_MAX; hand_type++)
    {
        if (hand_levels_has_planet(hand_type, g_game_vars.nb_played_hands[hand_type - 1]))
            candidates[nb_candidates++] = (enum HandType)hand_type;
    }

    if (nb_candidates == 0)
        return NULL;

    return (Item*)planet_object_new(candidates[rng_get_u32(key) % nb_candidates]);
}

int planet_object_get_buy_price(Item* planet_object)
{
    GBAL_RETURN_IF_NULL(planet_object, UNDEFINED);

    return PLANET_BUY_PRICE;
}

const char* planet_object_get_name(Item* planet_object)
{
    if (!is_planet(planet_object))
        return ITEM_NAME_UNDEFINED;

    return PLANET_INFO[((PlanetObject*)planet_object)->hand_type].name;
}

ItemDescSubtypeInfo planet_object_get_subtype_info(Item* planet_object)
{
    ItemDescSubtypeInfo subtype_info = ITEM_SUBTYPE_INFO_DEFAULT;
    if (!is_planet(planet_object))
        return subtype_info;

    subtype_info.main_color = PLANET_SUBTYPE_MAIN_COLOR;
    subtype_info.shadow_color = PLANET_SUBTYPE_SHADOW_COLOR;
    subtype_info.name_str = PLANET_SUBTYPE_NAME;

    return subtype_info;
}

bool planet_object_can_acquire(Item* planet_object)
{
    return is_planet(planet_object);
}

void planet_object_acquire(Item* planet_object)
{
    if (!is_planet(planet_object))
        return;

    PlanetObject* planet = (PlanetObject*)planet_object;
    hand_level_up(planet->hand_type);
    hand_show_level_up(planet->hand_type);
    play_sfx(SFX_MULT, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);

    // WHY: a Planet is used as soon as it is bought, it never goes to the inventory
    planet_object_destroy(planet);
}

void planet_object_dispose(Item** planet_object)
{
    GBAL_RETURN_IF_NULL(planet_object, RET_NONE);
    if (!is_planet(*planet_object))
        return;

    planet_object_destroy((PlanetObject*)(*planet_object));
    *planet_object = NULL;
}

int planet_object_print_description(Item* planet_object, Rect dest_rect)
{
    if (!is_planet(planet_object))
        return 0;

    enum HandType hand_type = ((PlanetObject*)planet_object)->hand_type;
    HandBonus level_up_bonus = hand_get_level_up_bonus(hand_type);

    char desc[PLANET_DESC_MAX_LENGTH];
    snprintf(
        desc,
        sizeof(desc),
        TTE_BLACK_TAG "Level up " TTE_DARK_BLUE_TAG "%s " TTE_BLACK_TAG "to lvl " TTE_DARK_BLUE_TAG
                      "%lu " TTE_BLUE_TAG "+%lu " TTE_BLACK_TAG "Chips " TTE_RED_TAG "+%lu "
                      TTE_BLACK_TAG "Mult",
        get_hand_type_name(hand_type),
        hand_get_level(hand_type) + 1,
        level_up_bonus.chips,
        level_up_bonus.mult
    );

    return tte_printf_justified_in_rect(desc, dest_rect, JUSTIFY_CENTER, SCREEN_LEFT, true);
}
