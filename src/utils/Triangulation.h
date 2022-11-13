//
// Created by bfcda on 10/21/2022.
//

#ifndef LPPG_TRIANGULATION_H
#define LPPG_TRIANGULATION_H

#include <vector>
#include <array>
#include "raylib.h"
#include "ChunkCoord.h"

using namespace std;

class Triangulation {
private:
    vector<Vector2> points_;
public:
    Triangulation(ChunkCoord chunk_coord, float chunk_size, const vector<Vector2>& center_points, const array<vector<Vector2>, 8>& outer_points);
    vector<Vector2> get_points() const;
};


#endif //LPPG_TRIANGULATION_H
