#ifndef _DUCKS_CONSTANTS_HPP_
#define _DUCKS_CONSTANTS_HPP_

namespace ducks
{

/**
 * This enum is used for representing the species of a bird
 */
enum ESpecies
{
    SPECIES_UNKNOWN = -1,    ///< the species is unknown
    SPECIES_PIGEON,
    SPECIES_RAVEN,
    SPECIES_SKYLARK,
    SPECIES_SWALLOW,
    SPECIES_SNIPE,
    SPECIES_BLACK_STORK,
    COUNT_SPECIES
};

/**
 * This enum is used for representing the movement observations of birds
 */
enum EMovement
{
    MOVE_DEAD = -1,
    MOVE_UP_LEFT,
    MOVE_UP,
    MOVE_UP_RIGHT,
    MOVE_LEFT,
    MOVE_STOPPED,
    MOVE_RIGHT,
    MOVE_DOWN_LEFT,
    MOVE_DOWN,
    MOVE_DOWN_RIGHT,
    COUNT_MOVE,
};

extern const char *DEFAULT_GAME_SEASON;

} /*namespace ducks*/

#endif
