/**
 * @file shop_packs.c
 *
 * @brief The packs of the Shop, implementation.
 */
#include "game/shop_packs.h"

#include "audio_utils.h"
#include "game.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "list.h"
#include "pack.h"
#include "pack_rules.h"
#include "random.h"
#include "soundbank.h"
#include "sprite.h"
#include "util.h"

#include <tonc.h>

// Positions in pixels. The Planet Pack is sold in the left panel of the bottom row, the Joker
// Pack in the right one.
#define PACK_FOR_SALE_Y     115
#define PACK_OFFSCREEN_Y    168
#define PLANET_PACK_X       83
#define JOKER_PACK_X        137
#define PLANET_PACK_PRICE_X 112
#define JOKER_PACK_PRICE_X  168
#define PACK_PRICE_Y        128
// Where a pack opens: in the middle of the items for sale
#define PACK_OPENING_X      136
#define PACK_OPENING_Y      71

// WHY: raised as much as the items for sale above it, a selected pack would cover their prices
#define PACK_FOCUS_RAISE_PX 4

// Covers the prices of the packs and the text shown while a pack is open
static const Rect PACK_TEXT_RECT = {112, 120, 192, 136};

// How to use the open pack, on two lines in the right panel of the bottom row
#define PACK_HINT_TEXT TTE_WHITE_TAG "#{P:136,120}Pick 1#{P:136,128}or Next"

typedef struct
{
    List for_sale;
    // The items of the Shop, put aside while a pack is open
    List shop_items_aside;
    SpriteContainer* shop_items;
    // The pack that was bought, until it pops
    Item* opening_pack;
    // The pack the cursor is on, its price stands out
    const Item* focused_pack;
    int price_paid;
    bool is_open;
} ShopPacks;

static EWRAM_BSS ShopPacks s_packs;

static void shop_packs_dispose_list(List* items)
{
    ListItr itr = list_itr_create(items);
    Item* item;
    while ((item = list_itr_next(&itr)))
        item_dispose(&item);

    list_clear(items);
}

static void shop_packs_put_for_sale(enum ItemType pack_type, int x)
{
    Item* pack = item_roll_new(pack_type, RNG_SEQ_SHOP_ITEMS);
    if (pack == NULL)
        return;

    // Come up from under the screen
    pack->x = int2fx(x);
    pack->y = int2fx(PACK_OFFSCREEN_Y);
    pack->tx = pack->x;
    pack->ty = int2fx(PACK_FOR_SALE_Y);
    list_push_back(&s_packs.for_sale, pack);
}

void shop_packs_create(SpriteContainer* shop_items)
{
    s_packs.shop_items = shop_items;

    shop_packs_put_for_sale(ITEM_TYPE_PLANET_PACK, PLANET_PACK_X);
    shop_packs_put_for_sale(ITEM_TYPE_JOKER_PACK, JOKER_PACK_X);
}

void shop_packs_dispose(void)
{
    shop_packs_dispose_list(&s_packs.for_sale);
    shop_packs_dispose_list(&s_packs.shop_items_aside);
    if (s_packs.opening_pack != NULL)
        item_dispose(&s_packs.opening_pack);

    s_packs.focused_pack = NULL;
    s_packs.is_open = false;
}

int shop_packs_get_nb_for_sale(void)
{
    return s_packs.is_open ? 0 : list_get_len(&s_packs.for_sale);
}

Item* shop_packs_get_for_sale(int pack_idx)
{
    if (pack_idx < 0 || pack_idx >= shop_packs_get_nb_for_sale())
        return NULL;

    return (Item*)list_get_at_idx(&s_packs.for_sale, pack_idx);
}

/**
 * @brief Select or deselect a pack: it rises a little and its price stands out.
 */
static void shop_packs_focus(Item* pack, bool is_focused)
{
    if (pack == NULL || pack->focused == is_focused)
        return;

    // Raise it less than the other items are
    int lowered_px = SPRITE_FOCUS_RAISE_PX - PACK_FOCUS_RAISE_PX;
    sprite_object_set_focus((SpriteObject*)pack, is_focused);
    pack->ty += int2fx(is_focused ? lowered_px : -lowered_px);

    s_packs.focused_pack = is_focused ? pack : NULL;
    shop_packs_print_text();
}

void shop_packs_set_focus(int pack_idx, bool is_focused)
{
    shop_packs_focus(shop_packs_get_for_sale(pack_idx), is_focused);
}

void shop_packs_set_shown(bool is_shown, const Item* except)
{
    bool is_on_screen = is_shown && !s_packs.is_open;
    FIXED target_y = int2fx(is_on_screen ? PACK_FOR_SALE_Y : PACK_OFFSCREEN_Y);

    ListItr itr = list_itr_create(&s_packs.for_sale);
    Item* pack;
    while ((pack = list_itr_next(&itr)))
    {
        if (pack != except)
            pack->ty = target_y;
    }
}

