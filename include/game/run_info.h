/**
 * @file run_info.h
 *
 * @brief Run Info page: full-screen tabs with information about the current run (Hand Levels).
 */
#ifndef GAME_RUN_INFO_H
#define GAME_RUN_INFO_H

/**
 * @brief Open the Run Info page. Will erase and then take the whole screen for itself.
 */
void run_info_show(void);

/**
 * @brief Handle user input and change the Hands shown or the Tab accordingly.
 *
 * Must be called every frame while the page is open.
 */
void run_info_update(void);

/**
 * @brief Hide the Run Info page. The screen it covered has to be drawn again by the caller.
 */
void run_info_hide(void);

#endif // GAME_RUN_INFO_H
