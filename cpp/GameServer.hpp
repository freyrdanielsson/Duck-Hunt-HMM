#ifndef _DUCKS_GAMESERVER_HPP_
#define _DUCKS_GAMESERVER_HPP_

#include "Deadline.hpp"
#include "Bird.hpp"
#include <vector>
#include <memory>
#include <iostream>

namespace ducks
{

class GameServer
{
    struct SPlayer
    {
        SPlayer(std::istream &pInputStream, std::ostream &pOutputStream, int pID);

        std::istream *mInputStream;
        std::ostream *mOutputStream;

        int mID;
        int mNumSent;
        int mScore;
        bool mGameOver;
    };

    struct BirdSequence
    {
        ESpecies mSpecies;
        std::vector<EMovement> mActions;
    };

    typedef std::vector<BirdSequence> Round;

public:
    GameServer(std::istream &pInputStream, std::ostream &pOutputStream);

    void load(std::istream &pStream);
    void run();
    void writeObservations(std::ostream &os);

private:
    void sendRound(SPlayer &pPlayer, int pRound);
    void sendBirds(SPlayer &pPlayer);
    void sendScores(SPlayer &pPlayer);
    void playerShoot(SPlayer &pPlayer);
    void playerGuess(SPlayer &pPlayer);
    void removePlayer(SPlayer &pPlayer);
    int playersLeft();

private:
    int mMaxRounds;
    int mMaxTurns;

    int64_t mTimeForShoot;
    int64_t mTimeForHit;
    int64_t mTimeForGuess;
    int64_t mTimeForReveal;

    std::vector<SPlayer> mPlayers;
    std::vector<Round> mEnvironment;
    std::vector<Bird> mBirds;
    std::vector<ESpecies> mBirdSpecies;
};

} /*namespace ducks*/

#endif
