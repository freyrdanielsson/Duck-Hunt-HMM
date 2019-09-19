#ifndef _DUCKS_MODEL_HPP_
#define _DUCKS_MODEL_HPP_

#include <vector>
#include <iostream>
#include <cmath>
#include <tuple>

using namespace std;

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
    vector<vector<double>> A;
    vector<vector<double>> B;
    vector<double> Pi;
    vector<int> O;

    vector<vector<double>> Alpha;
    vector<vector<double>> Beta;
    vector<vector<vector<double>>> diGamma;
    vector<vector<double>> Gamma;
    vector<double> C;

    int n, m, l;

    void alphaPass()
    {
        // initialize alpha
        this->C[0] = 0;
        for (int i = 0; i < n; i++)
        {
            this->Alpha[i][0] = this->Pi[i] * this->B[i][O[0]];
            this->C[0] += this->Alpha[i][0];
        }

        // Scale
        this->C[0] = 1 / this->C[0];
        for (int i = 0; i < n; i++)
        {
            this->Alpha[i][0] *= this->C[0];
        }

        // finish alpha-pass
        for (int k = 1; k < l; k++)
        {
            this->C[k] = 0;
            for (int i = 0; i < n; i++)
            {
                this->Alpha[i][k] = 0;
                for (int j = 0; j < n; j++)
                {
                    this->Alpha[i][k] += this->Alpha[j][k - 1] * A[j][i];
                }
                this->Alpha[i][k] *= this->B[i][O[k]];
                this->C[k] += this->Alpha[i][k];
            }
            // Scale
            this->C[k] = 1 / this->C[k];
            for (int i = 0; i < n; i++)
            {
                this->Alpha[i][k] *= this->C[k];
            }
        }
    }

    void betaPass()
    {
        // usually Beta[i,T-1] = 1 but we scale.
        for (int i = 0; i < n; i++)
        {
            this->Beta[i][l - 1] = this->C[l - 1];
        }

        // finish beta-pass
        for (int k = l - 2; k >= 0; k--)
        {
            for (int i = 0; i < n; i++)
            {
                this->Beta[i][k] = 0;
                for (int j = 0; j < n; j++)
                {
                    // Scale w same C as aplha
                    this->Beta[i][k] += (this->A[i][j] * B[j][O[k + 1]] * this->Beta[j][k + 1]) * this->C[k];
                }
            }
        }
    }

    void diGammaPass()
    {
        // compute edge case
        for (int i = 0; i < n; i++)
        {
            this->Gamma[i][l - 1] = this->Alpha[i][l - 1];
        }

        for (int k = 0; k < l - 1; k++)
        {
            for (int i = 0; i < n; i++)
            {
                this->Gamma[i][k] = 0;
                for (int j = 0; j < n; j++)
                {
                    this->diGamma[i][j][k] = this->Alpha[i][k] * this->A[i][j] * this->B[j][O[k + 1]] * this->Beta[j][k + 1];
                    Gamma[i][k] += this->diGamma[i][j][k];
                }
            }
        }
    }

