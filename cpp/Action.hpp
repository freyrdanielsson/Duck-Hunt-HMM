#ifndef _DUCKS_ACTION_HPP_
#define _DUCKS_ACTION_HPP_

#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "Constants.hpp"

namespace ducks
{

/**
 * Identifies a bird, and its movement direction
 * This class is used for shooting at the birds
 */
class Action
{
public:
    /**
     * Construct a bird action object
     *
     * \param pBirdNumber the bird index
     * \param pMovement the movement of the bird
     */
    Action(int pBirdNumber, EMovement pMovement)
        : mBirdNumber(pBirdNumber)
        , mMovement(pMovement)
    {
    }

    ///the index of the bird this action corresponds too
    int getBirdNumber() const
    {
        return mBirdNumber;
    }

    ///the movement of the bird

    ///can be either BIRD_STOPPED or one of the eight basic directions
    EMovement getMovement() const
    {
        return mMovement;
    }

    ///represents a no-shoot action
    bool isDontShoot() const
    {
        return (mBirdNumber == -1);
    }

    ///prints the content of this action object
    std::string toString() const
    {
        if (isDontShoot())
            return "DONT SHOOT";
        else
        {
            std::stringstream ss;
            ss << mBirdNumber << " ";

            if (mMovement == MOVE_DEAD)
                ss << "DEAD";
            else
            {
                std::string move_names[] = {
                        "UP LEFT", "UP", "UP RIGHT",
                        "LEFT", "STOPPED", "RIGHT",
                        "DOWN LEFT", "DOWN", "DOWN RIGHT"};
                ss << move_names[mMovement];
            }
            return ss.str();
        }
    }

    bool operator==(const Action &pRH) const
    {
        return mBirdNumber == pRH.mBirdNumber && mMovement == pRH.mMovement;
    }

private:
    int mBirdNumber;
    EMovement mMovement;
};

///special Action object used to choose not to shoot
static const Action cDontShoot(-1, MOVE_DEAD);

} /*namespace ducks*/

#endif
