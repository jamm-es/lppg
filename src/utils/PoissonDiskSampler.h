//
// Created by bfcda on 10/18/2022.
//

#ifndef LPPG_POISSONDISKSAMPLER_H
#define LPPG_POISSONDISKSAMPLER_H

#include <vector>
#include <random>
#include <map>
#include "raylib.h"
#include "ChunkCoord.h"

using namespace std;

class PoissonDiskSampler {
private:
    struct MajorBGCoord {
        int x;
        int z;
        bool operator<(const MajorBGCoord rhs) const;
    };

    struct MinorBGCoord {
        MajorBGCoord major;
        int x;
        int z;
    };

    float chunk_size_;
    float radius_;
    int k_;
    float epsilon_;
    float bg_cell_size_;
    int bg_grid_length_;
    mt19937_64 gen_;
    map<MajorBGCoord, vector<int>> bg_grids_;
    map<ChunkCoord, vector<int>> stasis_;
    map<ChunkCoord, vector<int>> finished_;
    vector<Vector2> samples_;

    MinorBGCoord make_minor_bg_coord(Vector2 point) const;
    void offset_minor_bg_coord_x(MinorBGCoord& bg_coord, int x) const;
    void offset_minor_bg_coord_z(MinorBGCoord& bg_coord, int z) const;

    int check_bg_grid(const MinorBGCoord& bg_coord) const;
    void set_bg_grid(const MinorBGCoord& bg_coord, int index);

    bool try_adding_point(ChunkCoord chunk_coord, Vector2 point, vector<int>& active);
public:
    PoissonDiskSampler(float chunk_size, float r, bool is_truly_random=false, int k = 10, float epsilon = 0.001, int bg_grid_length=20);
    vector<Vector2> get_points_in_chunk(ChunkCoord chunk_coord);
    void DEBUG_draw(ChunkCoord tl, int l) const;
};


#endif //LPPG_POISSONDISKSAMPLER_H
