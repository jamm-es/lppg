//
// Created by bfcda on 10/18/2022.
//

#include "PoissonDiskSampler.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include "raymath.h"

using namespace std;

// note that the bg grid and chunks are COMPLETELY disassociated!
PoissonDiskSampler::PoissonDiskSampler(float chunk_size, float r, bool is_truly_random, int k, float epsilon, int bg_grid_length) :
    chunk_size_(chunk_size),
    radius_(r),
    k_(k),
    epsilon_(epsilon),
    bg_cell_size_(r / sqrt(2)),
    bg_grid_length_(bg_grid_length),
    gen_(is_truly_random ? random_device()() : 0) {}

// converts point to given background grid minor coordinates
PoissonDiskSampler::MinorBGCoord PoissonDiskSampler::make_minor_bg_coord(Vector2 point) const {
    float net_x_coord = floor(point.x/bg_cell_size_);
    float net_z_coord = floor(point.y/bg_cell_size_);

    int x_quot = static_cast<int>(floor(net_x_coord/bg_grid_length_));
    int z_quot = static_cast<int>(floor(net_z_coord/bg_grid_length_));

    int x_rem = static_cast<int>(net_x_coord) - x_quot*bg_grid_length_;
    int z_rem = static_cast<int>(net_z_coord) - z_quot*bg_grid_length_;

    return {
        .major = {.x = x_quot, .z = z_quot},
        .x = x_rem,
        .z = z_rem,
    };
}

// offsets minor x coordinate, adjusting major and minor accordingly to ensure no overflows
void PoissonDiskSampler::offset_minor_bg_coord_x(PoissonDiskSampler::MinorBGCoord &bg_coord, int x) const {
    bg_coord.x += x;
    int maj_diff = static_cast<int>(floor(static_cast<float>(bg_coord.x) / bg_grid_length_));
    bg_coord.major.x += maj_diff;
    bg_coord.x -= bg_grid_length_ * maj_diff;
}

// offsets minor z coordinate, adjusting major and minor accordingly to ensure no overflows
void PoissonDiskSampler::offset_minor_bg_coord_z(PoissonDiskSampler::MinorBGCoord &bg_coord, int z) const {
    bg_coord.z += z;
    int maj_diff = static_cast<int>(floor(static_cast<float>(bg_coord.z) / bg_grid_length_));
    bg_coord.major.z += maj_diff;
    bg_coord.z -= bg_grid_length_ * maj_diff;
}

// returns -1 when no samples_ index can be found, returns index otherwise
int PoissonDiskSampler::check_bg_grid(const PoissonDiskSampler::MinorBGCoord& bg_coord) const {
    // always -1 if bg_grids_ isn't even populated for this coord,
    // but doesn't set the vector as that's set_bg_grid's job
    if(bg_grids_.find(bg_coord.major) == bg_grids_.end()) {
        return -1;
    }
    return bg_grids_.at(bg_coord.major)[bg_coord.x*bg_grid_length_+bg_coord.z];
}

// sets index in bg_grids_ at the given point
void PoissonDiskSampler::set_bg_grid(const PoissonDiskSampler::MinorBGCoord& bg_coord, int index) {
    // if bg_grids_'s vector isn't made yet, do so and populate with -1
    if(bg_grids_.find(bg_coord.major) == bg_grids_.end()) {
        bg_grids_[bg_coord.major] = vector<int>(bg_grid_length_*bg_grid_length_, -1);
    }

    // set bg_grids_'s corresponding vector at given point to the given index
    bg_grids_[bg_coord.major][bg_coord.x*bg_grid_length_+bg_coord.z] = index;
}

// tries adding point to given chunk coord, returns true is successful. assumes
// that bg grid already exists
bool PoissonDiskSampler::try_adding_point(ChunkCoord chunk_coord, Vector2 point, vector<int>& active) {
    // check 5x5 (not corners) around current bg_coord
    bool is_point_ok = true;
    MinorBGCoord center_bg_coord = make_minor_bg_coord(point);
    MinorBGCoord curr_bg_coord = center_bg_coord;
    offset_minor_bg_coord_x(curr_bg_coord, -2);
    offset_minor_bg_coord_z(curr_bg_coord, -2);

    // iterate through 5x5, adjusting curr_bg_coord along the way
    for (int x = -2; x <= 2; ++x) {
        offset_minor_bg_coord_x(curr_bg_coord, 1);
        for (int z = -2; z <= 2; ++z) {
            offset_minor_bg_coord_z(curr_bg_coord, 1);

            // skip corners
            if(abs(x)+ abs(z) == 4) {
                continue;
            }

            // get index of potential conflict and check distance with candidate
            int conflict_index = check_bg_grid(curr_bg_coord);
            if(conflict_index == -1) {
                continue;
            }
            Vector2 conflict = samples_[conflict_index];
            if (Vector2DistanceSqr(point, conflict) < radius_*radius_) {
                is_point_ok = false;
                break;
            }
        }

        // quickly break out if point is failed
        if (!is_point_ok) {
            break;
        }

        offset_minor_bg_coord_z(curr_bg_coord, -5);
    }

    // we're good! add this point
    if(is_point_ok) {
        int point_index = samples_.size();
        samples_.push_back(point);

        // add to bg grid
        set_bg_grid(center_bg_coord, point_index);

        // if we're in the current chunk, add point to active list and finished_ for this chunk
        if (chunk_coord.is_point_in(point, chunk_size_)) {
            active.push_back(point_index);
            finished_[chunk_coord].push_back(point_index);
        }

        // otherwise, add to stasis list for the chunk the point is actually in
        else {
            ChunkCoord neighbor_chunk(point, chunk_size_);
            stasis_[neighbor_chunk].push_back(point_index);
        }
    }

    // returns if the point was added or not
    return is_point_ok;
}

