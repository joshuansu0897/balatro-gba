/**
 * @file wiki_pictures.c
 *
 * @brief Wiki pictures implementation.
 */
#include "game/wiki_pictures.h"

#include "game.h"
#include "joker.h"
#include "list.h"
#include "pack.h"
#include "sprite.h"
#include "util.h"

#include <tonc.h>

// Positions in pixels
static const BG_POINT PICTURE_POS = {28, 46};
static const BG_POINT SMALL_PICTURE_POS = {36, 54};

static EWRAM_BSS JokerObject* s_joker_object;
static EWRAM_BSS SkipTag* s_skip_tag;
static EWRAM_BSS Sprite* s_blind_token;
static EWRAM_BSS Item* s_pack;

/**
 * @brief Put a picture in place.
 * WHY: a sprite object only reaches its place when the game updates it, which it does not do
 * while paused, so its sprite is put there directly
 */
static void wiki_pictures_place(SpriteObject* picture, BG_POINT pos)
{
    if (picture == NULL || picture->sprite == NULL)
        return;

    sprite_object_position(picture, pos.x, pos.y);
    sprite_position(picture->sprite, pos.x, pos.y);
}

Item* wiki_pictures_show_joker(int joker_id)
{
    // WHY: a description can depend on the state of its Joker, so a real one is created for it
    Joker* joker = joker_new((u8)joker_id);
    GBAL_RETURN_IF_NULL(joker, NULL);

    s_joker_object = joker_object_new(joker);
    if (s_joker_object == NULL)
    {
        joker_destroy(&joker);
        return NULL;
    }

    wiki_pictures_place((SpriteObject*)s_joker_object, PICTURE_POS);

    return (Item*)s_joker_object;
}

void wiki_pictures_show_tag(enum SkipTagType tag_type)
{
    // WHY: the layers up to this one are taken by the Tags the player owns
    int layer = OWNED_SKIP_TAG_STARTING_LAYER + list_get_len(get_owned_skip_tags());
    if (layer >= MAX_SKIP_TAGS)
        return;

    s_skip_tag = skip_tag_new(tag_type);
    skip_tag_set_sprite(s_skip_tag, layer);
    wiki_pictures_place((SpriteObject*)s_skip_tag, SMALL_PICTURE_POS);
}

void wiki_pictures_show_boss(enum BlindType blind)
{
    s_blind_token = blind_token_new(blind, PICTURE_POS.x, PICTURE_POS.y, BOSS_BLIND_TOKEN_LAYER);
}

Item* wiki_pictures_show_pack(enum ItemType pack_type)
{
    s_pack = pack_object_new(pack_type);
    wiki_pictures_place((SpriteObject*)s_pack, PICTURE_POS);

    return s_pack;
}

void wiki_pictures_clear(void)
{
    joker_object_destroy(&s_joker_object);
    skip_tag_destroy(&s_skip_tag);
    sprite_destroy(&s_blind_token);
    if (s_pack != NULL)
        item_dispose(&s_pack);
}
