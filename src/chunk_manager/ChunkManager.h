//
// Created by bfcda on 10/27/2022.
//

#ifndef LPPG_CHUNKMANAGER_H
#define LPPG_CHUNKMANAGER_H

#include <map>
#include <thread>
#include <future>
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
    map<ChunkCoord, future<Mesh>> threads_;
    Material material_;
    ChunkCoord prev_center_;
    int max_gpu_io_per_frame_;
    Mesh make_mesh(ChunkCoord coord);
public:
    ChunkManager(int render_distance=10, int max_gpu_io_per_frame=2, float chunk_size=40, float start_x=0, float start_z=0);
    ~ChunkManager();
    void load(float x, float z, bool force=false);
    void draw();
};


#endif //LPPG_CHUNKMANAGER_H
