//
// Created by bfcda on 10/27/2022.
//

#include <thread>
#include <mutex>
#include "ChunkManager.h"
#include "raylib.h"
#include "raymath.h"

#include <iostream>

ChunkManager::ChunkManager(int render_distance, float chunk_size, float start_x, float start_z) :
    render_distance_(render_distance),
    chunk_size_(chunk_size),
    sampler_(chunk_size, 3),
    prev_center_(999, 999) {
    material_ = LoadMaterialDefault();
    load(start_x, start_z, true);
}

Mesh ChunkManager::make_mesh(ChunkCoord coord) {
    vector<Vector2> xz_points = sampler_.gen_points_in_chunk(coord);
    vector<Vector3> points(xz_points.size());
    for(int i = 0; i < points.size(); ++i) {
        points[i].x = xz_points[i].x;
        points[i].z = xz_points[i].y;
        points[i].y = terrain_.gen_height_at_coord(points[i].x, points[i].z);
    }
    return terrain_.gen_mesh_from_points(points);
}

// given the current position, checks if there's any chunks that need to be
// loaded or unloaded.
void ChunkManager::load(float x, float z, bool force) {
    ChunkCoord center({.x = x, .y = z}, chunk_size_);

    // don't need to load/unload anything if center hasn't changed, but force bypasses this
    if(!force && center == prev_center_) {
        return;
    }
    prev_center_ = center;

    // unloading chunks outside radius
    auto it = meshes_.begin();
    while(it != meshes_.end()) {
        if(center.distance(it->first) > render_distance_-0.5) {
            UnloadMesh(it->second);
            it = meshes_.erase(it);
        }
        else {
            ++it;
        }
    }

    // load chunks in radius by scanning square around center point but restricts to radius around
    for(int i = center.x - render_distance_; i <= center.x + render_distance_; ++i) {
        for(int j = center.z - render_distance_; j <= center.z + render_distance_; ++j) {
            ChunkCoord cur_chunk(i, j);

            // don't re-make chunks outside the render radius
            if(center.distance(cur_chunk) > render_distance_-0.5) {
                continue;
            }

            // make chunk if it isn't already made
            if(meshes_.find(cur_chunk) == meshes_.end()) {
                meshes_[cur_chunk] = make_mesh(cur_chunk);
            }
        }
    }
}

void ChunkManager::draw() {
    for(const auto& p : meshes_) {
        DrawMesh(p.second, material_, MatrixIdentity());
    }
}
