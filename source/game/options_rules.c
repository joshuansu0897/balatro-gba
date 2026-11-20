/**
 * @file options_rules.c
 *
 * @brief Rules page of the Options menu implementation.
 */
#include "game/options_rules.h"

#include "audio_utils.h"
#include "boss_blind_effects.h"
#include "button.h"
#include "game.h"
#include "game/options_menu.h"
#include "graphic_utils.h"
#include "selection_grid.h"
#include "shop_rules.h"
#include "soundbank.h"
#include "util.h"

#include <tonc.h>

enum RulesRow
{
    BOSS_EFFECTS_ROW_IDX,
    PLANET_CARDS_ROW_IDX,
    PLANET_PACK_ROW_IDX,
    JOKER_PACK_ROW_IDX,
    NB_SWITCH_ROWS,
    SAVE_CANCEL_ROW_IDX = NB_SWITCH_ROWS,
    NB_RULES_ROWS
};

// Outlines of the checkboxes that only this page draws, see background_options_menu_gfx.png
#define PLANET_CARDS_OUTLINE_COLOR_PAL_IDX 26
#define PLANET_PACK_OUTLINE_COLOR_PAL_IDX  27
#define JOKER_PACK_OUTLINE_COLOR_PAL_IDX   28

// Positions in tiles. A switch is its label, then its checkbox, both 2 tiles high.
#define SWITCH_LABEL_COL  7
#define SWITCH_CHECK_COL  21
#define SWITCH_FIRST_ROW  3
#define SWITCH_ROW_HEIGHT 3

// The rows the Display page draws on, between the page names and the buttons
#define PAGE_FIRST_ROW  3
#define PAGE_LAST_ROW   15
#define PAGE_FIRST_COL  4

// clang-format off
// A row of the panel with nothing drawn on it, used to wipe the Display page
static const Rect PAGE_BLANK_ROW_SRC    = { 4,  2, 25,  2};
// WHY: the row above the buttons also holds the shadow of the lower volume bar. The image keeps
// a copy of it without that shadow for this page.
static const Rect BUTTONS_TOP_ROW_SRC   = { 4, 30, 25, 30};
static const BG_POINT BUTTONS_TOP_ROW_POS = { 4, 16};

// Text of the Display page, in pixels
static const Rect PAGE_TEXT_RECT        = {32, 16, 208, 128};

typedef struct
{
    Rect label_src;
    Rect check_no_src;
    Rect check_yes_src;
} SwitchGfx;

static const SwitchGfx SWITCHES_GFX[NB_SWITCH_ROWS] = {
    [BOSS_EFFECTS_ROW_IDX] = {{ 0, 26, 11, 27}, { 8, 24,  9, 25}, {10, 24, 11, 25}},
    [PLANET_CARDS_ROW_IDX] = {{14, 26, 25, 27}, {12, 24, 13, 25}, {14, 24, 15, 25}},
    [PLANET_PACK_ROW_IDX]  = {{ 0, 28, 11, 29}, {16, 24, 17, 25}, {18, 24, 19, 25}},
    [JOKER_PACK_ROW_IDX]   = {{14, 28, 25, 29}, {20, 24, 21, 25}, {22, 24, 23, 25}},
};

// WHY: the Boss Blind effects have their own switch, the other rows are Shop rules
static const enum ShopRule SWITCHES_SHOP_RULE[NB_SWITCH_ROWS] = {
    [BOSS_EFFECTS_ROW_IDX] = SHOP_RULE_MAX,
    [PLANET_CARDS_ROW_IDX] = SHOP_RULE_PLANET_CARDS,
    [PLANET_PACK_ROW_IDX]  = SHOP_RULE_PLANET_PACK,
    [JOKER_PACK_ROW_IDX]   = SHOP_RULE_JOKER_PACK,
};
// clang-format on

static bool rules_switch_is_on(enum RulesRow row)
{
    if (row == BOSS_EFFECTS_ROW_IDX)
        return boss_blind_is_enabled();

    return shop_rules_is_on(SWITCHES_SHOP_RULE[row]);
}

static void rules_switch_draw(enum RulesRow row)
{
    const SwitchGfx* gfx = &SWITCHES_GFX[row];
    int tile_row = SWITCH_FIRST_ROW + row * SWITCH_ROW_HEIGHT;
    BG_POINT label_pos = {SWITCH_LABEL_COL, tile_row};
    BG_POINT check_pos = {SWITCH_CHECK_COL, tile_row};
    Rect check_src = rules_switch_is_on(row) ? gfx->check_yes_src : gfx->check_no_src;

    main_bg_se_copy_rect(gfx->label_src, label_pos);
    main_bg_se_copy_rect(check_src, check_pos);
}

static void rules_switch_toggle(enum RulesRow row)
{
    if (row == BOSS_EFFECTS_ROW_IDX)
        boss_blind_set_enabled(!boss_blind_is_enabled());
    else
        shop_rules_set(SWITCHES_SHOP_RULE[row], !shop_rules_is_on(SWITCHES_SHOP_RULE[row]));

    // WHY: a switch can take another one with it, the Planet Pack needs the Planet cards
    for (int drawn_row = 0; drawn_row < NB_SWITCH_ROWS; drawn_row++)
        rules_switch_draw(drawn_row);
}

