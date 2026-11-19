/**
 * @file run_info.c
 *
 * @brief Run Info page implementation: the Hand Levels and Deck tabs, and the tab bar they
 * share with the Wiki.
 */
#include "game/run_info.h"

#include "background_run_info_gfx.h"
#include "button.h"
#include "card.h"
#include "game.h"
#include "game/wiki.h"
#include "selection_grid.h"
#include "state_machine.h"

// Palette indices
#define TAB_MAIN_COLOR_PAL_IDX          1
#define HANDS_TAB_OUTLINE_COLOR_PAL_IDX 2
#define DECK_TAB_OUTLINE_COLOR_PAL_IDX  3
#define WIKI_TAB_OUTLINE_COLOR_PAL_IDX  4

#define HAND_LEVELS_NB_HANDS_SHOWN 7

// TODO: Rects and Points
// clang-format off
static const BG_POINT RUN_INFO_CLEAR_SRC_POS            = { 1,  1};
static const Rect     RUN_INFO_CLEAR_DEST_RECT          = { 1,  3, 28, 17};

static const Rect     HAND_LEVELS_LVL_TEXT_SRC_RECT       = { 6, 24,  9, 27};
static const BG_POINT HAND_LEVELS_LVL_TEXT_DEST_POS       = { 1,  3};
static const Rect     HAND_LEVELS_LVL_BOX_SRC_RECT        = { 9, 24, 11, 27};
static const BG_POINT HAND_LEVELS_LVL_BOX_DEST_POS        = { 5,  3};

static const Rect     HAND_LEVELS_NAME_BOX_SRC_COL        = {12, 24, 12, 27};
static const BG_POINT HAND_LEVELS_NAME_BOX_DEST_POS       = { 8,  3};
static const u16      HAND_LEVELS_NAME_BOX_DEST_LENGTH    =   7;

static const BG_POINT HAND_LEVELS_CHIPS_BOX_SRC_3W_ROW    = {13, 24};
static const Rect     HAND_LEVELS_CHIPS_BOX_DEST_RECT     = {15,  3, 19,  3};
static const int      HAND_LEVELS_CHIPS_BOX_DEST_HEIGHT   =   4;

static const Rect     HAND_LEVELS_MULT_BOX_SRC_COL        = {16, 24, 16, 27};
static const BG_POINT HAND_LEVELS_MULT_BOX_DEST_POS       = {20,  3};
static const u16      HAND_LEVELS_MULT_BOX_DEST_LENGTH    =   3;

static const Rect     HAND_LEVELS_NB_USES_SRC_RECT        = {17, 24, 22, 27};
static const BG_POINT HAND_LEVELS_NB_USES_DEST_POS        = {23,  3};

static const Rect     HAND_LEVELS_BOTTOM_LINE_SRC_RECT    = { 1,  6, 28,  6};
static const BG_POINT HAND_LEVELS_BOTTOM_LINE_DEST_POS    = { 1, 17};

static const Rect     HAND_LEVELS_MIDDLE_LINE_SRC_RECT    = { 1,  4, 28,  5};
static const BG_POINT HAND_LEVELS_MIDDLE_LINE_BASE_POS    = { 1,  6};
static const int      HAND_LEVELS_MIDDLE_LINE_NB_COPY     = HAND_LEVELS_NB_HANDS_SHOWN - 1;

static const Rect     HAND_LEVELS_ARROW_UP_ON_SRC_RECR    = {23, 24, 24, 25};
static const Rect     HAND_LEVELS_ARROW_UP_OFF_SRC_RECR   = {25, 24, 26, 25};
static const BG_POINT HAND_LEVELS_ARROW_UP_DEST_POS       = {14,  2};

static const Rect     HAND_LEVELS_ARROW_DOWN_ON_SRC_RECR  = {23, 26, 24, 27};
static const Rect     HAND_LEVELS_ARROW_DOWN_OFF_SRC_RECR = {25, 26, 26, 27};
static const BG_POINT HAND_LEVELS_ARROW_DOWN_DEST_POS     = {14, 17};

static const BG_POINT HAND_LEVELS_TEXT_POS                = {32, 32};

// Deck tab: one row per suit, one column per rank
static const BG_POINT PEEK_DECK_TEXT_POS                  = { 8, 40};
static const BG_POINT PEEK_DECK_TOTAL_TEXT_POS            = { 8, 128};
// clang-format on

