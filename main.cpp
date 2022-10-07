#include <iostream>
#include <vector>
#include "raylib.h"
#include "raymath.h"
#include "fastnoiselite.h"

using namespace std;

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "LPPG, the Low Poly Procedural (terrain) Generator");
    SetTargetFPS(60);

    int height_map_size = 500;
    int display_size = 20;

    // generate noise
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    unsigned char* image_noise_data = (unsigned char*) malloc((height_map_size+1)*(height_map_size+1)*sizeof(unsigned char));
    for(int x = 0; x <= height_map_size; ++x) {
        for(int y = 0; y <= height_map_size; ++y) {
            image_noise_data[x*(height_map_size+1)+y] = (unsigned char) (noise.GetNoise((float) x, (float) y)*256);
        }
    }
    Image noise_image = {
        .data = image_noise_data,
        .width = height_map_size+1,
        .height = height_map_size+1,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
    };
    Mesh mesh = GenMeshHeightmap(noise_image, {static_cast<float>(display_size), 5, static_cast<float>(display_size)});
    Texture test = LoadTextureFromImage(noise_image);
    //UnloadImage(noise_image);
    Model terrain = LoadModelFromMesh(mesh);

    // generate and set terrain texture
    Image checker = GenImageChecked(height_map_size, height_map_size, 1, 1, GREEN, BLUE);
    Texture2D texture = LoadTextureFromImage(checker);
    UnloadImage(checker);
    terrain.materials->maps[MATERIAL_MAP_DIFFUSE].texture = test;

    Camera3D camera = {
        .position =  {0, 10, 10},
        .target =  {0, 0, 0},
        .up =  {0, 1, 0},
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE
    };\

    // camera options
    SetCameraMode(camera, CAMERA_ORBITAL);

    // main draw loop
    while (!WindowShouldClose()) {
        // camera updates
        UpdateCamera(&camera);

        // drawing
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(camera);
        DrawGrid(20, 1.0f);
        DrawModel(terrain, {static_cast<float>(-display_size)/2, 0, static_cast<float>(-display_size)/2}, 1, WHITE);
        EndMode3D();
        //DrawTexture(test, 0, 0, WHITE);
        DrawFPS(5, 5);
        EndDrawing();
    }


    return 0;
}