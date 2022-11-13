#ifndef LPPG_TERRAIN_H
#define LPPG_TERRAIN_H

#include <vector>
#include "raylib.h"

using namespace std;

class Terrain {
private:
    float scale_factor_;
    int height_map_size_;
    int display_size_;
    bool enable_mountains_;
    bool enable_ground_;
    Mesh gen_mesh_from_heightmap(float heightmap[], int width, int height) const;
public:
    Terrain();
    float gen_height_at_coord(float x, float z) const;
    Mesh gen_mesh_from_points(const vector<Vector3>& points) const;
};


#endif //LPPG_TERRAIN_H
