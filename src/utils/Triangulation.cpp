//
// Created by bfcda on 10/21/2022.
//

#include "Triangulation.h"
#include "raymath.h"
#undef EPSILON
#include "delaunator-header-only.hpp"

using namespace std;

Triangulation::Triangulation(ChunkCoord chunk_coord, float chunk_size, const vector<Vector2>& center_points, const array<vector<Vector2>, 8>& outer_points) {
    // initialize array of points to feed into triangulator
    int points_size = center_points.size();
    for(const auto& v : outer_points) {
        points_size += v.size();
    }

    vector<double> xz(points_size*2);
    int xz_i = 0;
    for(const auto& v2 : center_points) {
        xz[xz_i++] = v2.x;
        xz[xz_i++] = v2.y;
    }
    for(const auto& vec : outer_points) {
        for(const auto& v2 : vec) {
            xz[xz_i++] = v2.x;
            xz[xz_i++] = v2.y;
        }
    }

    delaunator::Delaunator d(xz);

    // filter out triangles so only those where either 2 or more points are in
    // the center chunk, or has 3 points in all differing chunks, so gets assigned
    // to whoever is the centermost chunk
    // note that an index for a point is in the center chunk if its less than center_point's size
    for(int i = 0; i < d.triangles.size(); i += 3) {
        int tri_status[3] = {-1, -1, -1}; // 0 corresponds to center, anything else corresponds to different chunk
        for(int j = 0; j < 3; ++j) {
            if(d.triangles[i+j] < center_points.size()) {
                tri_status[j] = 0;
            }
            else {
                int size_counter = center_points.size();

                for(int k = 0; k < 8; ++k) {
                    size_counter += outer_points[k].size();
                    if(d.triangles[i+j] < size_counter) {
                        tri_status[j] = k+1;
                    }
                }
            }
        }

        // if 2 or more are in center chunk, add this triangle
        int num_in_center = 0;
        for(int j = 0; j < 3; ++j) {
            if(tri_status[j] == 0) {
                ++num_in_center;
            }
        }

        // also, if all points are in different chunks (incl center)
        // and center is the minmost. this check only necessary if num in center is 1
        bool is_minmost_center = false;
        if(num_in_center == 1) {
            is_minmost_center = true;
            for(int j = 0; j < 3; ++j) {
                int tri_index = d.triangles[i + j];
                ChunkCoord this_tri_chunk({
                    .x = static_cast<float>(xz[tri_index*2]),
                    .y = static_cast<float>(xz[tri_index*2+1])
                }, chunk_size);
                if(chunk_coord == this_tri_chunk) continue;
                if(this_tri_chunk < chunk_coord) is_minmost_center = false;
            }
        }

        // adds triangle that satisfies conditions calculated above
        if(num_in_center >= 2 || is_minmost_center) {
            for(int j = 0; j < 3; ++j) {
                int tri_index = d.triangles[i + j];
                points_.push_back({
                    .x = static_cast<float>(xz[tri_index*2]),
                    .y = static_cast<float>(xz[tri_index*2+1])
                });
            }
        }
    }

//    // filter out indices for sliver tirangles
//    for(int i = 0; i < d.triangles.size(); i += 3) {
//        cout << i << ' ' << d.triangles.size() << endl;
//        // calculate angles with vector math
//        float tri_angles[3];
//        for(int j = 0; j < 3; ++j) {
//            const Vector3& a = vertices[d.triangles[i+j%3]];
//            const Vector3& o = vertices[d.triangles[i+(j+1)%3]];
//            const Vector3& b = vertices[d.triangles[i+(j+2)%3]];
//
//            Vector3 leg1 = Vector3Subtract(a, o);
//            Vector3 leg2 = Vector3Subtract(b, o);
//
//            tri_angles[j] = Vector3Angle(leg1, leg2);
//        }
//
//        float max_angle = *max_element(tri_angles, tri_angles+3);
//        float min_angle = *min_element(tri_angles, tri_angles+3);
//
//        // check if angle is on outer edge
//        bool is_outer = false;
//        for(int j = 0; j < 3; ++j) {
//            if(d.halfedges[i+j] == -1) {
//                is_outer = true;
//                break;
//            }
//        }
//
//        // triangle is ok, add indices to vector
//        if(!is_outer || (max_angle < DEG2RAD*135 && min_angle > DEG2RAD*10)) {
//            indices_.push_back(d.triangles[i]);
//            indices_.push_back(d.triangles[i+1]);
//            indices_.push_back(d.triangles[i+2]);
//        }
//    }
}

vector<Vector2> Triangulation::get_points() const {
    return points_;
}