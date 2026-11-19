/**
 * @file shop_packs.h
 *
 * @brief The packs of the Shop: the bottom row they are sold in, and what happens once one is
 *        bought.
 *
 * A pack that is bought opens in the middle of the Shop. The items for sale wait aside and the
 * content of the pack takes their place, for free. Once one of them is taken, or none, the pack
 * is closed and the items for sale come back.
 */
#ifndef GAME_SHOP_PACKS_H
#define GAME_SHOP_PACKS_H

#include "item.h"
#include "sprite_container.h"

#include <stdbool.h>

/**
 * @brief Outline of the Shop panel in the background palette. It takes the color of the pack
 *        that is open.
 */
#define SHOP_PANEL_BORDER_PAL_IDX 26
#define SHOP_PANEL_BORDER_COLOR   0x213D

/**
 * @brief Where the opening of a pack stands.
 */
enum ShopPackOpening
{
    SHOP_PACK_OPENING,
    /** The content of the pack is out, one item can be taken */
    SHOP_PACK_OPENED,
    /** The pack had nothing to offer: it is refunded and closed */
    SHOP_PACK_EMPTY
};

/**
 * @brief Put the packs of this visit up for sale.
 *
 * @param shop_items the items for sale in the Shop, an open pack offers its content there
 */
void shop_packs_create(SpriteContainer* shop_items);

/**
 * @brief Throw away every pack and everything an open pack holds, when the Shop is left.
 */
void shop_packs_dispose(void);

/**
 * @brief Get the number of packs that can be selected. None can while a pack is open.
 */
int shop_packs_get_nb_for_sale(void);

/**
 * @brief Get a pack for sale.
 *
 * @return the pack, or NULL if there is none at @p pack_idx or if a pack is open
 */
Item* shop_packs_get_for_sale(int pack_idx);

void shop_packs_set_focus(int pack_idx, bool is_focused);

/**
 * @brief Slide the packs for sale in or out of the screen. They stay out while a pack is open.
 *
 * @param except a pack to leave where it is, may be NULL
 */
void shop_packs_set_shown(bool is_shown, const Item* except);

/**
 * @brief Print the prices of the packs, or how to use the pack that is open.
 */
void shop_packs_print_text(void);

/**
 * @brief Tell whether a pack is opening or open.
 */
bool shop_packs_is_open(void);

/**
 * @brief Buy a pack: it is paid and starts to open.
 *
 * @return false if the pack cannot be bought, and nothing happens
 */
bool shop_packs_buy(int pack_idx);

/**
 * @brief Play one frame of the opening of the pack that was bought.
 *
 * @param nb_frames number of frames since the pack was bought
 */
enum ShopPackOpening shop_packs_update_opening(int nb_frames);

/**
 * @brief Close the open pack: what is left of its content is thrown away, the Shop items come
 *        back. To call once an item was taken, or to pass.
 */
void shop_packs_close(void);

#endif // GAME_SHOP_PACKS_H