/**
 * @brief Print how to use the open pack: take an item, or pass with the "Next Round" button.
 */
static void shop_packs_print_hint(void)
{
    // WHY: there is nothing to pick until the pack has popped
    if (s_packs.opening_pack == NULL)
        tte_printf(PACK_HINT_TEXT);
}

void shop_packs_print_text(void)
{
    tte_erase_rect_wrapper(PACK_TEXT_RECT);

    if (s_packs.is_open)
    {
        shop_packs_print_hint();
        return;
    }

    ListItr itr = list_itr_create(&s_packs.for_sale);
    Item* pack;
    while ((pack = list_itr_next(&itr)))
    {
        bool is_joker_pack = (pack->type == ITEM_TYPE_JOKER_PACK);
        int price_x = is_joker_pack ? JOKER_PACK_PRICE_X : PLANET_PACK_PRICE_X;
        const char* color = (pack == s_packs.focused_pack) ? TTE_YELLOW_TAG : TTE_WHITE_TAG;

        tte_printf("%s#{P:%d,%d}$%d", color, price_x, PACK_PRICE_Y, item_get_buy_price(pack));
    }
}

bool shop_packs_is_open(void)
{
    return s_packs.is_open;
}

static void shop_packs_put_shop_items_aside(void)
{
    ListItr itr = list_itr_create(&s_packs.shop_items->contents_list);
    Item* item;

    while ((item = list_itr_next(&itr)))
    {
        sprite_object_erase_text_under((SpriteObject*)item);
        item->ty = int2fx(PACK_OFFSCREEN_Y);
        list_push_back(&s_packs.shop_items_aside, item);
    }

    container_clear_contents(s_packs.shop_items);
}

bool shop_packs_buy(int pack_idx)
{
    Item* pack = shop_packs_get_for_sale(pack_idx);
    if (pack == NULL)
        return false;

    int price = item_get_buy_price(pack);
    if (!item_can_acquire(pack) || g_game_vars.money < price)
        return false;

    // Pay
    g_game_vars.money -= price;
    play_sfx(SFX_COIN1_MONEY_CHNG, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);
    display_money();

    // Take the pack off its shelf
    shop_packs_focus(pack, false);
    list_remove_at_idx(&s_packs.for_sale, pack_idx);
    s_packs.opening_pack = pack;
    s_packs.price_paid = price;
    s_packs.is_open = true;

    // Make room for it in the middle of the Shop
    shop_packs_put_shop_items_aside();
    shop_packs_set_shown(false, NULL);
    shop_packs_print_text();
    pack->tx = int2fx(PACK_OPENING_X);
    pack->ty = int2fx(PACK_OPENING_Y);
    pal_bg_mem[SHOP_PANEL_BORDER_PAL_IDX] = item_get_subtype_info(pack).main_color;

    return true;
}

/**
 * @brief Put the content of the pack that popped where the Shop items were.
 */
static void shop_packs_offer(List* choices)
{
    container_append_list(s_packs.shop_items, choices);

    // Make them come out of the pack
    ListItr itr = list_itr_create(&s_packs.shop_items->contents_list);
    Item* choice;
    while ((choice = list_itr_next(&itr)))
    {
        choice->x = int2fx(PACK_OPENING_X);
        choice->y = int2fx(PACK_OPENING_Y);
    }
}

enum ShopPackOpening shop_packs_update_opening(int nb_frames)
{
    Item* pack = s_packs.opening_pack;
    GBAL_RETURN_IF_NULL(pack, SHOP_PACK_EMPTY);

    enum PackOpeningStep step = pack_rules_get_opening_step(nb_frames);
    if (step == PACK_OPENING_SHAKE)
        sprite_object_shake((SpriteObject*)pack, SFX_CARD_FOCUS);
    if (step != PACK_OPENING_POP)
        return SHOP_PACK_OPENING;

    // Pop the pack
    List choices = LIST_DEFAULT;
    int nb_offered = pack_object_roll_content(pack, &choices);
    item_acquire(pack);
    s_packs.opening_pack = NULL;
    play_sfx(SFX_POP, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);

    // Offer its content, or give the money back
    shop_packs_offer(&choices);
    int refund = pack_rules_get_refund(s_packs.price_paid, nb_offered);
    if (refund == 0)
        return SHOP_PACK_OPENED;

    g_game_vars.money += refund;
    display_money();
    shop_packs_close();

    return SHOP_PACK_EMPTY;
}

void shop_packs_close(void)
{
    s_packs.is_open = false;

    // Throw away what was not taken
    shop_packs_dispose_list(&s_packs.shop_items->contents_list);

    // Bring the Shop back
    container_append_list(s_packs.shop_items, &s_packs.shop_items_aside);
    s_packs.shop_items_aside = list_init();
    shop_packs_set_shown(true, NULL);
    shop_packs_print_text();
    pal_bg_mem[SHOP_PANEL_BORDER_PAL_IDX] = SHOP_PANEL_BORDER_COLOR;
}
