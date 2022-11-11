//
// Created by bfcda on 10/21/2022.
//

#include "Triangulation.h"
#undef EPSILON
#include "delaunator-header-only.hpp"

using namespace std;

Triangulation::Triangulation(const vector<Vector3>& vertices) {
    vector<double> xz(vertices.size()*2);
    int i = 0;
    for(const auto& v3 : vertices) {
        xz[i++] = v3.x;
        xz[i++] = v3.z;
    }
    delaunator::Delaunator d(xz);
    indices_ = d.triangles;
}

vector<size_t> Triangulation::getIndices() const {
    return indices_;
}