/**
 * @file wiki.h
 *
 * @brief Wiki tab of the Run Info page: every Joker, Skip Tag and Boss Blind with its picture and
 * what it does.
 */
#ifndef GAME_WIKI_H
#define GAME_WIKI_H

/**
 * @brief Draw the Wiki over the content area of the Run Info page, on the entry last looked at.
 */
void wiki_show(void);

/**
 * @brief Handle user input: Left/Right change the category, Up/Down the entry.
 *
 * Must be called every frame while the Wiki is shown.
 */
void wiki_update(void);

/**
 * @brief Remove the picture of the entry on screen. Does nothing if the Wiki is not shown.
 */
void wiki_hide(void);

#endif // GAME_WIKI_H
