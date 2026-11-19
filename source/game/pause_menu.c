/**
 * @file pause_menu.c
 *
 * @brief Pause overlay implementation.
 */
#include "game/pause_menu.h"

#include "audio_utils.h"
#include "game.h"
#include "game/overlay.h"
#include "game/run_info.h"
#include "graphic_utils.h"
#include "soundbank.h"

#include <string.h>
#include <tonc.h>

#define PAUSE_OPTIONS_SPACING (2 * TILE_SIZE)

enum PauseOption
{
    PAUSE_OPTION_RESUME,
    PAUSE_OPTION_RUN_INFO,
    PAUSE_OPTION_QUIT_RUN,
    PAUSE_OPTION_MAX
};

enum PauseStage
{
    PAUSE_STAGE_CLOSED,
    PAUSE_STAGE_OPEN,
    // An option was chosen, it is applied once every key is released
    PAUSE_STAGE_CLOSING,
    // The Run Info page covers the screen
    PAUSE_STAGE_RUN_INFO,
    // The Run Info page was closed, the game resumes once every key is released
    PAUSE_STAGE_RESUMING
};

// Positions in pixels. The title and the options are centered vertically on the screen as a block.
#define PAUSE_TITLE_Y   48
#define PAUSE_OPTIONS_Y 72
#define PAUSE_NOTE_Y    128

// WHY: text can only be placed every 8 pixels, the width of a character. A name with an odd
// number of characters cannot be centered and ends up half a character to the left, so every
// name here, like the title, has an even number of characters.
static const char* const PAUSE_OPTION_NAMES[PAUSE_OPTION_MAX] = {
    [PAUSE_OPTION_RESUME] = "Resume",
    [PAUSE_OPTION_RUN_INFO] = "Run Info",
    [PAUSE_OPTION_QUIT_RUN] = "Back to Menu"
};

static enum PauseStage s_stage = PAUSE_STAGE_CLOSED;
static enum PauseOption s_selected_option = PAUSE_OPTION_RESUME;
static const PauseMenuHost* s_host = NULL;

/**
 * @brief Print @p text horizontally centered on the screen.
 */
static void pause_menu_print_centered(const char* text, int y, u8 text_color)
{
    int x = (SCREEN_WIDTH - (int)strlen(text) * TTE_CHAR_SIZE) / 2;

    tte_printf("#{P:%d,%d; cx:0x%X000}%s", x, y, text_color, text);
}

/**
 * @brief Print the reminder of the paused screen under the options, if it has one.
 */
static void pause_menu_print_note(void)
{
    if (s_host->get_note == NULL)
        return;

    const char* note = s_host->get_note();
    if (note != NULL)
        pause_menu_print_centered(note, PAUSE_NOTE_Y, TTE_RED_PB);
}

static void pause_menu_print(void)
{
    pause_menu_print_centered("PAUSED", PAUSE_TITLE_Y, TTE_WHITE_PB);

    // WHY: the selected option is only told apart by its color, the font has no arrow glyph
    for (int option = 0; option < PAUSE_OPTION_MAX; option++)
    {
        bool is_selected = (option == (int)s_selected_option);

        pause_menu_print_centered(
            PAUSE_OPTION_NAMES[option],
            PAUSE_OPTIONS_Y + option * PAUSE_OPTIONS_SPACING,
            is_selected ? TTE_YELLOW_PB : TTE_WHITE_PB
        );
    }

    pause_menu_print_note();
}

/**
 * @brief Replace the pause menu by the Run Info page.
 */
static void pause_menu_open_run_info(void)
{
    // WHY: the overlay stays open so that closing it later restores the text of the paused screen
    overlay_set_dimmed(false);
    s_host->hide_screen();

    run_info_show();
    s_stage = PAUSE_STAGE_RUN_INFO;
}

/**
 * @brief Close the Run Info page and go back to the paused screen.
 */
static void pause_menu_close_run_info(void)
{
    run_info_hide();

    s_host->show_screen();
    overlay_close();

    s_stage = PAUSE_STAGE_RESUMING;
}

static void pause_menu_finish_closing(void)
{
    if (!overlay_are_keys_released())
        return;

    if (s_selected_option == PAUSE_OPTION_RUN_INFO)
    {
        pause_menu_open_run_info();
        return;
    }

    overlay_close();
    s_stage = PAUSE_STAGE_CLOSED;

    if (s_selected_option == PAUSE_OPTION_QUIT_RUN)
        s_host->quit_to_main_menu();
}

static void pause_menu_choose(enum PauseOption option)
{
    s_selected_option = option;
    s_stage = PAUSE_STAGE_CLOSING;

    play_sfx(SFX_BUTTON, MM_BASE_PITCH_RATE, SFX_DEFAULT_VOLUME);
}

static void pause_menu_move_selection(int direction)
{
    s_selected_option = (s_selected_option + PAUSE_OPTION_MAX + direction) % PAUSE_OPTION_MAX;

    pause_menu_print();
}

static void pause_menu_process_input(void)
{
    if (key_hit(KEY_PAUSE_GAME | KEY_CANCEL))
        pause_menu_choose(PAUSE_OPTION_RESUME);
    else if (key_hit(KEY_CONFIRM))
        pause_menu_choose(s_selected_option);
    else if (key_hit(KEY_UP))
        pause_menu_move_selection(-1);
    else if (key_hit(KEY_DOWN))
        pause_menu_move_selection(1);
}

static void pause_menu_update_run_info(void)
{
    if (key_hit(KEY_PAUSE_GAME | KEY_CANCEL))
    {
        pause_menu_close_run_info();
        return;
    }

    run_info_update();
}

void pause_menu_open(const PauseMenuHost* host)
{
    if (s_stage != PAUSE_STAGE_CLOSED || host == NULL)
        return;

    s_host = host;
    s_selected_option = PAUSE_OPTION_RESUME;
    s_stage = PAUSE_STAGE_OPEN;

    overlay_open();
    pause_menu_print();
}

bool pause_menu_is_open(void)
{
    return s_stage != PAUSE_STAGE_CLOSED;
}

void pause_menu_update(void)
{
    switch (s_stage)
    {
        case PAUSE_STAGE_OPEN:
            pause_menu_process_input();
            break;

        case PAUSE_STAGE_CLOSING:
            pause_menu_finish_closing();
            break;

        case PAUSE_STAGE_RUN_INFO:
            pause_menu_update_run_info();
            break;

        case PAUSE_STAGE_RESUMING:
            if (overlay_are_keys_released())
                s_stage = PAUSE_STAGE_CLOSED;
            break;

        default:
            break;
    }
}
