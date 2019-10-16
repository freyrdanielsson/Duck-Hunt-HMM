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
    n = 9; // state: type of birds
    m = 9; // emission: type of moves
    vector<vector<vector<Model>>> mdl(6);
    speciesModels = mdl; // Vector of model for each species
}

std::vector<int> getObsSeq(Bird bird)
{
    /* std::vector<int> O(bird.getSeqLength());

    for (int i = 0; i < bird.getSeqLength(); i++)
    {
        O[i] = bird.wasAlive(i) ? bird.getObservation(i) : 0; // check later...
    }

    return O; */

    int cnt = 0;
    for (int i = 0; i < bird.getSeqLength(); i++)
    {
        if (bird.wasAlive(i))
            cnt++;
    }
    std::vector<int> O(cnt);
    for (int i = 0; i < cnt; i++)
    {
        O[i] = bird.getObservation(i);
    }
    return O;
}

Action Player::shoot(const GameState &pState, const Deadline &pDue)
{

    if (pState.getRound() == 0)
    {
        speciesModels.clear();
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
    double victimOdds = 0.70;
    int nextBirdMove = -1;
    for (int b = 0; b < nBirds; b++)
    {
        bool couldBeStork = false;
        Bird bird = pState.getBird(b);
        int nObs = bird.getSeqLength();

        if (nObs > openSeason)
        {
            Model model(n, m);
            vector<int> O = getObsSeq(bird);
            model.estimate(O);
            birdModels[b][nObs % openSeason] = model;

            // If bird is alive try to shoot
            if (bird.isAlive())
            {
                for (int j = 0; j < speciesModels[SPECIES_BLACK_STORK].size(); j++)
                {
                    for (int k = 0; k < speciesModels[SPECIES_BLACK_STORK][j].size(); k++)
                    {
                        double tmpSpecieMaxP = speciesModels[SPECIES_BLACK_STORK][j][k].estimateEmissionSequence(O);
                        if (tmpSpecieMaxP > 0)
                        {
                            couldBeStork = true;
                            break;
                        }
                    }
                }

                if (couldBeStork) continue;

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
        vector<double> avgL(COUNT_SPECIES);
        vector<double> maxSpiece(COUNT_SPECIES);
        double norm = 0;

        int specie = -1;
        double bestOfAllS = 0;

        for (int i = 0; i < COUNT_SPECIES; i++)
        {
            bool timeout = false;
            double specieMaxP = 0;
            for (int j = 0; j < speciesModels[i].size(); j++)
            {
                if (timeout)
                    break;
                for (int k = 0; k < speciesModels[i][j].size(); k++)
                {
                    double tmpSpecieMaxP = speciesModels[i][j][k].estimateEmissionSequence(O);
                    if (specieMaxP < tmpSpecieMaxP)
                    {
                        specieMaxP = tmpSpecieMaxP;
                    }
                }
                if (pDue.remainingMs() < 100)
                {
                    for (int k = b; k < pState.getNumBirds(); k++)
                    {
                        specieGuess[k] = ESpecies(rand() % COUNT_SPECIES);
                    }
                    cerr << "TIMEOUT TIMEOUT" << endl;
                    return specieGuess;
                }
            }
            maxSpiece[i] = specieMaxP;
            norm += specieMaxP;
        }

        for (int i = 0; i < COUNT_SPECIES; i++)
        {
            maxSpiece[i] /= norm;
        }

        for (int i = 0; i < COUNT_SPECIES; i++)
        {
            if (bestOfAllS < maxSpiece[i])
            {
                bestOfAllS = maxSpiece[i];
                specie = i;
            }
        }

        /* cerr << "best " << specie << endl;
        for (int i = 0; i < COUNT_SPECIES; i++)
        {
            cerr << maxSpiece[i] << " ";
        }
        cerr << endl; */

        specieGuess[b] = ESpecies(specie);
    }
    cerr << pDue.remainingMs() << endl;
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

        if (pSpecies[i] < 0)
            continue;

        speciesModels[pSpecies[i]].push_back(birdModels[i]);
    }
}

} /*namespace ducks*/