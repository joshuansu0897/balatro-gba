/**
 * @file pause_menu.h
 *
 * @brief Pause overlay that freezes the game and offers to resume, see the Run Info page or leave
 * for the Main Menu.
 */
#ifndef GAME_PAUSE_MENU_H
#define GAME_PAUSE_MENU_H

#include <stdbool.h>

/**
 * @brief What the paused screen provides to the pause menu.
 */
typedef struct PauseMenuHost
{
    /**
     * @brief Called after the menu closed because "Main Menu" was chosen.
     */
    void (*quit_to_main_menu)(void);

    /**
     * @brief Hide the sprites of the paused screen before a full-screen page is drawn over it.
     */
    void (*hide_screen)(void);

    /**
     * @brief Draw the background and sprites of the paused screen again once that page is closed.
     * Its text is restored by the pause menu.
     */
    void (*show_screen)(void);
} PauseMenuHost;

/**
 * @brief Open the pause menu: dims the screen and replaces the on-screen text by the menu.
 *
 * Does nothing if the menu is already open.
 *
 * @param host the paused screen, must stay valid until the menu is closed
 */
void pause_menu_open(const PauseMenuHost* host);

/**
 * @brief Tell whether the pause menu is open, including while it waits to close.
 *
 * @return true if the game must stay frozen, false otherwise
 */
bool pause_menu_is_open(void);

/**
 * @brief Handle the pause menu inputs. Must be called every frame while the menu is open.
 */
void pause_menu_update(void);

#endif // GAME_PAUSE_MENU_H
