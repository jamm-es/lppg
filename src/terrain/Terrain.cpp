//
// Created by bfcda on 10/7/2022.
//

#include <cstdlib>
#include <algorithm>
#include "Terrain.h"
#include "raylib.h"
#include "raymath.h"
#include "fastnoiselite.h"
#include "PoissonDiskSampler.h"

#include <iostream>

using namespace std;

Terrain::Terrain(int hms, int ds, float scale_factor) {
    height_map_size_ = hms;
    display_size_ = ds;
    enable_mountains_ = false;
    enable_ground_ = true;

    // generate grid of simplex2 noise
    int base_noise_increments = 500;
    FastNoiseLite base_noise;
    base_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    base_noise.SetFractalType(FastNoiseLite::FractalType_None);
    vector<float> base_data((base_noise_increments+1) * (base_noise_increments+1));
    for(int x = 0; x <= base_noise_increments; ++x) {
        for(int y = 0; y <= base_noise_increments; ++y) {
            float x_inc = static_cast<float>(x)/base_noise_increments*display_size_;
            float y_inc = static_cast<float>(y)/base_noise_increments*display_size_;
            base_data[x*(base_noise_increments+1)+y] = base_noise.GetNoise(x_inc, y_inc);
        }
    }


    auto* noise_data = (float*) malloc((height_map_size_+1)*(height_map_size_+1) * sizeof(float));
    fill(noise_data, noise_data+(height_map_size_+1)*(height_map_size_+1), 0);

    // generate mountain noise
    if(enable_mountains_) {
        FastNoiseLite mountain_noise;
        mountain_noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        mountain_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        mountain_noise.SetFrequency(0.002);
        mountain_noise.SetFractalWeightedStrength(0.3);
        for(int x = 0; x <= height_map_size_; ++x) {
            for(int y = 0; y <= height_map_size_; ++y) {
                noise_data[x*(height_map_size_ + 1) + y] = 20*mountain_noise.GetNoise(x*scale_factor, y*scale_factor)/scale_factor;
            }
        }
    }

    // generate ground bumps with decreasing frequency
    if(enable_ground_) {
        float ground_freq = 0.01;
        float ground_freq_multiplier = 2;
        float ground_amp = 0.2;
        float ground_amp_multiplier = 0.5;
        float ground_iterations = 3;
        for(int i = 0; i < ground_iterations; ++i) {
            FastNoiseLite ground_noise;
            ground_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
            ground_noise.SetFractalType(FastNoiseLite::FractalType_None);
            ground_noise.SetFrequency(ground_freq);
            for(int x = 0; x <= height_map_size_; ++x) {
                for(int y = 0; y <= height_map_size_; ++y) {
                    noise_data[x*(height_map_size_ + 1) + y] += ground_amp*ground_noise.GetNoise(x*scale_factor, y*scale_factor)/scale_factor;
                }
            }
            ground_freq *= ground_freq_multiplier;
            ground_amp *= ground_amp_multiplier;
        }
    }

    Mesh mesh = gen_mesh_from_heightmap(noise_data, height_map_size_+1, height_map_size_+1);
    free(noise_data);

    terrain_ = LoadModelFromMesh(mesh);
    /*
    Texture uv_test_texture = LoadTexture("C:/Users/bfcda/CLionProjects/lppg/src/terrain/uv_test.png");
    terrain_.materials->maps[MATERIAL_MAP_DIFFUSE].texture = uv_test_texture;
     */
}

Terrain::~Terrain() {
    UnloadModel(terrain_);
}

void Terrain::draw() {
    DrawModel(terrain_, {static_cast<float>(-display_size_) / 2, 0, static_cast<float>(-display_size_) / 2}, 1, WHITE);
}

Mesh Terrain::gen_mesh_from_triangulation() {
    Mesh mesh = {0};
    return mesh;
}

