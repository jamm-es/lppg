#ifndef LPPG_TERRAIN_H
#define LPPG_TERRAIN_H

#include <vector>
#include "raylib.h"

using namespace std;

class Terrain {
private:
    int height_map_size_;
    int display_size_;
    Model terrain_;
    bool enable_mountains_;
    bool enable_ground_;
    Mesh gen_mesh_from_triangulation();
    Mesh gen_mesh_from_heightmap(float heightmap[], int width, int height) const;
public:
    Terrain(int height_map_size, int display_size, float scale_factor);
    ~Terrain();
    void draw();
};


#endif //LPPG_TERRAIN_H
