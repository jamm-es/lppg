//
// Created by bfcda on 10/18/2022.
//

#ifndef LPPG_POISSONDISKSAMPLER_H
#define LPPG_POISSONDISKSAMPLER_H

#include <vector>
#include "raylib.h"

using namespace std;

class PoissonDiskSampler {
private:
    int radius_;
    int k_;
    float cell_size_;
public:
    PoissonDiskSampler(int r, int k = 3);
    vector<Vector2> sample(float side_length);
};


#endif //LPPG_POISSONDISKSAMPLER_H