Mesh Terrain::gen_mesh_from_heightmap(float heightmap[], int width, int height) const {
    Color base_color = {20, 97, 33, 255};

    float x_gap = display_size_ / static_cast<float>(width - 1);
    float z_gap = display_size_ / static_cast<float>(height - 1);

    float heightmap_min = *min_element(heightmap, heightmap+width*height);
    float heightmap_max = *max_element(heightmap, heightmap+width*height);

    Mesh mesh = {0};
    mesh.triangleCount = 2*(width-1)*(height-1);
    mesh.vertexCount = mesh.triangleCount*3;
    mesh.vertices = (float*) malloc(mesh.vertexCount*3*sizeof(float));
    mesh.normals = (float*) malloc(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float*) malloc(mesh.vertexCount*2*sizeof(float));
    mesh.colors = (unsigned char*) malloc(mesh.vertexCount*4*sizeof(unsigned char));

    int cur_quad = 0;
    float min_x_corner = 0;
    float min_z_corner = 0;
    for(int row = 0; row < width-1; ++row) {
        min_z_corner = 0;
        for(int col = 0; col < height-1; ++col) {
            // define vertices to use over and over
            // top left is min x and min z
            Vector3 top_left = {min_x_corner, heightmap[row*width+col], min_z_corner};
            Vector3 top_right = {min_x_corner+x_gap, heightmap[(row+1)*width+col], min_z_corner};
            Vector3 bottom_left = {min_x_corner, heightmap[row*width+col+1], min_z_corner+z_gap};
            Vector3 bottom_right = {min_x_corner+x_gap, heightmap[(row+1)*width+col+1], min_z_corner+z_gap};

            // vertices

            // triangle 1 - top left, bottom left, top right
            mesh.vertices[cur_quad*18] = top_left.x;
            mesh.vertices[cur_quad*18+1] = top_left.y;
            mesh.vertices[cur_quad*18+2] = top_left.z;

            mesh.vertices[cur_quad*18+3] = bottom_left.x;
            mesh.vertices[cur_quad*18+4] = bottom_left.y;
            mesh.vertices[cur_quad*18+5] = bottom_left.z;

            mesh.vertices[cur_quad*18+6] = top_right.x;
            mesh.vertices[cur_quad*18+7] = top_right.y;
            mesh.vertices[cur_quad*18+8] = top_right.z;

            // triangle 2 - top right, bottom left, bottom right
            mesh.vertices[cur_quad*18+9] = top_right.x;
            mesh.vertices[cur_quad*18+10] = top_right.y;
            mesh.vertices[cur_quad*18+11] = top_right.z;

            mesh.vertices[cur_quad*18+12] = bottom_left.x;
            mesh.vertices[cur_quad*18+13] = bottom_left.y;
            mesh.vertices[cur_quad*18+14] = bottom_left.z;

            mesh.vertices[cur_quad*18+15] = bottom_right.x;
            mesh.vertices[cur_quad*18+16] = bottom_right.y;
            mesh.vertices[cur_quad*18+17] = bottom_right.z;

            // texcoords

            // triangle 1 - top left, top right, bottom left
            mesh.texcoords[cur_quad*12] = row/static_cast<float>(width-1);
            mesh.texcoords[cur_quad*12+1] = col/static_cast<float>(height-1);

            mesh.texcoords[cur_quad*12+2] = row/static_cast<float>(width-1);
            mesh.texcoords[cur_quad*12+3] = (col+1)/static_cast<float>(height-1);

            mesh.texcoords[cur_quad*12+4] = (row+1)/static_cast<float>(width-1);
            mesh.texcoords[cur_quad*12+5] = col/static_cast<float>(height-1);

            // triangle 2 - bottom left, top right, bottom right
            mesh.texcoords[cur_quad*12+6] = (row+1)/static_cast<float>(width-1);
            mesh.texcoords[cur_quad*12+7] = col/static_cast<float>(height-1);

            mesh.texcoords[cur_quad*12+8] = row/static_cast<float>(width-1);
            mesh.texcoords[cur_quad*12+9] = (col+1)/static_cast<float>(height-1);

            mesh.texcoords[cur_quad*12+10] = (row+1)/static_cast<float>(width-1);
            mesh.texcoords[cur_quad*12+11] = (col+1)/static_cast<float>(height-1);

            // normals
            Vector3 tri_1_leg_a = Vector3Subtract(bottom_left, top_left);
            Vector3 tri_1_leg_b = Vector3Subtract(top_right, top_left);
            Vector3 tri_1_normal = Vector3Normalize(Vector3CrossProduct(tri_1_leg_a, tri_1_leg_b));

            Vector3 tri_2_leg_a = Vector3Subtract(bottom_left, bottom_right);
            Vector3 tri_2_leg_b = Vector3Subtract(top_right, bottom_right);
            Vector3 tri_2_normal = Vector3Normalize(Vector3CrossProduct(tri_2_leg_a, tri_2_leg_b));

            // triangle 1 - top left, bottom left, top right
            mesh.normals[cur_quad*18] = tri_1_normal.x;
            mesh.normals[cur_quad*18+1] = tri_1_normal.y;
            mesh.normals[cur_quad*18+2] = tri_1_normal.z;

            mesh.normals[cur_quad*18+3] = tri_1_normal.x;
            mesh.normals[cur_quad*18+4] = tri_1_normal.y;
            mesh.normals[cur_quad*18+5] = tri_1_normal.z;

            mesh.normals[cur_quad*18+6] = tri_1_normal.x;
            mesh.normals[cur_quad*18+7] = tri_1_normal.y;
            mesh.normals[cur_quad*18+8] = tri_1_normal.z;

            // triangle 2 - top right, bottom left, bottom right
            mesh.normals[cur_quad*18+9] = tri_2_normal.x;
            mesh.normals[cur_quad*18+10] = tri_2_normal.y;
            mesh.normals[cur_quad*18+11] = tri_2_normal.z;

            mesh.normals[cur_quad*18+12] = tri_2_normal.x;
            mesh.normals[cur_quad*18+13] = tri_2_normal.y;
            mesh.normals[cur_quad*18+14] = tri_2_normal.z;

            mesh.normals[cur_quad*18+15] = tri_2_normal.x;
            mesh.normals[cur_quad*18+16] = tri_2_normal.y;
            mesh.normals[cur_quad*18+17] = tri_2_normal.z;

            // colors
            float hm_scale = (heightmap[row*width+col]-heightmap_min)/(heightmap_max-heightmap_min);
            auto color_bright = (unsigned char) (hm_scale*255);
            int max_amplitude = 20;
            Color tri_1_color = {
                static_cast<unsigned char>(base_color.r+rand()%(max_amplitude*2+1)-max_amplitude),
                static_cast<unsigned char>(base_color.g+rand()%(max_amplitude*2+1)-max_amplitude),
                static_cast<unsigned char>(base_color.b+rand()%(max_amplitude*2+1)-max_amplitude),
                255
            };
            Color tri_2_color = {
                static_cast<unsigned char>(base_color.r+rand()%(max_amplitude*2+1)-max_amplitude),
                static_cast<unsigned char>(base_color.g+rand()%(max_amplitude*2+1)-max_amplitude),
                static_cast<unsigned char>(base_color.b+rand()%(max_amplitude*2+1)-max_amplitude),
                255
            };

            // triangle 1 - top left, bottom left, top right
            mesh.colors[cur_quad*24] = tri_1_color.r;
            mesh.colors[cur_quad*24+1] = tri_1_color.g;
            mesh.colors[cur_quad*24+2] = tri_1_color.b;
            mesh.colors[cur_quad*24+3] = 255;

            mesh.colors[cur_quad*24+4] = tri_1_color.r;
            mesh.colors[cur_quad*24+5] = tri_1_color.g;
            mesh.colors[cur_quad*24+6] = tri_1_color.b;
            mesh.colors[cur_quad*24+7] = 255;

            mesh.colors[cur_quad*24+8] = tri_1_color.r;
            mesh.colors[cur_quad*24+9] = tri_1_color.g;
            mesh.colors[cur_quad*24+10] = tri_1_color.b;
            mesh.colors[cur_quad*24+11] = 255;

            // triangle 2 - top right, bottom left, bottom right
            mesh.colors[cur_quad*24+12] = tri_2_color.r;
            mesh.colors[cur_quad*24+13] = tri_2_color.g;
            mesh.colors[cur_quad*24+14] = tri_2_color.b;
            mesh.colors[cur_quad*24+15] = 255;

            mesh.colors[cur_quad*24+16] = tri_2_color.r;
            mesh.colors[cur_quad*24+17] = tri_2_color.g;
            mesh.colors[cur_quad*24+18] = tri_2_color.b;
            mesh.colors[cur_quad*24+19] = 255;

            mesh.colors[cur_quad*24+20] = tri_2_color.r;
            mesh.colors[cur_quad*24+21] = tri_2_color.g;
            mesh.colors[cur_quad*24+22] = tri_2_color.b;
            mesh.colors[cur_quad*24+23] = 255;

            ++cur_quad;
            min_z_corner += z_gap;
        }
        min_x_corner += x_gap;
    }

    UploadMesh(&mesh, false);

    return mesh;
}