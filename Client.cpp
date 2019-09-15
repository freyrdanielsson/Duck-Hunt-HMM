#include "Client.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>

extern bool gVerbose;

namespace ducks
{

Client::Client(Player &pPlayer, std::istream &pInputStream, std::ostream &pOutputStream)
    : mPlayer(pPlayer)
    , mState(0,1)
    , mInputStream(pInputStream)
    , mOutputStream(pOutputStream)
{
}

void Client::run()
{
    // Process messages until the stream is closed
    while (processMessage())
        ;

    if (gVerbose)
        std::cerr << "Final score: " << mState.myScore() << std::endl;
}

bool Client::processMessage()
{
    // Read message from stream
    std::string lString;
    if (!getline(mInputStream, lString))
        return false;

    // Parse message
    std::stringstream lMessage(lString);
    std::string lMessageType;
    if (!(lMessage >> lMessageType))
        throw std::runtime_error("Failed to read message type in readMessage for:\n" + lString);

    if (lMessageType == "GAME")
    {
        // Get my player id and how many players there are in total
        int lNumPlayers = 0;
        int lWhoIAm = 0;
        if (!(lMessage >> lWhoIAm >> lNumPlayers))
            throw std::runtime_error("Failed to parse GAME in readMessage");

        mState = GameState(lWhoIAm, lNumPlayers);
    }
    else if (lMessageType == "SCORE")
    {
        std::vector<int> lScores(mState.getNumPlayers(), 0);
        for (auto &lScore : lScores)
        {
            if (!(lMessage >> lScore))
                throw std::runtime_error("Failed to parse SCORE in readMessage");
        }
        mState.setScores(lScores);
        if (gVerbose)
            std::cerr << "My score: " << mState.myScore() << std::endl;
    }
    else if (lMessageType == "ROUND")
    {
        // Get current round and number of birds
        int lRound = 0;
        int lNumBirds = 0;
        if (!(lMessage >> lRound >> lNumBirds))
            throw std::runtime_error("Failed to parse ROUND in readMessage");

        mState.newRound(lRound, lNumBirds);
    }
    else if (lMessageType == "MOVES")
    {
        int lNumMoves = 0;
        if (!(lMessage >> lNumMoves))
            throw std::runtime_error("Failed to parse MOVES in readMessage");

        // Read moves line by line
        for (int i = 0; i < lNumMoves; ++i)
        {
            if (!getline(mInputStream, lString))
                throw std::runtime_error("getline failed while reading MOVES in readMessage");

            std::vector<EMovement> lMoves(mState.getNumBirds(), MOVE_DEAD);
            std::stringstream lMovesStream(lString);
            for (auto &lMove : lMoves)
            {
                int lMovement;
                if (!(lMovesStream >> lMovement))
                    throw std::runtime_error("Failed to read move for MOVES in readMessage");
                lMove = (EMovement)lMovement;
            }

            if (lMovesStream >> std::ws >> lString)
                throw std::runtime_error("Trailing input for MOVES data in readMessage");

            // Add the observed moves to the birds
            mState.addMoves(lMoves);
        }
    }
    else if (lMessageType == "SHOOT")
    {
        // Read deadline in milliseconds
        int lMs = 0;
        if (!(lMessage >> lMs))
            throw std::runtime_error("Failed to read deadline for SHOOT in readMessage");

        // Ask the player what to do
        Deadline lDue(lMs);
        Action lAction = mPlayer.shoot(mState, lDue);
        if (lDue.remainingMs() < 0)
            throw std::runtime_error("Player timed out during SHOOT");

        // Mark any new moves as processed
        mState.resetNumNewTurns();

        // Send response
        mOutputStream << lAction.getBirdNumber() << " " << lAction.getMovement() << std::endl;
    }
    else if (lMessageType == "GUESS")
    {
        // Read deadline in milliseconds
        int lMs = 0;
        if (!(lMessage >> lMs))
            throw std::runtime_error("Failed to read deadline for GUESS in readMessage");

        // Ask the player what to do
        Deadline lDue(lMs);
        std::vector<ESpecies> lGuesses = mPlayer.guess(mState, lDue);
        if (lDue.remainingMs() < 0)
            throw std::runtime_error("Player timed out during GUESS");

        if (lGuesses.size() != mState.getNumBirds())
            throw std::runtime_error("Player returned invalid number of birds in GUESS");

        // Mark any new moves as processed
        mState.resetNumNewTurns();

        // Send response
        for (const auto &g : lGuesses)
            mOutputStream << g << " ";
        mOutputStream << std::endl;
    }
    else if (lMessageType == "HIT")
    {
        // Read which bird we hit
        int lBird = -1;
        int lMs = 0;
        if (!(lMessage >> lBird >> lMs))
            throw std::runtime_error("Failed to read bird for HIT in readMessage");

        // Tell the player
        Deadline lDue(lMs);
        mPlayer.hit(mState, lBird, lDue);
        if (lDue.remainingMs() < 0)
            throw std::runtime_error("Player timed out during HIT");
    }
    else if (lMessageType == "REVEAL")
    {
        // Read the species of the birds
        std::vector<ESpecies> lRevealedSpecies(mState.getNumBirds(), SPECIES_UNKNOWN);
        for (size_t i = 0; i < mState.getNumBirds(); ++i)
        {
            int lSpecies;
            if (!(lMessage >> lSpecies))
                throw std::runtime_error("Failed to read species for REVEAL in readMessage");
            lRevealedSpecies[i] = ESpecies(lSpecies);
        }

        int lMs = 0;
        if (!(lMessage >> lMs))
            throw std::runtime_error("Failed to read deadline for REVEAL in readMessage");

        // Tell the player
        Deadline lDue(lMs);
        mPlayer.reveal(mState, lRevealedSpecies, lDue);
        if (lDue.remainingMs() < 0)
            throw std::runtime_error("Player timed out during REVEAL");
    }
    else if (lMessageType == "TIMEOUT")
    {
        throw std::runtime_error("Received GAMEOVER from server");
    }
    else if (lMessageType == "GAMEOVER")
    {
        if (gVerbose)
            std::cerr << "Received GAMEOVER from server" << std::endl;
        return false;
    }
    else
    {
        throw std::runtime_error("Failed to parse message in readMessage:\n" + lString);
    }

    if (lMessage >> std::ws >> lString)
        throw std::runtime_error("Trailing input for " + lMessageType + " in readMessage");

    // Return false if the stream is broken or closed
    return true;
}

} /*namespace ducks*/
