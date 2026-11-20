/**
 * @file options_menu.h
 *
 * @brief Options menu state functions.
 */
#ifndef GAME_OPTIONS_MENU_H
#define GAME_OPTIONS_MENU_H

// Palette indices for colors that change at runtime (i.e. button outlines) start at 16 and are
// sorted by the position of the buttons in the menu from top to bottom, and from left to right.
// Indices for static colors (i.e. background and main button colors) range from 0 to 15.
// The indexing is determined by background_options_menu_gfx.png.
#define MENU_BUTTON_MAIN_COLOR_PAL_IDX          1
#define SAVE_BUTTON_MAIN_COLOR_PAL_IDX          3
#define BACK_BUTTON_MAIN_COLOR_PAL_IDX          4
#define SPEED_DOWN_BUTTON_OUTLINE_COLOR_PAL_IDX 16
#define SPEED_BUTTON_OUTLINE_COLOR_PAL_IDX      17
#define SPEED_UP_BUTTON_OUTLINE_COLOR_PAL_IDX   18
#define CONTRAST_BUTTON_OUTLINE_COLOR_PAL_IDX   19
#define READABLE_BUTTON_OUTLINE_COLOR_PAL_IDX   20
#define MUSIC_BUTTON_OUTLINE_COLOR_PAL_IDX      21
#define SOUND_BUTTON_OUTLINE_COLOR_PAL_IDX      22
#define SAVE_BUTTON_OUTLINE_COLOR_PAL_IDX       23
#define BACK_BUTTON_OUTLINE_COLOR_PAL_IDX       24
// WHY: added after the other outlines were numbered, renumbering them would mean repainting
// the whole background
#define BOSS_BUTTON_OUTLINE_COLOR_PAL_IDX       25

/**
 * @brief Change the options menu background
 */
void options_menu_change_background(void);

/**
 * @brief Options menu state initialization
 */
void options_menu_on_init(void);

/**
 * @brief Options menu state update
 */
void options_menu_on_update(void);

/**
 * @brief Options menu cleanup (called when going back to main menu)
 */
void options_menu_on_exit(void);

/**
 * @brief Save the options and go back to the main menu, what the Save button does.
 */
void options_menu_save(void);

/**
 * @brief Forget the changes and go back to the main menu, what the Cancel button does.
 */
void options_menu_cancel(void);

#endif // GAME_OPTIONS_MENU_H
