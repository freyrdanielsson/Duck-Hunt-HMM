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
        C[0] = 0;
        for (int i = 0; i < n; i++)
        {
            Alpha[i][0] = Pi[i] * B[i][O[0]];
            C[0] += Alpha[i][0];
        }

        // Scale
        C[0] = 1 / C[0];
        for (int i = 0; i < n; i++)
        {
            Alpha[i][0] *= C[0];
        }

        // finish alpha-pass
        for (int k = 1; k < l; k++)
        {
            C[k] = 0;
            for (int i = 0; i < n; i++)
            {
                Alpha[i][k] = 0;
                for (int j = 0; j < n; j++)
                {
                    Alpha[i][k] += Alpha[j][k - 1] * A[j][i];
                }
                Alpha[i][k] *= B[i][O[k]];
                C[k] += Alpha[i][k];
            }
            // Scale
            C[k] = 1 / C[k];
            for (int i = 0; i < n; i++)
            {
                Alpha[i][k] *= C[k];
            }
        }
    }

    void betaPass()
    {
        // usually Beta[i,T-1] = 1 but we scale.
        for (int i = 0; i < n; i++)
        {
            Beta[i][l - 1] = C[l - 1];
        }

        // finish beta-pass
        for (int k = l - 2; k >= 0; k--)
        {
            for (int i = 0; i < n; i++)
            {
                Beta[i][k] = 0;
                for (int j = 0; j < n; j++)
                {
                    // Scale w same C as aplha
                    Beta[i][k] += (A[i][j] * B[j][O[k + 1]] * Beta[j][k + 1]) * C[k];
                }
            }
        }
    }

    void diGammaPass()
    {
        // compute edge case
        for (int i = 0; i < n; i++)
        {
            Gamma[i][l - 1] = Alpha[i][l - 1];
        }

        for (int k = 0; k < l - 1; k++)
        {
            for (int i = 0; i < n; i++)
            {
                Gamma[i][k] = 0;
                for (int j = 0; j < n; j++)
                {
                    diGamma[i][j][k] = Alpha[i][k] * A[i][j] * B[j][O[k + 1]] * Beta[j][k + 1];
                    Gamma[i][k] += diGamma[i][j][k];
                }
            }
        }
    }

public:
    Model(int nState, int nEmission)
    {
        n = nState;
        m = nEmission;
        A.resize(n, vector<double>(n));
        B.resize(n, vector<double>(m));
        Pi.resize(n);

        double sum;

        // Initialize transition matrix, random normalized values
        for (int i = 0; i < nState; i++)
        {
            sum = 0;
            for (int j = 0; j < nState; j++)
            {
                double randDouble = (double)rand() / RAND_MAX;
                sum += randDouble;
                A[i][j] = randDouble;
            }
            // Normalize
            for (int j = 0; j < nState; j++)
            {
                A[i][j] /= sum;
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
                B[i][j] = randDouble;
            }
            // Normalize
            for (int j = 0; j < m; j++)
            {
                B[i][j] /= sum;
            }
        }

        // Initialize initial state dist matrix
        sum = 0;
        for (int i = 0; i < n; i++)
        {
            double randDouble = (double)rand() / RAND_MAX;
            sum += randDouble;
            Pi[i] = randDouble;
        }

        for (int i = 0; i < n; i++)
        {
            Pi[i] /= sum;
        }
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
    vector<int> estimateStateSeq(vector<int> pO)
    {
        O = pO;
        l = O.size();
        // initialize Delta[0]
        double max = 0;
        int argmax = -1;

        vector<double> Delta(n);
        for (int i = 0; i < n; i++)
        {
            Delta[i] = Pi[i] * B[i][O[0]];
        }

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
                    if (max < Delta[j] * A[j][i] * B[i][O[k]])
                    {
                        max = Delta[j] * A[j][i] * B[i][O[k]];
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
            if (max < Delta[i])
            {
                max = Delta[i];
                argmax = i; // Most likely current state (at t=T)
            }
        }

        vector<int> path(O.size());
        path[l - 1] = argmax;
        for (int i = l - 1; i > 0; i--)
        {
            path[i - 1] = DeltaStates[argmax][i]; // Put most likely previous state in path
            argmax = DeltaStates[argmax][i];      // Go to most likely previous state
        }

        return path;
    }

    tuple<double, int> getNextEmission(int state) const
    {
        vector<double> stateDist = A[state];
        vector<double> emissionProb(m);
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                for (int k = 0; k < m; k++)
                {
                    emissionProb[k] += A[j][i] * B[i][k] * stateDist[j];
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
        return make_tuple(max, emission);
    }

    /**
     * Baum-Welch Algorithm
     */
    void estimate(vector<int> pO)
    {
        O = pO;
        l = O.size();

        C.resize(l);
        Alpha.resize(n, vector<double>(l));
        Beta.resize(n, vector<double>(l));
        diGamma.resize(n, vector<vector<double>>(n, vector<double>(l)));
        Gamma.resize(n, vector<double>(l));


        for (int t = 0; t < 40; t++)
        {
            alphaPass();
            betaPass();
            diGammaPass();

            // estimate initial distribution
            for (int i = 0; i < n; i++)
            {
                Pi[i] = Gamma[i][0];
            }

            // estimate transition matrix
            for (int i = 0; i < n; i++)
            {
                double d = 0;
                for (int k = 0; k < l - 1; k++)
                {
                    d += Gamma[i][k];
                }
                for (int j = 0; j < n; j++)
                {
                    double num = 0;
                    for (int k = 0; k < l - 1; k++)
                    {
                        num += diGamma[i][j][k];
                    }
                    A[i][j] = num / d;
                }
            }

            // estimate emission matrix
            for (int i = 0; i < n; i++)
            {
                double d = 0;
                for (int k = 0; k < l; k++)
                {
                    d += Gamma[i][k];
                }
                for (int j = 0; j < m; j++)
                {
                    double num = 0;
                    for (int k = 0; k < l; k++)
                    {
                        num += O[k] == j ? Gamma[i][k] : 0;
                    }
                    B[i][j] = num / d;
                }
            }
        }
    }

    // For debugging
    vector<vector<double>> getTransitionMatrix() const
    {
        return A;
    }

    vector<vector<double>> getEmissionMatrix() const
    {
        return B;
    }

    vector<double> getInitialDistMatrix() const
    {
        return Pi;
    }
};

} // namespace ducks

#endif