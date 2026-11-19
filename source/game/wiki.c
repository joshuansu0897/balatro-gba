/**
 * @file wiki.c
 *
 * @brief Wiki tab implementation.
 */
#include "game/wiki.h"

#include "blind.h"
#include "boss_blind_effects.h"
#include "game.h"
#include "game/wiki_entries.h"
#include "game_variables.h"
#include "graphic_utils.h"
#include "item.h"
#include "joker.h"
#include "list.h"
#include "skip_tag.h"
#include "sprite.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <tonc.h>

enum WikiCategory
{
    WIKI_CATEGORY_JOKERS,
    WIKI_CATEGORY_TAGS,
    WIKI_CATEGORY_BOSSES,
    WIKI_CATEGORY_MAX
};

// clang-format off
// Positions in tiles
static const Rect          WIKI_PANEL_RECT       = { 2,  5, 27, 16};
static const NinePatchRect WIKI_PANEL_9_PTCH_SRC = {
                                .patch_rect = {28, 20, 30, 22},
                                .margins    = { 1,  1,  1,  1}
};
static const Rect          WIKI_DESC_TEXT_RECT   = { 4, 11, 25, 15};

// Positions in pixels
static const BG_POINT WIKI_HEADER_TEXT_POS   = { 16, 24};
static const BG_POINT WIKI_COUNTER_TEXT_POS  = {184, 24};
static const BG_POINT WIKI_PICTURE_POS       = { 28, 46};
static const BG_POINT WIKI_SMALL_PICTURE_POS = { 36, 54};
static const BG_POINT WIKI_NAME_TEXT_POS     = { 72, 48};
static const BG_POINT WIKI_SUBTITLE_TEXT_POS = { 72, 64};
// clang-format on

static const char* const WIKI_CATEGORY_NAMES[WIKI_CATEGORY_MAX] = {
    [WIKI_CATEGORY_JOKERS] = "Jokers",
    [WIKI_CATEGORY_TAGS] = "Tags",
    [WIKI_CATEGORY_BOSSES] = "Bosses",
};

static const char WIKI_BOSSES_OFF_TEXT[] =
    "Boss Blinds have no effect. Turn Boss Effects on in the Options menu to use them.";

#define WIKI_TEXT_MAX_LENGTH    128
// WHY: room for two numbers of any size, which the compiler checks
#define WIKI_COUNTER_MAX_LENGTH 24

// Frames a direction has to be held before entries scroll by themselves, then between each entry
#define WIKI_SCROLL_REPEAT_DELAY    20
#define WIKI_SCROLL_REPEAT_INTERVAL 5

static bool s_is_shown = false;
static enum WikiCategory s_category = WIKI_CATEGORY_JOKERS;
static int s_entry_idx[WIKI_CATEGORY_MAX] = {0};
static int s_scroll_held_frames = 0;

// The picture of the entry on screen. At most one of them exists at a time.
static JokerObject* s_joker_object = NULL;
static SkipTag* s_skip_tag = NULL;
static Sprite* s_blind_token = NULL;

static int wiki_get_nb_entries(enum WikiCategory category)
{
    switch (category)
    {
        case WIKI_CATEGORY_JOKERS:
            return (int)get_joker_registry_size();
        case WIKI_CATEGORY_TAGS:
            return wiki_entries_get_nb_tags();
        case WIKI_CATEGORY_BOSSES:
            // A single entry tells the effects are off
            return boss_blind_is_enabled() ? wiki_entries_get_nb_bosses() : 1;
        default:
            return 1;
    }
}

static void wiki_destroy_picture(void)
{
    joker_object_destroy(&s_joker_object);
    skip_tag_destroy(&s_skip_tag);
    sprite_destroy(&s_blind_token);
}

/**
 * @brief Put the picture of an entry in place.
 * WHY: a sprite object only reaches its place when the game updates it, which it does not do
 * while paused, so its sprite is put there directly
 */
static void wiki_place_picture(SpriteObject* picture, BG_POINT pos)
{
    if (picture == NULL || picture->sprite == NULL)
        return;

    sprite_object_position(picture, pos.x, pos.y);
    sprite_position(picture->sprite, pos.x, pos.y);
}

static void wiki_print_at(BG_POINT pos, u8 text_color, const char* text)
{
    tte_printf("#{P:%d,%d; cx:0x%X000}%s", pos.x, pos.y, text_color, text);
}

static void wiki_print_header(void)
{
    // Print category names, the current one stands out
    BG_POINT pos = WIKI_HEADER_TEXT_POS;
    for (int category = 0; category < WIKI_CATEGORY_MAX; category++)
    {
        const char* name = WIKI_CATEGORY_NAMES[category];
        u8 text_color = (category == (int)s_category) ? TTE_YELLOW_PB : TTE_WHITE_PB;

        wiki_print_at(pos, text_color, name);
        pos.x += ((int)strlen(name) + 1) * TTE_CHAR_SIZE;
    }

    // Print entry counter
    char counter[WIKI_COUNTER_MAX_LENGTH];
    int nb_entries = wiki_get_nb_entries(s_category);
    snprintf(counter, sizeof(counter), "%2d/%-2d", s_entry_idx[s_category] + 1, nb_entries);
    wiki_print_at(WIKI_COUNTER_TEXT_POS, TTE_WHITE_PB, counter);
}

static void wiki_print_title(const char* name, const char* subtitle)
{
    wiki_print_at(WIKI_NAME_TEXT_POS, TTE_BLACK_PB, name);
    wiki_print_at(WIKI_SUBTITLE_TEXT_POS, TTE_DARK_BLUE_PB, subtitle);
}

