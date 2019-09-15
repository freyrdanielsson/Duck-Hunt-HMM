#ifndef _DUCKS_BIRD_HPP_
#define _DUCKS_BIRD_HPP_

#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

namespace ducks
{

class Model
{

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

public:
    Model(int nState, int nEmission)
    {
        A.resize(nState, vector<double> (nState));
        B.resize(nState, vector<double> (nEmission));
        Pi.resize(nState);

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
        for (int i = 0; i < nEmission; i++)
        {
            sum = 0;
            for (int j = 0; j < nEmission; j++)
            {   
                double randDouble = (double)rand() / RAND_MAX;
                sum += randDouble;
                B[i][j] = randDouble;
            }
            // Normalize
            for (int j = 0; j < nEmission; j++)
            {
                B[i][j] /= sum;
            }
            
        }

        // Initialize initial state dist matrix
        sum = 0;
        for (int i = 0; i < nState; i++)
        {
            double randDouble = (double)rand() / RAND_MAX;
            sum += randDouble;
            Pi[i] = randDouble;
        }

        for (int i = 0; i < nState; i++)
        {
            Pi[i] /= sum;
        }
    }

    // For debugging
    vector<vector <double>> getTransition() const
    {
        return A;
    }

    vector<vector <double>> getEmission() const
    {
        return B;
    }

    vector<double> getInitialDist() const
    {
        return Pi;
    }
};

} // namespace ducks

#endif