#define PEEK_DECK_RANK_COL_WIDTH 2
#define PEEK_DECK_ROWS_SPACING   (2 * TILE_SIZE)

// Ranks from the Ace down to the 2, each right-aligned in its column
static const char PEEK_DECK_RANKS_HEADER[] = " A K Q J10 9 8 7 6 5 4 3 2";

typedef struct
{
    u8 suit;
    char letter;
    const char* color_tag;
} PeekDeckSuitRow;

// clang-format off
static const PeekDeckSuitRow PEEK_DECK_SUIT_ROWS[NUM_SUITS] = {
    {SPADES,   'S', TTE_BLUE_TAG  },
    {HEARTS,   'H', TTE_RED_TAG   },
    {CLUBS,    'C', TTE_GREEN_TAG },
    {DIAMONDS, 'D', TTE_YELLOW_TAG},
};
// clang-format on

enum RunInfoState
{
    RUN_INFO_STATE_HAND_LEVELS,
    RUN_INFO_STATE_PEEK_DECK,
    RUN_INFO_STATE_WIKI,
    RUN_INFO_STATE_MAX
};

static void hand_level_state_init(void);
static void peek_deck_state_init(void);
static void wiki_state_init(void);

// clang-format off
static StateInfo state_info[] =
{
    // WHY: the page is shown while the game is paused, when state machines are not updated.
    // Its owner calls run_info_update() instead, so only the init functions are used here.
    [RUN_INFO_STATE_HAND_LEVELS] = STATE_INFO_INIT_UPDATE_FN(
        hand_level_state_init, 
        noop
    ),
    [RUN_INFO_STATE_PEEK_DECK] = STATE_INFO_INIT_UPDATE_FN(
        peek_deck_state_init,
        noop
    ),
    [RUN_INFO_STATE_WIKI] = {
        .on_init = wiki_state_init,
        .on_update = noop,
        .on_exit = wiki_hide
    }
};
// clang-format on

static StateMachine run_info_sm = {
    .state_infos = &state_info[0],
    .num_infos = RUN_INFO_STATE_MAX,
};

// clang-format off
static Button tab_buttons[] = {
    [RUN_INFO_STATE_HAND_LEVELS] = {
        HANDS_TAB_OUTLINE_COLOR_PAL_IDX,
        TAB_MAIN_COLOR_PAL_IDX,
        NULL, NULL
    },
    [RUN_INFO_STATE_PEEK_DECK] = {
        DECK_TAB_OUTLINE_COLOR_PAL_IDX,
        TAB_MAIN_COLOR_PAL_IDX,
        NULL, NULL
    },
    [RUN_INFO_STATE_WIKI] = {
        WIKI_TAB_OUTLINE_COLOR_PAL_IDX,
        TAB_MAIN_COLOR_PAL_IDX,
        NULL, NULL
    }
};
// clang-format on

static void run_info_compute_data_on_display(void);
static void run_info_handle_tab_change(enum RunInfoState new_state);
static void hand_levels_print_info(void);
static void hand_levels_handle_vert_scroll(enum ScreenVertDir page_dir);
static void hand_levels_update_arrows(void);

/**
 * @brief Lists the Hand Types to display, in the right order and without any gaps.
 */
static enum HandType s_hands_on_display[HAND_TYPE_MAX + 1] = {0};

/**
 * @brief How many valid Hand Types are listed in @p s_hands_on_display
 *
 * By default, all Hands up to the STRAIGHT_FLUSH are shown, the ones above are secret
 *
 * @sa s_hands_on_display
 */
static u8 s_nb_hand_types = HAND_TYPE_NORMAL_MAX;

/**
 * @brief Scrolling offset in the Hands Levels tab, expressed as an offset from the highest Hand
 * shown
 */
static u8 s_hand_levels_offset = 0;

/**
 * @brief If true, do not update background Tiles, as they are already arranged as needed when
 * loaded from the ROM via GRIT_CPY
 */
static bool first_show = true;

void run_info_show(void)
{
    GRIT_CPY(pal_bg_mem, background_run_info_gfxPal);
    GRIT_CPY(&tile_mem[MAIN_BG_CBB], background_run_info_gfxTiles);
    GRIT_CPY(&se_mem[MAIN_BG_SBB], background_run_info_gfxMap);

    run_info_compute_data_on_display();

    first_show = true;
    state_machine_register(&run_info_sm);
    run_info_handle_tab_change(RUN_INFO_STATE_HAND_LEVELS);
}