// based on Roberts' improved version of Birdson's algorithm
// http://extremelearning.com.au/an-improved-version-of-bridsons-algorithm-n-for-poisson-disc-sampling/
// https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
vector<Vector2> PoissonDiskSampler::get_points_in_chunk(ChunkCoord chunk_coord) {
    // completely lock out this current chunk_coord
    write_m_.lock();
    mutex_m_.lock();
    scoped_lock read_lock(finished_read_m_[chunk_coord]);
    mutex_m_.unlock();

    // return samples if this chunk is already done
    auto it = finished_.find(chunk_coord);
    if(it != finished_.end()) {
        write_m_.unlock(); // releaqse write mutex to let others write
        vector<Vector2> points(it->second.size());
        for(int i = 0; i < it->second.size(); ++i) {
            points[i] = samples_.at(it->second.at(i));
        }
        return points;
    }

    // otherwise, need to make points in this chunk
    vector<int> active;

    // no active points in stasis, so set new seed point
    if(stasis_.find(chunk_coord) == stasis_.end()) {
        uniform_real_distribution<float> gen_start_pt(chunk_size_/2.0-radius_, chunk_size_/2.0+radius_);
        Vector2 starting_pt = {
            .x = gen_start_pt(gen_) + chunk_size_*chunk_coord.x,
            .y = gen_start_pt(gen_) + chunk_size_*chunk_coord.z
        };
        try_adding_point(chunk_coord, starting_pt, active);
    }

    // copy stasis points to active and finished and delete from stasis_
    else {
        active.resize(stasis_[chunk_coord].size());
        active = stasis_[chunk_coord]; // copy stasis
        finished_[chunk_coord] = move(stasis_[chunk_coord]); // move stasis
        stasis_.erase(chunk_coord);
    }

    // ensures random numbers generated henceforth don't generally repeat
    uniform_real_distribution<float> rand_dist(0, 1);

    while(!active.empty()) {
        int rand_active_index = uniform_int_distribution(0, static_cast<int>(active.size() - 1))(gen_);
        Vector2 active_point = samples_[active[rand_active_index]];
        float rand_seed = rand_dist(gen_); // used to offset theta

        // checks points radially around active point for an ok candidate
        bool found_cand = false;
        for(int i = 0; i < k_; ++i) {
            float theta = 2 * PI * (rand_seed + 1.0 * i / k_);
            float r = radius_ + epsilon_; // epsilon ensures distance from active_point
            Vector2 delta = {r * cos(theta), r * sin(theta)};
            Vector2 cand = Vector2Add(active_point, delta);

            // point adding was successful!
            if(try_adding_point(chunk_coord, cand, active)) {
                found_cand = true;
                break;
            }
        }

        // no valid candidates - remove active_point from active list
        if(!found_cand) {
            int swap = active.back();
            active.pop_back();

            // don't do swap if the point to remove was already the backmsot
            if(rand_active_index != active.size()) {
                active[rand_active_index] = swap;
            }
        }
    }

    vector<Vector2> output(finished_[chunk_coord].size());
    for(int i = 0; i < finished_[chunk_coord].size(); ++i) {
        output[i] = samples_[finished_[chunk_coord][i]];
    }

    write_m_.unlock();
    return output;
}

bool PoissonDiskSampler::MajorBGCoord::operator<(const MajorBGCoord rhs) const {
    return x != rhs.x ? x < rhs.x : z < rhs.z;
}

void PoissonDiskSampler::DEBUG_draw(ChunkCoord tl, int l) const {
    for(int i = 0; i < l; ++i) {
        for(int j = 0; j < l; ++j) {
            ChunkCoord cc(tl.x+i, tl.z+j);
            for(int x : finished_.at(cc)) {
                DrawPoint3D({.x=samples_[x].x, .y=5, .z=samples_[x].y}, RED);
            }
        }
    }
}