static void boss_effects_on_pressed(void)
{
    rules_switch_toggle(BOSS_EFFECTS_ROW_IDX);
}

static void planet_cards_on_pressed(void)
{
    rules_switch_toggle(PLANET_CARDS_ROW_IDX);
}

static void planet_pack_on_pressed(void)
{
    rules_switch_toggle(PLANET_PACK_ROW_IDX);
}

static void joker_pack_on_pressed(void)
{
    rules_switch_toggle(JOKER_PACK_ROW_IDX);
}

// clang-format off
#define SWITCH_BUTTON(outline_pal_idx, on_pressed) \
    {{outline_pal_idx, MENU_BUTTON_MAIN_COLOR_PAL_IDX, on_pressed, NULL}}

// WHY: these tables are kept out of the fast memory, which the stack needs
static EWRAM_DATA Button s_rules_buttons[NB_RULES_ROWS][2] = {
    SWITCH_BUTTON(BOSS_BUTTON_OUTLINE_COLOR_PAL_IDX,  boss_effects_on_pressed),
    SWITCH_BUTTON(PLANET_CARDS_OUTLINE_COLOR_PAL_IDX, planet_cards_on_pressed),
    SWITCH_BUTTON(PLANET_PACK_OUTLINE_COLOR_PAL_IDX,  planet_pack_on_pressed),
    SWITCH_BUTTON(JOKER_PACK_OUTLINE_COLOR_PAL_IDX,   joker_pack_on_pressed),
    {
        {SAVE_BUTTON_OUTLINE_COLOR_PAL_IDX, SAVE_BUTTON_MAIN_COLOR_PAL_IDX, options_menu_save,   NULL},
        {BACK_BUTTON_OUTLINE_COLOR_PAL_IDX, BACK_BUTTON_MAIN_COLOR_PAL_IDX, options_menu_cancel, NULL},
    }
};
// clang-format on

static int rules_switch_row_get_size(void)
{
    return 1;
}

static int rules_buttons_row_get_size(void)
{
    return 2;
}

static bool rules_row_on_selection_changed(
    SelectionGrid* selection_grid,
    int row_idx,
    const Selection* prev_selection,
    const Selection* new_selection
)
{
    if (prev_selection->y == row_idx)
        button_set_highlight(&s_rules_buttons[row_idx][prev_selection->x], false);

    if (new_selection->y == row_idx)
    {
        play_sfx(SFX_BUTTON, MM_BASE_PITCH_RATE, BUTTON_SFX_VOLUME);
        button_set_highlight(&s_rules_buttons[row_idx][new_selection->x], true);
    }

    return true;
}

static void rules_row_on_key_transit(SelectionGrid* selection_grid, Selection* selection)
{
    if (key_hit(KEY_CONFIRM))
        button_press(&s_rules_buttons[selection->y][selection->x]);
}

// clang-format off
#define RULES_ROW(row_idx, get_size) \
    {row_idx, get_size, rules_row_on_selection_changed, rules_row_on_key_transit, {.wrap = false}}

static EWRAM_DATA SelectionGridRow s_rules_rows[NB_RULES_ROWS] = {
    RULES_ROW(BOSS_EFFECTS_ROW_IDX, rules_switch_row_get_size),
    RULES_ROW(PLANET_CARDS_ROW_IDX, rules_switch_row_get_size),
    RULES_ROW(PLANET_PACK_ROW_IDX,  rules_switch_row_get_size),
    RULES_ROW(JOKER_PACK_ROW_IDX,   rules_switch_row_get_size),
    RULES_ROW(SAVE_CANCEL_ROW_IDX,  rules_buttons_row_get_size),
};
// clang-format on

static const Selection RULES_INIT_SEL = {0, BOSS_EFFECTS_ROW_IDX};

static EWRAM_DATA SelectionGrid s_rules_grid = {s_rules_rows, NB_RULES_ROWS, RULES_INIT_SEL};

void options_rules_show(void)
{
    // Wipe the Display page
    tte_erase_rect_wrapper(PAGE_TEXT_RECT);
    for (int tile_row = PAGE_FIRST_ROW; tile_row <= PAGE_LAST_ROW; tile_row++)
    {
        BG_POINT row_pos = {PAGE_FIRST_COL, tile_row};
        main_bg_se_copy_rect(PAGE_BLANK_ROW_SRC, row_pos);
    }
    main_bg_se_copy_rect(BUTTONS_TOP_ROW_SRC, BUTTONS_TOP_ROW_POS);

    // Draw the switches
    for (int row = 0; row < NB_SWITCH_ROWS; row++)
        rules_switch_draw(row);

    // Put the cursor on the first switch
    s_rules_grid.selection = RULES_INIT_SEL;
    for (int row = 0; row < NB_SWITCH_ROWS; row++)
        button_set_highlight(&s_rules_buttons[row][0], row == RULES_INIT_SEL.y);

    button_set_highlight(&s_rules_buttons[SAVE_CANCEL_ROW_IDX][0], false);
    button_set_highlight(&s_rules_buttons[SAVE_CANCEL_ROW_IDX][1], false);
}

void options_rules_update(void)
{
    selection_grid_process_input(&s_rules_grid);
}
