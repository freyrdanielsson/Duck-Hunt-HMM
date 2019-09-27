#include "Player.hpp"
#include "Model.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <tuple>

using namespace std;

namespace ducks
{

int nStates, mEmission;
vector<vector<vector<Model>>> speciesModels;
vector<vector<Model>> birdModels;
vector<ESpecies> specieGuess(6);

Player::Player()
{
    nStates = 9;   // state: type of birds
    mEmission = 9; // emission: type of moves
    vector<vector<vector<Model>>> mdl(6);
    speciesModels = mdl; // Vector of model for each species
}

std::vector<int> getObsSeq(Bird bird)
{
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

    int nBirds = pState.getNumBirds();
    int openSeason = 100 - nBirds;

    if (pState.getBird(0).getSeqLength() == 1)
    {
        // each bird will have openSeason nr of models.
        vector<vector<Model>> tmp(nBirds, vector<Model>(nBirds, Model(nStates, mEmission)));
        birdModels = tmp;
        cerr << "Start Round " << pState.getRound() << endl;
    }

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

            Model model(nStates, mEmission);
            vector<int> O = getObsSeq(bird);

            vector<vector<double>> Alpha = model.estimate(O);

            // Collect models for bird.
            birdModels[b][nBirds - 100 + nObs] = model;

            /* if (pState.getRound() > 0)
            {
                vector<double> maxSpiece(COUNT_SPECIES);
                double norm = 0;

                int specie = -1;
                double bestOfAllS = 0;

                for (int i = 0; i < COUNT_SPECIES; i++)
                {
                    double specieMaxP = 0;
                    for (int j = 0; j < speciesModels[i].size(); j++)
                    {
                        for (int k = 0; k < speciesModels[i][j].size(); k++)
                        {
                            double tmpSpecieMaxP = speciesModels[i][j][k].estimateEmissionSequence(O);
                            if (specieMaxP < tmpSpecieMaxP && !isinf(tmpSpecieMaxP))
                            {
                                specieMaxP = tmpSpecieMaxP;
                            }
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

                if (speciesModels[SPECIES_BLACK_STORK].empty() || specie == SPECIES_BLACK_STORK || (maxSpiece[SPECIES_BLACK_STORK] > 0.1))
                {
                    continue;
                }

                int likelyState = model.estimateStateSeq(O);

                tuple<double, int> action = model.getNextEmission(Alpha);

                if (get<0>(action) >= victimOdds)
                {
                    victimOdds = get<0>(action);
                    victim = b;
                    nextBirdMove = get<1>(action);
                }
            } */
        }
    }
    return cDontShoot;
    //std::cerr << "Round: " << pState.getRound() << " action " << EMovement(nextBirdMove) << " victim " << victim <<endl;

    //return Action(victim, EMovement(nextBirdMove));
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
            double specieMaxP = 0;
            for (int j = 0; j < speciesModels[i].size(); j++)
            {
                for (int k = 0; k < speciesModels[i][j].size(); k++)
                {

                    double tmpSpecieMaxP = speciesModels[i][j][k].estimateEmissionSequence(O);
                    //double tmpSpecieMaxP = 0;
                    //avg += tmpSpecieMaxP;
                    if (specieMaxP <= tmpSpecieMaxP && !isinf(tmpSpecieMaxP))
                    {
                        specieMaxP = tmpSpecieMaxP;
                    }
                }
            }
            //avgL[i] = avg / COUNT_SPECIES;
            maxSpiece[i] = specieMaxP;
            norm += specieMaxP;
        }

        /* for (int i = 0; i < COUNT_SPECIES; i++)
        {
            maxSpiece[i] /= norm;
        } */

        for (int i = 0; i < COUNT_SPECIES; i++)
        {
            if (bestOfAllS < maxSpiece[i])
            {
                bestOfAllS = maxSpiece[i];
                specie = i;
            }
        }

        cerr << "best " << specie << endl;
        for (int i = 0; i < COUNT_SPECIES; i++)
        {
            cerr << maxSpiece[i] << " ";
        }
        cerr << endl;

        specieGuess[b] = ESpecies(specie);
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

        if (pSpecies[i] == ESpecies(SPECIES_BLACK_STORK))
            cerr << "put in stork, bird nr: " << i << endl;

        if (pSpecies[i] < 0)
            continue;

        speciesModels[pSpecies[i]].push_back(birdModels[i]);
    }
}

} /*namespace ducks*/