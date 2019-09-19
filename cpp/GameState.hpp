#ifndef _DUCKS_GAMESTATE_HPP_
#define _DUCKS_GAMESTATE_HPP_

#include "Bird.hpp"
#include <vector>

namespace ducks
{

/**
 * Represents a game state
 */
class GameState
{
public:
    ///returns what round we are currently playing
    int getRound() const
    {
        return mRound;
    }

    ///returns the number of birds
    size_t getNumBirds() const
    {
        return mBirds.size();
    }

    ///returns a reference to the i-th bird
    const Bird &getBird(int i) const
    {
        return mBirds[i];
    }

    ///returns the index of your player among all players
    int whoAmI() const
    {
        return mWhoIAm;
    }

    ///returns the number of players
    int getNumPlayers() const
    {
        return mScores.size();
    }

    ///returns your current score
    int myScore() const
    {
        return mScores[mWhoIAm];
    }

    ///returns the score of the i-th player
    int getScore(int i) const
    {
        return mScores[i];
    }

    ///returns the number of turns elapsed since last time Shoot was called.
    ///this is the amount of new data available for each bird
    int getNumNewTurns() const
    {
        return mNumNewTurns;
    }

    /**
     * The following methods are used by the Client class.
     * Don't use them yourself!
     */
    GameState(int pWhoIAm, int pNumPlayers)
        : mRound(-1)
        , mWhoIAm(pWhoIAm)
        , mNumNewTurns(0)
        , mScores(pNumPlayers, 0)
    {
    }

    void newRound(int pRound, int pNumBirds)
    {
        // Clear the sky and fill it with new birds
        mRound = pRound;
        mBirds.assign(pNumBirds, Bird());
        mNumNewTurns = 0;
    }

    void addMoves(std::vector<EMovement> pMoves)
    {
        for (size_t b = 0; b < mBirds.size(); ++b)
            mBirds[b].addObservation(pMoves[b]);
        mNumNewTurns += 1;
    }

    void setScores(std::vector<int> pScores)
    {
        mScores = pScores;
    }

    void resetNumNewTurns()
    {
        mNumNewTurns = 0;
    }

private:
    int mRound;
    int mWhoIAm;
    int mNumNewTurns;
    std::vector<Bird> mBirds;
    std::vector<int> mScores;
};

} /*namespace ducks*/

#endif
