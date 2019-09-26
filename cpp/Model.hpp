#ifndef _DUCKS_MODEL_HPP_
#define _DUCKS_MODEL_HPP_

#include <vector>
#include <iostream>
#include <cmath>
#include <tuple>

namespace ducks
{

class Model
{
    /*NOTES
    +   Instead of always feeding in whole observation seq
        just feed one obs at a time and apent it to a global obs vector?
        estimating will then just update for that new observation instead of starting over
    */

private:
    std::vector<std::vector<double>> A;
    std::vector<std::vector<double>> B;
    std::vector<double> Pi;

    int n, m, l;


public:
    Model(int nState, int nEmission);

    void alphaPass(std::vector<int> &O, std::vector<std::vector<double>> &Alpha, std::vector<double> &C);

    void betaPass(std::vector<int> &O, std::vector<std::vector<double>> &Beta, std::vector<double> &C);

    void diGammaPass(std::vector<int> &O, std::vector<std::vector<double>> &Alpha, std::vector<std::vector<double>> &Beta,
                     std::vector<std::vector<std::vector<double>>> &diGamma, std::vector<std::vector<double>> &Gamma);

    /** 
     * Estimate the probability that the current model
     * produced given observation sequence
     * 
     * @param O observation sequence
     * @return probability that current model produced the observation sequence
    */
    double estimateEmissionSequence(std::vector<int> &O);

    /**
     * Estimate state sequence for given observation sequence
     * using current model
     * 
     * @param O observation sequence
     * @return Most likely given the observation sequence
    */
    int estimateStateSeq(std::vector<int> &O);

    std::tuple<double, int> getNextEmission(std::vector<std::vector<double>> Alpha) const;

    /**
     * Common log scaling
     */
    double logScale(std::vector<double> C) const;

    /**
     * Baum-Welch Algorithm
     */
    std::vector<std::vector<double>> estimate(std::vector<int> &O);

    std::vector<std::vector<double>> getTransitionMatrix() const;

    std::vector<std::vector<double>> getEmissionMatrix() const;

    std::vector<double> getInitialDistMatrix() const;
};

} // namespace ducks

#endif