public:
    Model(int nState, int nEmission)
    {
        this->n = nState;
        this->m = nEmission;
        this->A.resize(n, vector<double>(n));
        this->B.resize(n, vector<double>(m));
        this->Pi.resize(n);

        double sum;

        // Initialize transition matrix, random normalized values
        for (int i = 0; i < nState; i++)
        {
            sum = 0;
            for (int j = 0; j < nState; j++)
            {
                double randDouble = (double)rand() / RAND_MAX;
                sum += randDouble;
                this->A[i][j] = randDouble;
            }
            // Normalize
            for (int j = 0; j < nState; j++)
            {
                this->A[i][j] /= sum;
            }
        }

        // Initialize emission matrix, random normalized values
        for (int i = 0; i < n; i++)
        {
            sum = 0;
            for (int j = 0; j < m; j++)
            {
                double randDouble = (double)rand() / RAND_MAX;
                sum += randDouble;
                this->B[i][j] = randDouble;
            }
            // Normalize
            for (int j = 0; j < m; j++)
            {
                this->B[i][j] /= sum;
            }
        }

        // Initialize initial state dist matrix
        sum = 0;
        for (int i = 0; i < n; i++)
        {
            double randDouble = (double)rand() / RAND_MAX;
            sum += randDouble;
            this->Pi[i] = randDouble;
        }

        for (int i = 0; i < n; i++)
        {
            this->Pi[i] /= sum;
        }
    }

    double estimateEmissionSequence(vector<int> &pO)
    {

        vector<double> Alphan(n);
        for (int i = 0; i < n; i++)
        {
            Alphan[i] = this->Pi[i] * this->B[i][pO[0]];
        }

        vector<double> AlphanTmp(n);
        for (int k = 1; k < pO.size(); k++)
        {
            for (int i = 0; i < n; i++)
            {
                double tmp = 0;
                for (int j = 0; j < n; j++)
                {
                    tmp += Alphan[j] * this->A[j][i];
                }
                AlphanTmp[i] = tmp * this->B[i][pO[k]];
            }
            for (int i = 0; i < n; i++)
            {
                Alphan[i] = AlphanTmp[i];
            }
        }

        double marginalize = 0;
        for (int i = 0; i < n; i++)
        {
            marginalize += Alphan[i];
        }

        //cerr << marginalize << endl;
        return marginalize;
    }

    /**
     * NOTE: Really just need the last state?
     * 
     * Estimate state sequence for given observation sequence
     * using current model
     * 
     * @param O observation sequence
     * @return estimation of state sequence that produced the observation sequence
    */
    int estimateStateSeq(vector<int> &pO)
    {

        //Bætti við const og type a þessa linu bæbæ.

        int l = pO.size();
        // initialize Delta[0]
        double max = 0;
        int argmax = -1;

        vector<double> Delta(n);
        for (int i = 0; i < n; i++)
        {
            Delta[i] = this->Pi[i] * B[i][pO[0]];
        }

        double prob = 0;
        vector<double> DeltaTmp(n);
        vector<vector<int>> DeltaStates(n, vector<int>(l));
        for (int k = 1; k < l; k++)
        {
            for (int i = 0; i < n; i++)
            {
                max = 0;
                argmax = -1;
                for (int j = 0; j < n; j++)
                {
                    prob = Delta[j] * A[j][i] * B[i][pO[k]];
                    if (max < prob && !isinf(prob)) // NOTE
                    {
                        max = prob;
                        argmax = j;
                    }
                }
                DeltaTmp[i] = max;          // Most likely current state
                DeltaStates[i][k] = argmax; // Most likely previous state
            }
            for (int i = 0; i < n; i++)
            {
                Delta[i] = DeltaTmp[i];
            }
        }

        max = 0;
        argmax = -1;

        for (int i = 0; i < n; i++)
        {
            if (max <= Delta[i])
            {
                max = Delta[i];
                argmax = i; // Most likely current state (at t=T)
            }
        }
        return argmax;
    }

    tuple<double, int> getNextEmission(int state) const
    {
        vector<double> emissionProb(m);

        for (int k = 0; k < m; k++)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    emissionProb[k] += A[j][i] * B[i][k] * Alpha[j][O.size() - 1];
                }
            }
        }

        double max = 0;
        int emission = -1;
        for (int k = 0; k < m; k++)
        {
            if (max < emissionProb[k])
            {
                max = emissionProb[k];
                emission = k;
            }
        }
        //cerr << max << endl;
        return make_tuple(max, emission);
    }

    // Common log scaling.
    double logScale()
    {
        double logP = 0;
        for (int k = 0; k < m; k++)
        {
            logP += log(C[k]);
        }
        return -logP;
    }

    /**
     * Baum-Welch Algorithm
     */
    void estimate(vector<int> &pO)
    {
        O = pO;
        l = pO.size();

        this->C.resize(l);
        this->Alpha.resize(n, vector<double>(l));
        this->Beta.resize(n, vector<double>(l));
        this->diGamma.resize(n, vector<vector<double>>(n, vector<double>(l)));
        this->Gamma.resize(n, vector<double>(l));

        //double oldProb = '-inf';
        for (int t = 0; t < 10; t++)
        {
            this->alphaPass();
            this->betaPass();
            this->diGammaPass();
            double oldProb = logScale();

            // estimate initial distribution
            for (int i = 0; i < n; i++)
            {
                this->Pi[i] = this->Gamma[i][0];
            }

            // estimate transition matrix
            for (int i = 0; i < n; i++)
            {
                double d = 0;
                for (int k = 0; k < l - 1; k++)
                {
                    d += this->Gamma[i][k];
                }
                for (int j = 0; j < n; j++)
                {
                    double num = 0;
                    for (int k = 0; k < l - 1; k++)
                    {
                        num += this->diGamma[i][j][k];
                    }
                    this->A[i][j] = num / d;
                }
            }

            // estimate emission matrix
            for (int i = 0; i < n; i++)
            {
                double d = 0;
                for (int k = 0; k < l; k++)
                {
                    d += this->Gamma[i][k];
                }
                for (int j = 0; j < m; j++)
                {
                    double num = 0;
                    for (int k = 0; k < l; k++)
                    {
                        num += O[k] == j ? this->Gamma[i][k] : 0;
                    }
                    this->B[i][j] = num / d;
                }
            }
            double newPr = logScale();
            if (newPr < oldProb)
                break; // break if not improving, could put some threshold aswell
            oldProb = newPr;
        }
    }

    // For debugging
    vector<vector<double>> getTransitionMatrix() const
    {
        return this->A;
    }

    vector<vector<double>> getEmissionMatrix() const
    {
        return this->B;
    }

    vector<double> getInitialDistMatrix() const
    {
        return this->Pi;
    }
};

} // namespace ducks

#endif
