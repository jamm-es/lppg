//
// Created by bfcda on 10/18/2022.
//

#include <vector>
#include <cmath>
#include <random>
#include <limits>
#include "raylib.h"
#include "raymath.h"
#include "PoissonDiskSampler.h"

#include <iostream>

using namespace std;

PoissonDiskSampler::PoissonDiskSampler(int r, int k) {
    radius_ = r;
    k_ = k;
    cell_size_ = r / sqrt(2);
}

// based on Roberts' improved version of Birdson's algorithm
// http://extremelearning.com.au/an-improved-version-of-bridsons-algorithm-n-for-poisson-disc-sampling/
// https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
vector<Vector2> PoissonDiskSampler::sample(float side_length) {
    vector<Vector2> samples;
    int grid_length = static_cast<int>(ceil(side_length/cell_size_));
    vector<int> background_grid(grid_length*grid_length, -1);
    vector<int> active_list;
    random_device rd;
    mt19937_64 gen(rd());

    // randomly choose starting point uniformly over whole grid
    uniform_real_distribution<float> gen_start_pt(0, side_length);
    Vector2 starting_pt = {gen_start_pt(gen), gen_start_pt(gen)};
    samples.push_back(starting_pt);
    active_list.push_back(0);
    background_grid[static_cast<int>(starting_pt.x/cell_size_)*grid_length+static_cast<int>(starting_pt.y/cell_size_)] = 0;
    uniform_real_distribution<float> rand_dist(0, 1);

    while(!active_list.empty()) {
        int rand_index = static_cast<int>(rand_dist(gen)*active_list.size());
        float rand_seed = rand_dist(gen);

        bool found_cand = false;
        for(int i = 0; i < k_; ++i) {
            float theta = 2*PI*(rand_seed+1.0*i/k_);
            float r = radius_ + 1;
            Vector2 delta = {r*cos(theta), r*sin(theta)};
            Vector2 cand = Vector2Add(samples[active_list[rand_index]], delta);

            // bounds check
            if(0 <= cand.x && 0 <= cand.y && side_length >= cand.x && side_length >= cand.y) {
                bool cand_is_ok = true;
                int x_m = static_cast<int>(cand.x / cell_size_);
                int y_m = static_cast<int>(cand.y / cell_size_);
                int x_0 = max(x_m - 2, 0);
                int x_1 = min(x_m + 2, grid_length - 1);
                int y_0 = max(y_m - 2, 0);
                int y_1 = min(y_m + 2, grid_length - 1);
                for(int x = x_0; x <= x_1; ++x) {
                    for(int y = y_0; y <= y_1; ++y) {
                        if(background_grid[x*grid_length+y] == -1) {
                            continue;
                        }
                        Vector2 conflict = samples[background_grid[x*grid_length+y]];
                        if(Vector2DistanceSqr(cand, conflict) < radius_*radius_) {
                            cand_is_ok = false;
                            break;
                        }
                    }
                    if(!cand_is_ok) {
                        break;
                    }
                }

                // we're good! add this point
                if(cand_is_ok) {
                    background_grid[x_m*grid_length+y_m] = samples.size();
                    active_list.push_back(samples.size());
                    samples.push_back(cand);
                    found_cand = true;
                    break;
                }
            }
        }

        // no valid candidates - remove rand_index
        if(!found_cand) {
            int swap = active_list.back();
            active_list.pop_back();
            if(rand_index != active_list.size()) {
                active_list[rand_index] = swap;
            }
        }
    }

    return samples;
}
