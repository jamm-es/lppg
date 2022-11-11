//
// Created by bfcda on 10/21/2022.
//

#ifndef LPPG_TRIANGULATION_H
#define LPPG_TRIANGULATION_H

#include <vector>
#include "raylib.h"

using namespace std;

class Triangulation {
private:
    vector<size_t> indices_;
public:
    Triangulation(const vector<Vector3>& vertices);
    vector<size_t> getIndices() const;
};


#endif //LPPG_TRIANGULATION_H