void run_info_hide(void)
{
    // WHY: removing the state machine does not run the exit function of its state
    wiki_hide();

    tte_erase_screen();
    state_machine_remove(&run_info_sm);
}

/**
 * @brief Print menu text and, if needed, setup background tiles for the Hand Levels menu
 */
static void hand_level_state_init(void)
{
    tte_erase_screen();

    hand_levels_print_info();

    // No need to manipulate anything in the background tiles when they're first copied
    // as they already represent the first page of the Hand Levels menu
    if (first_show)
    {
        first_show = false;
        return;
    }

    main_bg_se_copy_rect(HAND_LEVELS_LVL_TEXT_SRC_RECT, HAND_LEVELS_LVL_TEXT_DEST_POS);
    main_bg_se_copy_rect(HAND_LEVELS_LVL_BOX_SRC_RECT, HAND_LEVELS_LVL_BOX_DEST_POS);

    main_bg_se_copy_expand_column_hor(
        HAND_LEVELS_NAME_BOX_DEST_POS,
        HAND_LEVELS_NAME_BOX_DEST_LENGTH,
        HAND_LEVELS_NAME_BOX_SRC_COL
    );

    Rect chips_dest_rect = HAND_LEVELS_CHIPS_BOX_DEST_RECT;
    BG_POINT chips_src_pos = HAND_LEVELS_CHIPS_BOX_SRC_3W_ROW;
    for (int i = 0; i < HAND_LEVELS_CHIPS_BOX_DEST_HEIGHT; i++)
    {
        main_bg_se_copy_expand_3w_row(chips_dest_rect, chips_src_pos);
        chips_dest_rect.top++;
        chips_dest_rect.bottom++;
        chips_src_pos.y++;
    }

    main_bg_se_copy_expand_column_hor(
        HAND_LEVELS_MULT_BOX_DEST_POS,
        HAND_LEVELS_MULT_BOX_DEST_LENGTH,
        HAND_LEVELS_MULT_BOX_SRC_COL
    );

    main_bg_se_copy_rect(HAND_LEVELS_NB_USES_SRC_RECT, HAND_LEVELS_NB_USES_DEST_POS);

    // Copy the last line's bottom before it gets erased
    main_bg_se_copy_rect(HAND_LEVELS_BOTTOM_LINE_SRC_RECT, HAND_LEVELS_BOTTOM_LINE_DEST_POS);

    // Copy the full line we just completed 5 times over
    Rect middle_line_src = HAND_LEVELS_MIDDLE_LINE_SRC_RECT;
    BG_POINT middle_line_dest = HAND_LEVELS_MIDDLE_LINE_BASE_POS;
    int middle_line_height = rect_height(&HAND_LEVELS_MIDDLE_LINE_SRC_RECT);
    for (int i = 0; i < HAND_LEVELS_MIDDLE_LINE_NB_COPY; i++)
    {
        main_bg_se_copy_rect(middle_line_src, middle_line_dest);
        middle_line_dest.y += middle_line_height;
        // Don't copy the joint between two lines on the last one, as there is no line underneath
        if (i == HAND_LEVELS_MIDDLE_LINE_NB_COPY - 2)
            middle_line_src.bottom -= 1;
    }

    hand_levels_update_arrows();
}

/**
 * @brief Count the cards left in the Deck, by suit and rank.
 *
 * @param counts filled with the number of cards left for each suit and rank
 *
 * @return the total number of cards left in the Deck
 */
static int peek_deck_count_cards(u8 counts[NUM_SUITS][NUM_RANKS])
{
    int nb_cards = 0;

    for (int idx = 0; idx <= get_deck_top(); idx++)
    {
        const Card* card = deck_get_at_idx(idx);
        if (card == NULL || card->suit >= NUM_SUITS || card->rank >= NUM_RANKS)
            continue;

        counts[card->suit][card->rank]++;
        nb_cards++;
    }

    return nb_cards;
}

/**
 * @brief Write the number of cards left of each rank, in the order of @ref PEEK_DECK_RANKS_HEADER.
 *
 * @param cells output string, needs room for every rank column and the terminator
 * @param counts number of cards left of each rank for one suit
 */
static void peek_deck_format_counts(char* cells, const u8 counts[NUM_RANKS])
{
    for (int col = 0; col < NUM_RANKS; col++)
    {
        u8 count = counts[ACE - col];
        char* cell = cells + col * PEEK_DECK_RANK_COL_WIDTH;

        // A rank with no card left shows as a dash
        cell[0] = (count >= 10) ? ('0' + count / 10) : ' ';
        cell[1] = (count == 0) ? '-' : ('0' + count % 10);
    }

    cells[NUM_RANKS * PEEK_DECK_RANK_COL_WIDTH] = '\0';
}

