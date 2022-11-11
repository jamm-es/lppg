//
// Created by bfcda on 11/2/2022.
//

#ifndef LPPG_CHUNKCOORD_H
#define LPPG_CHUNKCOORD_H

#include "raylib.h"

struct ChunkCoord {
    int x;
    int z;
    bool operator<(ChunkCoord rhs) const;
    bool operator==(ChunkCoord rhs) const;
    float distance(ChunkCoord rhs) const;
    Vector2 to_world_coord(float chunk_size) const;
    Vector2 calc_subchunk_coord(Vector2 point, float chunk_size) const;
    bool is_point_in(Vector2 point, float chunk_size) const;
    ChunkCoord(Vector2 point, float chunk_size);
    ChunkCoord(int x_coord, int z_coord);
};


#endif //LPPG_CHUNKCOORD_H
