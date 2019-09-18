#include "Player.hpp"
#include "Model.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <tuple>

namespace ducks
{
int n, m;
vector<vector<vector<Model>>> speciesModels;
vector<vector<Model>> birdModels;
vector<ESpecies> specieGuess;

tuple<double, int> action;

Player::Player()
{
    n = 6;                   // state: type of birds
    m = 9;                   // emission: type of moves
    vector<vector<vector<Model>>> mdl(6);
    speciesModels = mdl; // Vector of model for each species
}

std::vector<int> getObsSeq(Bird bird)
{
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
    // => YES
    // How about learning the first (100 - numBirds) times and start
    // shooting the rest of the time (one shot for each bird).
    // => YES something like that
    // How about not shooting in the first round, and guess the birds
    // and that why collect models for each spiecy until I can atleast
    // predict if the bird is a stork to avoid shooting the stork in this environment
    // => ABSOLUTELY!!!

    // Things I know
    // Need a model for each bird
    // Will get max 100 observations from each bird (1 for each timestep in round)
    // unless bird is dead, then don't calculate anything

    if (pState.getRound() == 0 ) {
        vector<vector<vector<Model>>> mdl(6);
        speciesModels = mdl; 
    }
    

    int nBirds = pState.getNumBirds();
    int openSeason = 100 - nBirds;

    if (pState.getBird(0).getSeqLength() == 1)
    {
        // each bird will have openSeason nr of models.
        vector<vector<Model>> tmp(nBirds, vector<Model>(nBirds, Model(n, m)));
        birdModels = tmp;
        cerr << "Start Round " << pState.getRound() << endl;
    }

    // For each bird
    int victim = -1;
    double victimOdds = 0.7;
    int nextBirdMove = -1;
    for (int b = 0; b < nBirds; b++)
    {
        Bird bird = pState.getBird(b);
        int nObs = bird.getSeqLength();
        if (nObs > openSeason && bird.isAlive())
        {
            Model model(n, m);
            vector<int> O = getObsSeq(bird);
            model.estimate(O);

            // Collect models for bird.
            birdModels[b][nBirds - 100 + nObs] = model;

            double specieMax = 0;
            int specie = -1;
            if (pState.getRound() > 0)
            {
                double storkMax = 0;
                double storkTmp = 0;
                if (!speciesModels[ESpecies(SPECIES_BLACK_STORK)].empty())
                {
                    for (int i = 0; i < speciesModels[ESpecies(SPECIES_BLACK_STORK)].size(); i++)
                    {
                        for (int j = 0; j < speciesModels[ESpecies(SPECIES_BLACK_STORK)][i].size(); j++)
                        {
                            storkTmp = speciesModels[ESpecies(SPECIES_BLACK_STORK)][i][j].estimateEmissionSequence(O);
                            if (storkTmp > storkMax)
                                storkMax = storkTmp;
                        }
                    }

                    if (storkMax > 0.7)
                    {
                        cerr << "FOUND STORK" << endl;
                        return cDontShoot;
                    }
                }


                int likelyState = model.estimateStateSeq(O);

                action = model.getNextEmission(likelyState);

                if (get<0>(action) >= victimOdds)
                {
                    victimOdds = get<0>(action);
                    victim = b;
                    nextBirdMove = get<1>(action);
                }
            }
        }
    }

    //std::cerr << "Round: " << pState.getRound() << " action " << EMovement(nextBirdMove) << " victim " << victim <<endl;
    return cDontShoot;
    return Action(victim, EMovement(nextBirdMove));
}

std::vector<ESpecies> Player::guess(const GameState &pState, const Deadline &pDue)
{
    /*
     * Here you should write your clever algorithms to guess the species of each bird.
     * This skeleton makes no guesses, better safe than sorry!
     */
    specieGuess.resize(pState.getNumBirds(), SPECIES_UNKNOWN);
    

    if (pState.getRound() == 0)
    {
        for (int i = 0; i < pState.getNumBirds(); i++)
        {
            specieGuess[i] = ESpecies(rand() % COUNT_SPECIES); 
        }
        return specieGuess;
    }

    for (int b = 0; b < pState.getNumBirds(); b++)
    {
        Bird bird = pState.getBird(b);
        vector<int> O = getObsSeq(bird);
        double specieMaxP = 0;
        int specie = -1;
        double tmpSpecieMaxP = 0;
        if (pState.getRound() > 0)
        {
            for (int i = 0; i < COUNT_SPECIES; i++)
            {
                if (speciesModels[i].empty())
                    continue;
                for (int j = 0; j < speciesModels[i].size(); j++)
                {
                    if (speciesModels[i][j].empty())
                        continue;

                    for (int k = 0; k < speciesModels[i][j].size(); k++)
                    {
                        tmpSpecieMaxP = speciesModels[ESpecies(i)][j][k].estimateEmissionSequence(O);
                        if (specieMaxP < tmpSpecieMaxP && !isinf(tmpSpecieMaxP))
                        {
                            specieMaxP = tmpSpecieMaxP;
                            specie = ESpecies(i);
                        }
                    }
                }
            }
            specieGuess[b] = ESpecies(specie);
        }
    }
    return specieGuess;
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
    for (int i = 0; i < pSpecies.size(); i++)
    {
        // Add all hmm from the round to this specie
        // pSpecies[i] = specie of bird i in this round
        // species[i][j] = models for specie j in round i

        if (pSpecies[i] == ESpecies(SPECIES_BLACK_STORK))
            cerr << "put in stork, bird nr: " << i << endl;

        speciesModels[pSpecies[i]].push_back(birdModels[i]);
    }
}

} /*namespace ducks*/
