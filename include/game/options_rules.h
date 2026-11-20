/**
 * @file options_rules.h
 *
 * @brief The Rules page of the Options menu: switches for the Boss Blind effects and for what
 *        the Shop sells.
 *
 * It shares the Save and Cancel buttons at the bottom of the menu with the Display page.
 */
#ifndef GAME_OPTIONS_RULES_H
#define GAME_OPTIONS_RULES_H

/**
 * @brief Draw the Rules page over the Display page, with the cursor on its first switch.
 */
void options_rules_show(void);

/**
 * @brief Handle the inputs of the Rules page, to call once per frame while it is shown.
 */
void options_rules_update(void);

#endif // GAME_OPTIONS_RULES_H
