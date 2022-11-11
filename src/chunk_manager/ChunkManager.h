//
// Created by bfcda on 10/27/2022.
//

#ifndef LPPG_CHUNKMANAGER_H
#define LPPG_CHUNKMANAGER_H

#include <map>
#include "ChunkCoord.h"
#include "Terrain.h"
#include "PoissonDiskSampler.h"

class ChunkManager {
private:
    int render_distance_;
    float chunk_size_;
    Terrain terrain_;
    PoissonDiskSampler sampler_;
    map<ChunkCoord, Mesh> meshes_;
    Material material_;
    ChunkCoord prev_center_;
    Mesh make_mesh(ChunkCoord coord);
public:
    ChunkManager(int render_distance=10, float chunk_size=40, float start_x=0, float start_z=0);
    void load(float x, float z, bool force=false);
    void draw();
};


#endif //LPPG_CHUNKMANAGER_H