/**
 * @brief Print how many cards of each suit and rank are left to draw in the Deck.
 */
static void peek_deck_print_cards(void)
{
    u8 counts[NUM_SUITS][NUM_RANKS] = {0};
    int nb_cards = peek_deck_count_cards(counts);

    // Print rank names
    // WHY: the leading space skips the column of the suit letters
    tte_printf(
        "#{P:%d,%d}%s %s",
        PEEK_DECK_TEXT_POS.x,
        PEEK_DECK_TEXT_POS.y,
        TTE_WHITE_TAG,
        PEEK_DECK_RANKS_HEADER
    );

    // Print one row per suit
    char cells[NUM_RANKS * PEEK_DECK_RANK_COL_WIDTH + 1];
    for (int row = 0; row < NUM_SUITS; row++)
    {
        const PeekDeckSuitRow* suit_row = &PEEK_DECK_SUIT_ROWS[row];
        peek_deck_format_counts(cells, counts[suit_row->suit]);

        tte_printf(
            "#{P:%d,%d}%s%c%s%s",
            PEEK_DECK_TEXT_POS.x,
            PEEK_DECK_TEXT_POS.y + (row + 1) * PEEK_DECK_ROWS_SPACING,
            suit_row->color_tag,
            suit_row->letter,
            TTE_WHITE_TAG,
            cells
        );
    }

    // Print total
    tte_printf(
        "#{P:%d,%d}%sCards left: %d",
        PEEK_DECK_TOTAL_TEXT_POS.x,
        PEEK_DECK_TOTAL_TEXT_POS.y,
        TTE_WHITE_TAG,
        nb_cards
    );
}

/**
 * @brief Print menu text and setup background tiles for the Deck menu
 */
static void peek_deck_state_init(void)
{
    tte_erase_screen();
    main_bg_se_copy_rect(HAND_LEVELS_ARROW_UP_OFF_SRC_RECR, HAND_LEVELS_ARROW_UP_DEST_POS);
    main_bg_se_copy_rect(HAND_LEVELS_ARROW_DOWN_OFF_SRC_RECR, HAND_LEVELS_ARROW_DOWN_DEST_POS);
    main_bg_se_copy_expand_tile(RUN_INFO_CLEAR_DEST_RECT, RUN_INFO_CLEAR_SRC_POS);

    peek_deck_print_cards();
}

/**
 * @brief Clear the content area and let the Wiki draw itself in it
 */
static void wiki_state_init(void)
{
    main_bg_se_copy_rect(HAND_LEVELS_ARROW_UP_OFF_SRC_RECR, HAND_LEVELS_ARROW_UP_DEST_POS);
    main_bg_se_copy_rect(HAND_LEVELS_ARROW_DOWN_OFF_SRC_RECR, HAND_LEVELS_ARROW_DOWN_DEST_POS);
    main_bg_se_copy_expand_tile(RUN_INFO_CLEAR_DEST_RECT, RUN_INFO_CLEAR_SRC_POS);

    wiki_show();
}

void run_info_update(void)
{
    enum RunInfoState new_state = RUN_INFO_STATE_MAX;
    if (key_hit(KEY_TAB_LEFT))
        new_state = (run_info_sm.state - 1 + RUN_INFO_STATE_MAX) % RUN_INFO_STATE_MAX;
    else if (key_hit(KEY_TAB_RIGHT))
        new_state = (run_info_sm.state + 1) % RUN_INFO_STATE_MAX;

    if (new_state != RUN_INFO_STATE_MAX)
    {
        run_info_handle_tab_change(new_state);
        return;
    }

    // The Wiki handles the directions itself
    if (run_info_sm.state == RUN_INFO_STATE_WIKI)
    {
        wiki_update();
        return;
    }

    // WHY: only the Hand Levels tab scrolls, on the Deck tab this would print the Hands over it
    if (run_info_sm.state != RUN_INFO_STATE_HAND_LEVELS)
        return;

    enum ScreenVertDir page_dir = 0;

    if (key_hit(KEY_UP))
        page_dir = SCREEN_UP;
    else if (key_hit(KEY_DOWN))
        page_dir = SCREEN_DOWN;

    if (page_dir != 0)
        hand_levels_handle_vert_scroll(page_dir);
}

