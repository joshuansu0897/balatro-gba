/**
 * @file wiki_pictures.h
 *
 * @brief The picture of the Wiki entry on screen. At most one exists at a time.
 */
#ifndef GAME_WIKI_PICTURES_H
#define GAME_WIKI_PICTURES_H

#include "blind.h"
#include "item.h"
#include "skip_tag.h"

/**
 * @brief Show a Joker as the picture.
 *
 * @return the Joker, to read its name and description from, or NULL if it could not be created
 */
Item* wiki_pictures_show_joker(int joker_id);

void wiki_pictures_show_tag(enum SkipTagType tag_type);

void wiki_pictures_show_boss(enum BlindType blind);

/**
 * @brief Show a pack as the picture.
 *
 * @return the pack, to read its name from, or NULL if it could not be created
 */
Item* wiki_pictures_show_pack(enum ItemType pack_type);

/**
 * @brief Remove the picture, if there is one.
 */
void wiki_pictures_clear(void);

#endif // GAME_WIKI_PICTURES_H
