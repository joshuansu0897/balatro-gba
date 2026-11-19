/**
 * @file wiki_entries.h
 *
 * @brief What the Wiki lists besides the Jokers: the Skip Tags and the Boss Blinds, with the
 * texts the game has nowhere else.
 */
#ifndef GAME_WIKI_ENTRIES_H
#define GAME_WIKI_ENTRIES_H

#include "blind.h"
#include "skip_tag.h"

typedef struct
{
    enum SkipTagType type;
    const char* name;
    const char* description;
} WikiTagEntry;

/**
 * @brief Get how many Skip Tags the Wiki lists, the ones the game can give.
 */
int wiki_entries_get_nb_tags(void);

/**
 * @brief Get a Skip Tag of the Wiki.
 *
 * @return the entry, or NULL if @p tag_idx is out of range
 */
const WikiTagEntry* wiki_entries_get_tag(int tag_idx);

/**
 * @brief Get how many Boss Blinds the Wiki lists, the ones that have an effect.
 */
int wiki_entries_get_nb_bosses(void);

/**
 * @brief Get a Boss Blind of the Wiki.
 *
 * @return the Blind, or BLIND_TYPE_MAX if @p boss_idx is out of range
 */
enum BlindType wiki_entries_get_boss(int boss_idx);

/**
 * @brief Get the text describing the effect of a Boss Blind, for any run.
 *
 * @return the text, or NULL if the Blind has no effect or the Boss effects are off
 */
const char* wiki_entries_get_boss_text(enum BlindType boss);

#endif // GAME_WIKI_ENTRIES_H