/**
 * @brief Given a scrolling direction, update the Hand Levels tab
 *
 * @param scroll_dir which direction we scrolled by pressing UP or DOWN
 */
static void hand_levels_handle_vert_scroll(enum ScreenVertDir scroll_dir)
{
    s_hand_levels_offset =
        max(0,
            min(s_nb_hand_types - HAND_LEVELS_NB_HANDS_SHOWN, s_hand_levels_offset + scroll_dir));

    hand_levels_print_info();
    hand_levels_update_arrows();
}

/**
 * @brief Show the appropriate arrows, indicating whether we can still scroll up or down in the
 * Hands Levels tab
 */
static inline void hand_levels_update_arrows(void)
{
    Rect arrow_up_src_rect;
    Rect arrow_down_src_rect;

    if (s_hand_levels_offset == 0)
        arrow_up_src_rect = HAND_LEVELS_ARROW_UP_OFF_SRC_RECR;
    else
        arrow_up_src_rect = HAND_LEVELS_ARROW_UP_ON_SRC_RECR;

    if (s_hand_levels_offset == s_nb_hand_types - HAND_LEVELS_NB_HANDS_SHOWN)
        arrow_down_src_rect = HAND_LEVELS_ARROW_DOWN_OFF_SRC_RECR;
    else
        arrow_down_src_rect = HAND_LEVELS_ARROW_DOWN_ON_SRC_RECR;

    main_bg_se_copy_rect(arrow_up_src_rect, HAND_LEVELS_ARROW_UP_DEST_POS);
    main_bg_se_copy_rect(arrow_down_src_rect, HAND_LEVELS_ARROW_DOWN_DEST_POS);
}

/**
 * @brief Print all the relevant information about the Hand Types shown.
 *
 * This includes the Level, Name, Chips and Mult, and the number of times a particular Hand Type has
 * been played this run
 */
static inline void hand_levels_print_info(void)
{
    int hand_info_y = HAND_LEVELS_TEXT_POS.y;

    // Printed highest-hand first
    for (u8 i = 0; i < HAND_LEVELS_NB_HANDS_SHOWN; i++)
    {
        enum HandType hand_type = s_hands_on_display[s_hand_levels_offset + i];
        HandBonus hand_bonus = get_hand_total_bonus(hand_type);
        int nb_played_padding = (g_game_vars.nb_played_hands[hand_type - 1] < 10) ? 1 : 0;

        tte_printf(
            "#{P:%d,%d}%s%-3ld %s%-7s %3ld %-3ld  %s%*s%ld",
            HAND_LEVELS_TEXT_POS.x,
            hand_info_y,
            TTE_DARK_GREEN_TAG,
            hand_get_level(hand_type),
            TTE_WHITE_TAG,
            get_hand_type_name(hand_type),
            hand_bonus.chips,
            hand_bonus.mult,
            TTE_YELLOW_TAG,
            nb_played_padding,
            "",
            g_game_vars.nb_played_hands[hand_type - 1]
        );

        hand_info_y += 2 * TILE_SIZE;
    };
}

/**
 * @brief This function changes the state of the menu to the new one, as determined during input
 * ptocessing, and highlights the corresponding Tab at the top of the screen
 *
 * @param new_state new menu StateMachine state to change to
 *
 * @sa run_info_update
 */
static inline void run_info_handle_tab_change(enum RunInfoState new_state)
{
    for (enum RunInfoState i = 0; i < RUN_INFO_STATE_MAX; i++)
    {
        button_set_highlight(&tab_buttons[i], i == new_state);
    }
    state_machine_change_state(&run_info_sm, new_state);
}

/**
 * @brief Compute all data that will need to be displayed if it's not already available though @p
 * g_game_vars
 */
static inline void run_info_compute_data_on_display(void)
{
    s_nb_hand_types = 0;
    s_hand_levels_offset = 0;

    // This will fill the whole array every time, padding it with value NONE
    enum HandType hand_type = HAND_TYPE_MAX;
    int idx = 0;
    while (idx <= HAND_TYPE_MAX)
    {
        if (!is_hand_type_secret(hand_type) || g_game_vars.nb_played_hands[hand_type - 1] > 0)
        {
            s_hands_on_display[idx] = hand_type;
            idx++;
            if (hand_type > NONE)
                s_nb_hand_types++;
        }

        if (hand_type > NONE)
            hand_type--;
    }
}
