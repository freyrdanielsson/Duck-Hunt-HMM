#include "Model.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <tuple>

using namespace std;

namespace ducks
{
vector<vector<double>> A;
vector<vector<double>> B;
vector<double> Pi;

int n, m, l;

Model::Model(int nState, int nEmission)
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

double Model::estimateEmissionSequence(vector<int> &O)
{
    vector<double> Alpha(n);
    for (int i = 0; i < n; i++)
    {
        //cerr << Pi[i] << " " << B[i][O[0]] << endl;
        Alpha[i] = Pi[i] * B[i][O[0]];
    }

    vector<double> AlphaTmp(n);
    for (int k = 1; k < O.size(); k++)
    {
        for (int i = 0; i < n; i++)
        {
            double tmp = 0;
            for (int j = 0; j < n; j++)
            {
                tmp += Alpha[j] * A[j][i];
            }
            AlphaTmp[i] = tmp * B[i][O[k]];
        }
        for (int i = 0; i < n; i++)
        {
            Alpha[i] = AlphaTmp[i];
        }
    }

    double marginalize = 0;
    for (int i = 0; i < n; i++)
    {
        marginalize += Alpha[i];
    }

    //cerr << marginalize << endl;
    return marginalize;
}

// Useless ?
int Model::estimateStateSeq(vector<int> &O)
{
    // initialize Delta[0]
    double max = 0;
    int argmax = -1;

    vector<double> Delta(n);
    for (int i = 0; i < n; i++)
    {
        Delta[i] = Pi[i] * B[i][O[0]];
    }

    double prob = 0;
    vector<double> DeltaTmp(n);
    vector<vector<int>> DeltaStates(n, vector<int>(O.size()));
    for (int k = 1; k < O.size(); k++)
    {
        for (int i = 0; i < n; i++)
        {
            max = 0;
            argmax = -1;
            for (int j = 0; j < n; j++)
            {
                prob = Delta[j] * A[j][i] * B[i][O[k]];
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

tuple<double, int> Model::getNextEmission(vector<vector<double>> Alpha) const
{
    vector<double> emissionProb(m);

    for (int k = 0; k < m; k++)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                emissionProb[k] += A[j][i] * B[i][k] * Alpha[j][l - 1];
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

double Model::logScale(vector<double> C) const
{
    double logP = 0;
    for (int k = 0; k < m; k++)
    {
        logP += log(C[k]);
    }
    return -logP;
}

vector<vector<double>> Model::estimate(vector<int> &O)
{
    l = O.size();

    vector<vector<double>> Alpha(n, vector<double>(l));
    vector<vector<double>> Beta(n, vector<double>(l));
    vector<vector<vector<double>>> diGamma(n, vector<vector<double>>(n, vector<double>(l)));
    vector<vector<double>> Gamma(n, vector<double>(l));
    vector<double> C(l);

    //double oldProb = '-inf';
    for (int t = 0; t < 10; t++)
    {
        alphaPass(O, Alpha, C);
        betaPass(O, Beta, C);
        diGammaPass(O, Alpha, Beta, diGamma, Gamma);
        double oldProb = logScale(C);

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
        double newPr = logScale(C);
        if (newPr < oldProb)
            break; // break if not improving, could put some threshold aswell
        oldProb = newPr;
    }

    return Alpha;
}

void Model::alphaPass(vector<int> &O, vector<vector<double>> &Alpha, vector<double> &C)
{
    // initialize alpha
    C[0] = 0;
    for (int i = 0; i < n; i++)
    {
        //cerr << Pi[i] << " " << B[i][O[0]] << endl;
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

void Model::betaPass(vector<int> &O, vector<vector<double>> &Beta, vector<double> &C)
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

void Model::diGammaPass(vector<int> &O, vector<vector<double>> &Alpha, vector<vector<double>> &Beta,
                        vector<vector<vector<double>>> &diGamma, vector<vector<double>> &Gamma)
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

vector<vector<double>> Model::getTransitionMatrix() const
{
    return A;
}

vector<vector<double>> Model::getEmissionMatrix() const
{
    return B;
}

vector<double> Model::getInitialDistMatrix() const
{
    return Pi;
}

} /*namespace ducks*/
