#include "Player.hpp"
#include "Model.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <tuple>


namespace ducks
{
int n, m;
Player::Player()
{
    n = 6; // state: type of birds
    m = 9; // emission: type of moves
}

std::vector<int> getObsSeq(Bird bird) {
        std::vector<int> O(bird.getSeqLength());

        for (int i = 0; i < bird.getSeqLength(); i++)
        {
            O[i] = bird.wasAlive(i) ? bird.getObservation(i) : 0; // check later...
        }

        return O;
    }

Action Player::shoot(const GameState &pState, const Deadline &pDue)
{
    // TOTHINK
    // How does deadline look, what is is usefull for?
    // Should I start shooting like crazy when the deadline is close to end
    // How about learning the first (deadline - numBirds) times and start
    // shooting the rest of the time (one shot for each bird).
    // Each missed shot gives a penalty.

    // Things I know
    // Need a model for each bird
    // Will get max 100 observations from each bird (1 for each timestep in round)
    // unless bird is dead, then don't calculate anything
    

    int nBirds = pState.getNumBirds();
    int openSeason = 100 - (nBirds + 1);

    // For each bird
    int victim = -1;
    double victimOdds = 0.75;
    int nextBirdMove = -1;
    for (int b = 0; b < nBirds; b++)
    {
        Bird bird = pState.getBird(b);
        int nObs = bird.getSeqLength();
        if (nObs >= openSeason && bird.isAlive())
        {
            Model model(n, m);
            model.estimate(getObsSeq(bird));
            vector<int> stateSeq = model.estimateStateSeq(getObsSeq(bird));
            tuple<double, int> action = model.getNextEmission(stateSeq[stateSeq.size() - 1]);
            if (get<0>(action) >= victimOdds) {
                victimOdds = get<0>(action);
                victim = b;
                nextBirdMove = get<1>(action);
            }
        }
    }

    if (victim == -1 && nextBirdMove == -1) {
        //std::cerr << "no shoot, becauae bad prediction" << endl;
    }

    //std::cerr << "Round: " << pState.getRound() << " action " << EMovement(nextBirdMove) << " victim " << victim <<endl;
    return Action(victim, EMovement(nextBirdMove));
}

std::vector<ESpecies> Player::guess(const GameState &pState, const Deadline &pDue)
{
    /*
     * Here you should write your clever algorithms to guess the species of each bird.
     * This skeleton makes no guesses, better safe than sorry!
     */

    std::vector<ESpecies> lGuesses(pState.getNumBirds(), SPECIES_UNKNOWN);
    return lGuesses;
}

void Player::hit(const GameState &pState, int pBird, const Deadline &pDue)
{
    /*
     * If you hit the bird you are trying to shoot, you will be notified through this function.
     */
    std::cerr << "HIT BIRD!!!" << std::endl;
}

void Player::reveal(const GameState &pState, const std::vector<ESpecies> &pSpecies, const Deadline &pDue)
{
    /*
     * If you made any guesses, you will find out the true species of those birds in this function.
     */
}


} /*namespace ducks*/
