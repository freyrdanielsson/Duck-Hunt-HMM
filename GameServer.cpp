#include "GameServer.hpp"
#include "Deadline.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

extern bool gVerbose;

namespace ducks
{

// The communications protocol looks like this:
//
// GAME <PlayerID> <NumPlayers>
// ROUND <Round> <NumBirds>
// MOVES <NumObservations>
// <Observation of bird 0> <Observation of bird 1> ...
// <Observation of bird 0> <Observation of bird 1> ...
// ...
// SHOOT <Deadline>
// --wait for response--
// HIT <Bird> <Deadline> (bird = -1 if miss)
// ...
// GUESS <Deadline>
// --wait for response--
// REVEAL <specie of bird 0> <specie of bird 1> ... <Deadline>
// SCORE <Score of player 0> <Score of player 1> ..

// TIMEOUT (happens when player fails to meet the deadline)


GameServer::SPlayer::SPlayer(std::istream &pInputStream, std::ostream &pOutputStream, int pID)
    : mInputStream(&pInputStream)
    , mOutputStream(&pOutputStream)
    , mID(pID)
    , mNumSent(0)
    , mScore(0)
    , mGameOver(false)
{
}

GameServer::GameServer(std::istream &pInputStream, std::ostream &pOutputStream)
    : mMaxRounds(2)
    , mMaxTurns(100)
    , mTimeForShoot(2000)
    , mTimeForHit(200)
    , mTimeForGuess(10000)
    , mTimeForReveal(1000)
{
    mPlayers.emplace_back(pInputStream, pOutputStream, mPlayers.size());
}

void GameServer::load(std::istream &pStream)
{
    // Parse the environment file

    // Read line by line
    std::string lString;
    if (getline(pStream, lString))
    {
        std::stringstream lS(lString);
        if (!(lS >> mMaxRounds))
            throw std::runtime_error("Failed to parse number of rounds in GameServer::load");
        if (mMaxRounds < 0)
            throw std::runtime_error("Negative number of rounds in GameServer::load");
        if (lS >> std::ws >> lString)
            throw std::runtime_error("Trailing data after number of rounds in GameServer::load");
    }
    else
        throw std::runtime_error("getline failed when reading number of rounds in GameServer::load");

    // Create rounds
    mEnvironment.assign(mMaxRounds, Round());

    // Parse rounds
    for (int r = 0; r < mMaxRounds; ++r)
    {
        Round &lRound = mEnvironment[r];
        int lNumBirds = 0;

        if (getline(pStream, lString))
        {
            std::stringstream lS(lString);
            if (!(lS >> lNumBirds))
                throw std::runtime_error("Failed to parse number of birds in GameServer::load");
            if (lNumBirds < 0)
                throw std::runtime_error("Negative number of birds in GameServer::load");
            if (lS >> std::ws >> lString)
                throw std::runtime_error("Trailing data after number of birds in GameServer::load");
        }
        else
            throw std::runtime_error("getline failed when reading number of birds in GameServer::load");

        // Create birds
        lRound.resize(lNumBirds);

        // Parse birds
        for (int b = 0; b < lNumBirds; ++b)
        {
            if (getline(pStream, lString))
            {
                std::stringstream lS(lString);
                // Read species
                int lSpecies;
                if (!(lS >> lSpecies))
                    throw std::runtime_error("Failed to parse species of bird in GameServer::load");

                // Set species
                lRound[b].mSpecies = ESpecies(lSpecies);

                // Read observations
                for (int i = 0; i < 100; ++i)
                {
                    int lObs;
                    if (!(lS >> lObs))
                        throw std::runtime_error("Failed to read observation of bird in GameServer::load");
                    lRound[b].mActions.emplace_back(EMovement(lObs));
                }
                if (lS >> std::ws >> lString)
                    throw std::runtime_error("Trailing data after 100 observations in GameServer::load");
            }
            else
                throw std::runtime_error("getline failed when reading bird in GameServer::load");
        }
    }
    if (pStream >> std::ws >> lString)
        throw std::runtime_error("Trailing data after all rounds in GameServer::load");
}

void GameServer::run()
{
    // Abort game if nothing is loaded
    if (mEnvironment.empty())
    {
        std::cerr << "No environment loaded" << std::endl;
		exit(-1);
	}

    if (gVerbose)
        std::cerr << "Starting game with " << mPlayers.size()
                << (mPlayers.size() == 1 ? " player" : " players") << std::endl;

    // Send start of game
    for (size_t i = 0; i < mPlayers.size(); ++i)
        *mPlayers[i].mOutputStream << "GAME " << i << " " << mPlayers.size() << std::endl;

    // The players take turns shooting
    int lActivePlayer = 0;

    // Play all rounds
    for (size_t r = 0; r < mEnvironment.size(); ++r)
    {
        // Generate birds for this round
        size_t lNumBirds = mEnvironment[r].size();
        mBirds.clear();
        mBirds.resize(lNumBirds);
        mBirdSpecies.resize(lNumBirds);
        for (size_t b = 0; b < lNumBirds; ++b)
        {
            // Find bird in current round
            const auto &lBird = mEnvironment[r][b];

            // Set species
            mBirdSpecies[b] = lBird.mSpecies;

            // Add the first observation
            mBirds[b].addObservation(lBird.mActions[0]);
        }

        if (gVerbose)
            std::cerr << "Starting round " << r << " with " << mBirds.size() << " birds" << std::endl;

        // Send start of round
        for (auto &lPlayer : mPlayers)
        {
            lPlayer.mNumSent = 0;
            sendRound(lPlayer, r);
        }

        // Let the players take turns shooting, start with one observation
        for (int i = 1; i < mMaxTurns; ++i)
        {
            if (gVerbose)
                std::cerr << "Updating birds" << std::endl;

            // Let the birds fly
            for (size_t b = 0; b < mEnvironment[r].size(); ++b)
                mBirds[b].addObservation(mEnvironment[r][b].mActions[i]);

            SPlayer &lPlayer = mPlayers[lActivePlayer];
            lActivePlayer = (lActivePlayer + 1) % mPlayers.size();

            // Send birds
            sendBirds(lPlayer);

            // Let the player shoot
            playerShoot(lPlayer);

            // Stop if we have no players left
            if (playersLeft() == 0)
                return;

            // End the round if all birds are dead
            bool lAnyAlive = false;
            for (auto &lBird : mBirds)
            {
                if (!lBird.isDead())
                {
                    lAnyAlive = true;
                    break;
                }
            }
            if (lAnyAlive == false)
                break;
        }
        // End of round

        // Send any trailing observations
        for (auto &lPlayer : mPlayers)
            sendBirds(lPlayer);

        // Send scores to all players
        for (auto &lPlayer : mPlayers)
            sendScores(lPlayer);

        // Let the player guess species
        for (auto &lPlayer : mPlayers)
            playerGuess(lPlayer);

        // Send scores to all players
        for (auto &lPlayer : mPlayers)
            sendScores(lPlayer);

        // Stop if we have no players left
        if (playersLeft() == 0)
            return;
    }

    if (gVerbose)
    {
        std::cerr << "Final scores:";
        for (const auto &lPlayer : mPlayers)
            std::cerr << " " << lPlayer.mScore;
        std::cerr << std::endl;
    }
}

void GameServer::playerShoot(SPlayer &pPlayer)
{
    if (pPlayer.mGameOver)
        return;

    // Ask the player to shoot
    Deadline lDue(mTimeForShoot);
    *pPlayer.mOutputStream << "SHOOT " << lDue.remainingMs() << std::endl;

    if (gVerbose)
        std::cerr << "Waiting for player to shoot" << std::endl;

    std::string lLine;
    if (!getline(*pPlayer.mInputStream, lLine))
    {
        std::cerr << "getline failed for player " << pPlayer.mID << std::endl;
        pPlayer.mGameOver = true;
        return;
    }

    if (gVerbose)
        std::cerr << "Got message from player: " << lLine << std::endl;

    if (lDue.remainingMs() < 0)
    {
        std::cerr << "Player " << pPlayer.mID << " timed out" << std::endl;
        *pPlayer.mOutputStream << "TIMEOUT" << std::endl;
        removePlayer(pPlayer);
        return;
    }

    // Parse the message
    std::istringstream lIn(lLine);
    int lBird, lMovement;
    lIn >> lBird >> lMovement;
    if (!lIn)
    {
        std::cerr << "Failed to parse action for player " << pPlayer.mID << std::endl;
        removePlayer(pPlayer);
        return;
    }

    if (lBird >= 0 && lBird < (int)mBirds.size())
    {
        EMovement lTrueMovement = mBirds[lBird].getLastObservation();
        if (lMovement == lTrueMovement && lMovement != MOVE_DEAD)
        {
            // Mark the bird as dead
            mBirds[lBird].kill();
            pPlayer.mScore += 1;
            ESpecies lSpecies = mBirdSpecies[lBird];
            if (lSpecies == SPECIES_BLACK_STORK)
            {
                // Hitting the black stork means disqualification
                pPlayer.mScore = 0;
                sendScores(pPlayer);
                *pPlayer.mOutputStream << "GAMEOVER" << std::endl;
                removePlayer(pPlayer);
                return;
            }

            // Tell the player that it hit the bird
            // The time is only measured in the client since we don't ask for a response
            *pPlayer.mOutputStream << "HIT " << lBird << " " << mTimeForHit << std::endl;
        }
        else
        {
            pPlayer.mScore -= 1;
        }
    }
}

void GameServer::playerGuess(SPlayer &pPlayer)
{
    if (pPlayer.mGameOver)
        return;

    // Ask the player to guess
    Deadline lDue(mTimeForGuess);
    *pPlayer.mOutputStream << "GUESS " << lDue.remainingMs() << std::endl;

    if (gVerbose)
        std::cerr << "Waiting for player to guess" << std::endl;

    std::string lString;
    if (!getline(*pPlayer.mInputStream, lString))
    {
        std::cerr << "getline failed for player " << pPlayer.mID << std::endl;
        pPlayer.mGameOver = true;
        return;
    }

    if (gVerbose)
        std::cerr << "Got message from player: " << lString << std::endl;

    if (lDue.remainingMs() < 0)
    {
        std::cerr << "Player " << pPlayer.mID << " timed out" << std::endl;
        *pPlayer.mOutputStream << "TIMEOUT" << std::endl;
        removePlayer(pPlayer);
        return;
    }

    // Parse the message and score the guesses
    int lScore = 0;
    std::istringstream lIn(lString);
    std::vector<int> lRevealing(mBirds.size(), SPECIES_UNKNOWN);
    bool lDoReveal = false;
    for (size_t i = 0; i < mBirds.size(); ++i)
    {
        int lGuessedSpecies = -1;
        if (!(lIn >> lGuessedSpecies))
        {
            std::cerr << "Failed to read guess from player for bird " << i << std::endl;
            pPlayer.mGameOver = true;
            return;
        }

        if (lGuessedSpecies == SPECIES_UNKNOWN)
            continue;

        int lTrueSpecies = mBirdSpecies[i];
        if (lTrueSpecies == lGuessedSpecies)
            lScore += 1;
        else
            lScore -= 1;

        lRevealing[i] = lTrueSpecies;
        lDoReveal = true;
    }

    if (lIn >> std::ws >> lString)
    {
        std::cerr << "Trailing output when reading guess:\n" << lString << std::endl;
        pPlayer.mGameOver = true;
        return;
    }

    if (gVerbose)
        std::cerr << "Score for guessing: " << lScore << std::endl;

    pPlayer.mScore += lScore;

    // Reveal the true species of the birds to the player for the ones he/she made guessed for
    // if the player made any guesses
    if (lDoReveal)
    {
        *pPlayer.mOutputStream << "REVEAL";
        for (const auto& lSpecies : lRevealing)
            *pPlayer.mOutputStream << " " << lSpecies;
        *pPlayer.mOutputStream << " " << mTimeForReveal << std::endl;
        // The time is only measured in the client since we don't ask for a response
    }
}

void GameServer::removePlayer(SPlayer &pPlayer)
{
    pPlayer.mGameOver = true;
}

int GameServer::playersLeft()
{
    int lPlayersLeft = 0;
    for (const auto &p : mPlayers)
        if (!p.mGameOver)
            ++lPlayersLeft;
    return lPlayersLeft;
}

void GameServer::sendRound(SPlayer &pPlayer, int pRound)
{
    if (pPlayer.mGameOver)
        return;

    *pPlayer.mOutputStream << "ROUND " << pRound << " " << mBirds.size() << std::endl;
}

void GameServer::sendBirds(SPlayer &pPlayer)
{
    if (pPlayer.mGameOver)
        return;

    // Don't send the newest observation
    // It needs to be secret so we have something to check against when shooting
    int lToSend = mBirds[0].getSeqLength() - 1;

    // Abort if there are no new moves to send
    if (pPlayer.mNumSent >= lToSend)
        return;

    // Construct a message to send to the player
    std::ostringstream lStr;

    // Observations header
    lStr << "MOVES " << (lToSend - pPlayer.mNumSent) << "\n";

    // Observations
    for (; pPlayer.mNumSent < lToSend; ++pPlayer.mNumSent)
    {
        for (size_t i = 0; i < mBirds.size(); ++i)
            lStr << (int) mBirds[i].getObservation(pPlayer.mNumSent) << " ";
        lStr << "\n";
    }

    *pPlayer.mOutputStream << lStr.str() << std::flush;

    if (!pPlayer.mOutputStream)
    {
        std::cerr << "warning: error writing to player\n";
        removePlayer(pPlayer);
    }
}

void GameServer::sendScores(SPlayer &pPlayer)
{
    if (pPlayer.mGameOver)
        return;

    *pPlayer.mOutputStream << "SCORE";
    for (const auto &lPlayer : mPlayers)
        *pPlayer.mOutputStream << " " << lPlayer.mScore;
    *pPlayer.mOutputStream << std::endl;
}

} /*namespace ducks*/