static void wiki_print_plain_text(const char* plain_text)
{
    char text[WIKI_TEXT_MAX_LENGTH];
    snprintf(text, sizeof(text), TTE_BLACK_TAG "%s", plain_text);

    tte_printf_justified_in_rect(text, WIKI_DESC_TEXT_RECT, JUSTIFY_CENTER, SCREEN_LEFT, true);
}

static void wiki_show_joker(int joker_id)
{
    // WHY: a description can depend on the state of its Joker, so a real one is created for it
    Joker* joker = joker_new((u8)joker_id);
    GBAL_RETURN_IF_NULL(joker, RET_NONE);

    s_joker_object = joker_object_new(joker);
    if (s_joker_object == NULL)
    {
        joker_destroy(&joker);
        return;
    }

    Item* item = (Item*)s_joker_object;
    wiki_place_picture((SpriteObject*)s_joker_object, WIKI_PICTURE_POS);
    wiki_print_title(item_get_name(item), item_get_subtype_info(item).name_str);
    item_print_description(item, WIKI_DESC_TEXT_RECT);
}

static void wiki_show_tag(int tag_idx)
{
    const WikiTagEntry* entry = wiki_entries_get_tag(tag_idx);
    GBAL_RETURN_IF_NULL(entry, RET_NONE);

    // WHY: the layers up to this one are taken by the Tags the player owns
    int layer = OWNED_SKIP_TAG_STARTING_LAYER + list_get_len(get_owned_skip_tags());
    if (layer < MAX_SKIP_TAGS)
    {
        s_skip_tag = skip_tag_new(entry->type);
        skip_tag_set_sprite(s_skip_tag, layer);
        wiki_place_picture((SpriteObject*)s_skip_tag, WIKI_SMALL_PICTURE_POS);
    }

    wiki_print_title(entry->name, "Skip Tag");
    wiki_print_plain_text(entry->description);
}

static void wiki_show_boss(int boss_idx)
{
    if (!boss_blind_is_enabled())
    {
        wiki_print_title("Boss Effects", "Off");
        wiki_print_plain_text(WIKI_BOSSES_OFF_TEXT);
        return;
    }

    enum BlindType blind = wiki_entries_get_boss(boss_idx);
    s_blind_token =
        blind_token_new(blind, WIKI_PICTURE_POS.x, WIKI_PICTURE_POS.y, BOSS_BLIND_TOKEN_LAYER);

    wiki_print_title(boss_blind_get_name(blind), "Boss Blind");
    wiki_print_plain_text(wiki_entries_get_boss_text(blind));
}

static void wiki_draw_entry(void)
{
    // Clear previous entry
    wiki_destroy_picture();
    tte_erase_screen();

    // WHY: the number of Boss entries changes with the Boss Effects option
    int nb_entries = wiki_get_nb_entries(s_category);
    if (s_entry_idx[s_category] >= nb_entries)
        s_entry_idx[s_category] = 0;

    wiki_print_header();

    switch (s_category)
    {
        case WIKI_CATEGORY_JOKERS:
            wiki_show_joker(s_entry_idx[s_category]);
            break;
        case WIKI_CATEGORY_TAGS:
            wiki_show_tag(s_entry_idx[s_category]);
            break;
        case WIKI_CATEGORY_BOSSES:
            wiki_show_boss(s_entry_idx[s_category]);
            break;
        default:
            break;
    }
}

/**
 * @brief Get which way the entries scroll this frame: once per press, then repeatedly while held.
 *
 * @return 1 for the next entry, -1 for the previous one, 0 for none
 */
static int wiki_get_scroll_direction(void)
{
    int direction = bit_tribool(key_curr_state(), KI_DOWN, KI_UP);
    if (direction == 0)
    {
        s_scroll_held_frames = 0;
        return 0;
    }

    s_scroll_held_frames++;
    bool is_first_frame = (s_scroll_held_frames == 1);
    bool is_repeating = s_scroll_held_frames > WIKI_SCROLL_REPEAT_DELAY &&
                        s_scroll_held_frames % WIKI_SCROLL_REPEAT_INTERVAL == 0;

    return (is_first_frame || is_repeating) ? direction : 0;
}

void wiki_show(void)
{
    s_is_shown = true;
    s_scroll_held_frames = 0;

    main_bg_se_copy_expand_9_patch(WIKI_PANEL_RECT, &WIKI_PANEL_9_PTCH_SRC);
    wiki_draw_entry();
}

void wiki_update(void)
{
    if (!s_is_shown)
        return;

    int category_direction = bit_tribool(key_hit(KEY_ANY), KI_RIGHT, KI_LEFT);
    if (category_direction != 0)
    {
        s_category = wrap(s_category + category_direction, 0, WIKI_CATEGORY_MAX);
        wiki_draw_entry();
        return;
    }

    int scroll_direction = wiki_get_scroll_direction();
    if (scroll_direction == 0)
        return;

    int nb_entries = wiki_get_nb_entries(s_category);
    s_entry_idx[s_category] = wrap(s_entry_idx[s_category] + scroll_direction, 0, nb_entries);
    wiki_draw_entry();
}

void wiki_hide(void)
{
    if (!s_is_shown)
        return;

    s_is_shown = false;
    wiki_destroy_picture();

    // WHY: a Boss Blind picture loads its colors where the Blind of the Round keeps its own
    apply_blind_colors(g_game_vars.current_blind);
}
