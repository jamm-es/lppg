//
// Created by bfcda on 11/2/2022.
//

#include "ChunkCoord.h"
#include <cmath>

using namespace std;

bool ChunkCoord::operator<(ChunkCoord rhs) const {
    return x != rhs.x ? x < rhs.x : z < rhs.z;
}

bool ChunkCoord::operator==(ChunkCoord rhs) const {
    return x == rhs.x && z == rhs.z;
}

float ChunkCoord::distance(ChunkCoord rhs) const {
    return sqrt((x-rhs.x)*(x-rhs.x)+(z-rhs.z)*(z-rhs.z));
}

Vector2 ChunkCoord::to_world_coord(float chunk_size) const {
    return {.x = x*chunk_size, .y = z*chunk_size};
}

Vector2 ChunkCoord::calc_subchunk_coord(Vector2 point, float chunk_size) const {
    Vector2 world_coord = to_world_coord(chunk_size);
    return {.x = point.x-world_coord.x, .y = point.y-world_coord.y};
}

bool ChunkCoord::is_point_in(Vector2 point, float chunk_size) const {
    Vector2 subchunk = calc_subchunk_coord(point, chunk_size);
    return subchunk.x >= 0 && subchunk.y >= 0 && subchunk.x < chunk_size && subchunk.y < chunk_size;
}

ChunkCoord::ChunkCoord(Vector2 point, float chunk_size) :
    x(static_cast<int>(floor(point.x/chunk_size))),
    z(static_cast<int>(floor(point.y/chunk_size))) {}

ChunkCoord::ChunkCoord(int x_coord, int z_coord) :
    x(x_coord),
    z(z_coord) {}