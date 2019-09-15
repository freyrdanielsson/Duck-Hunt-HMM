#include "Player.hpp"
#include <cstdlib>
#include <iostream>

namespace ducks
{

Player::Player()
{
    // Initialize model?
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
    
    // Keep learning in each round ? Prob not possible

    int nBirds = pState.getNumBirds();
    int openSeason = 100 - (nBirds + 10);

    // For each bird
    for (int b = 0; b < nBirds; b++)
    {
        Bird bird = pState.getBird(b);
        int nObs = bird.getSeqLength();

        // This bird is alive and it's openSeason => run the model!
        if (nObs >= openSeason && bird.isAlive())
        {
            // (1) Initialize model
            
        }
                
    }
    
    /*
     * Here you should write your clever algorithms to get the best action.
     * This skeleton never shoots.
     */
    

    // This line choose not to shoot
    //return cDontShoot;

    //This line would predict that bird 0 will move right and shoot at it
    return Action(0, MOVE_RIGHT);
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
