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
    Model(int nState, int nEmmision)
    {
        A.resize(nState, vector<double> (nState));
        B.resize(nState, vector<double> (nEmmision));


        for (int i = 0; i < nState; i++)
        {
            double sum = 0;
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
    }
};

} // namespace ducks

#endif