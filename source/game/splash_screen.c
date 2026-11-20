#include "splash_screen.h"

#include "game.h"
#include "graphic_utils.h"
#include "maxmod.h"
#include "soundbank.h"
#include "util.h"

#include <string.h>
#include <tonc.h>

// The message of this build, shown on its own screen after the disclaimer. Change it here:
// each line is centered. Keep a line to 28 characters, an even number centers exactly.
static const char* const CUSTOM_MESSAGE_LINES[] = {
    "Balatro GBA is made by the",
    "GBALATRO team.",
    "",
    "This is a personal build",
    "with a few extras added.",
};

// Tile row of the first line of the message
#define CUSTOM_MESSAGE_FIRST_ROW 7

// How long the message stays on screen when no key is pressed
#define CUSTOM_MESSAGE_DURATION_SECONDS 3
#define CUSTOM_MESSAGE_DURATION_FRAMES  (SPLASH_FPS * CUSTOM_MESSAGE_DURATION_SECONDS)

// WHY: the button that launched the game is often still held when it starts. Without this
// delay it skips the first screen before anything can be read.
#define SKIP_DELAY_FRAMES (SPLASH_FPS / 2)

enum SplashPage
{
    SPLASH_PAGE_DISCLAIMER,
    SPLASH_PAGE_CUSTOM_MESSAGE
};

static const Rect COUNTDOWN_TIMER_RECT = {208, 144, 240, 152};
static uint s_timer = 0;
static enum SplashPage s_page = SPLASH_PAGE_DISCLAIMER;

void splash_screen_on_init(void)
{
    s_timer = 0;
    s_page = SPLASH_PAGE_DISCLAIMER;

    tte_printf("#{P:72,8; cx:0x%X000}DISCLAIMER", TTE_WHITE_PB);
    tte_printf(
        "#{P:8,24; cx:0x%X000}This project is NOT endorsed \n by or affiliated with \n Playstack "
        "or "
        "LocalThunk.\n\n If you have paid for this, \n you have been scammed \n and should request "
        "a refund \n IMMEDIATELY. \n\n The only official place \n to obtain this is from: \n\n "
        "'github.com/\n  GBALATRO/balatro-gba'",
        TTE_WHITE_PB
    );
    tte_printf("#{P:8,144; cx:0x%X000}(Press any key to skip)", TTE_WHITE_PB);
}

static void splash_screen_show_custom_message(void)
{
    s_page = SPLASH_PAGE_CUSTOM_MESSAGE;
    s_timer = 0;

    tte_erase_screen();
    for (int line_idx = 0; line_idx < NUM_ELEM_IN_ARR(CUSTOM_MESSAGE_LINES); line_idx++)
    {
        const char* line = CUSTOM_MESSAGE_LINES[line_idx];
        int nb_chars_before = (SCREEN_WIDTH / TTE_CHAR_SIZE - (int)strlen(line)) / 2;
        int x = nb_chars_before * TTE_CHAR_SIZE;
        int y = (CUSTOM_MESSAGE_FIRST_ROW + line_idx) * TTE_CHAR_SIZE;

        tte_printf("#{P:%d,%d; cx:0x%X000}%s", x, y, TTE_WHITE_PB, line);
    }
    tte_printf("#{P:8,144; cx:0x%X000}(Press any key to skip)", TTE_WHITE_PB);
}

/**
 * @brief Play one frame of the page on screen: count its time and print the seconds left.
 *
 * @return true once the page is over, because its time is up or a key was pressed
 */
static bool splash_screen_is_page_over(uint duration_frames)
{
    s_timer++;
    if (s_timer >= duration_frames)
        return true;

    tte_erase_rect_wrapper(COUNTDOWN_TIMER_RECT);
    tte_printf(
        "#{P:%d,%d; cx:0x%X000}%d",
        COUNTDOWN_TIMER_RECT.left,
        COUNTDOWN_TIMER_RECT.top,
        TTE_WHITE_PB,
        1 + (duration_frames - s_timer) / SPLASH_FPS
    );

    bool can_be_skipped = (s_timer > SKIP_DELAY_FRAMES);

    return can_be_skipped && key_hit(KEY_ANY);
}

void splash_screen_on_update(void)
{
    if (s_page == SPLASH_PAGE_DISCLAIMER)
    {
        if (splash_screen_is_page_over(SPLASH_DURATION_FRAMES))
            splash_screen_show_custom_message();
        return;
    }

    if (!splash_screen_is_page_over(CUSTOM_MESSAGE_DURATION_FRAMES))
        return;

    game_change_state(GAME_STATE_MAIN_MENU);
    tte_erase_screen();
}

void splash_screen_on_exit(void)
{
    mmStart(MOD_MAIN_THEME, MM_PLAY_LOOP);
}
