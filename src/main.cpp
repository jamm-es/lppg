#include <string>
#include "raylib.h"
#include "raymath.h"
#include "ChunkManager.h"

using namespace std;

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "LPPG, the Low Poly Procedural (terrain) Generator");
    SetTargetFPS(60);

    int height_map_size = 500;
    int display_size = 40;
    float scale_factor = 1;

    ChunkManager chunk_manager(5);

    // camera.target - camera.position should always be normalized.
    Camera3D camera = {
        .position =  {0, 3, 1},
        .target =  {0, 3, 0},
        .up =  {0, 1, 0},
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    // camera options
    float cam_move_speed = 0.1;
    float cam_pan_speed = 2*PI/360;

    while (!WindowShouldClose()) {
        // camera updates
        Vector3 cam_dir = Vector3Subtract(camera.target, camera.position);
        Vector3 cam_dir_horiz = Vector3Normalize({cam_dir.x, 0, cam_dir.z});
        Vector3 cam_dir_horiz_left = Vector3RotateByAxisAngle(cam_dir_horiz, {0, 1, 0}, PI/2);
        Vector3 cam_dir_horiz_right = Vector3RotateByAxisAngle(cam_dir_horiz, {0, 1, 0}, -PI/2);
        Vector3 cam_dir_horiz_back = Vector3Scale(cam_dir_horiz, -1);

        // panning
        // rotate left/right
        float horiz_angle_change = 0;
        if(IsKeyDown(KEY_LEFT)) {
            horiz_angle_change += cam_pan_speed;
        }
        if(IsKeyDown(KEY_RIGHT)) {
            horiz_angle_change -= cam_pan_speed;
        }
        cam_dir = Vector3RotateByAxisAngle(cam_dir, {0, 1, 0}, horiz_angle_change);
        // rotate up/down
        float vert_angle_change = 0;
        if(IsKeyDown(KEY_DOWN)) {
            vert_angle_change -= cam_pan_speed;
        }
        if(IsKeyDown(KEY_UP)) {
            vert_angle_change += cam_pan_speed;
        }
        float vert_angle = Vector3Angle(cam_dir, cam_dir_horiz);
        if(vert_angle+vert_angle_change < -PI/2*0.95) {
            vert_angle_change = PI/2*0.95-vert_angle_change;
        }
        cam_dir = Vector3RotateByAxisAngle(cam_dir, cam_dir_horiz_right, vert_angle_change);
        camera.target = Vector3Add(camera.position, cam_dir);

        // movement
        if(IsKeyDown(KEY_W)) {
            camera.target = Vector3Add(camera.target, Vector3Scale(cam_dir_horiz, cam_move_speed));
            camera.position = Vector3Add(camera.position, Vector3Scale(cam_dir_horiz, cam_move_speed));
        }
        if(IsKeyDown(KEY_A)) {
            camera.target = Vector3Add(camera.target, Vector3Scale(cam_dir_horiz_left, cam_move_speed));
            camera.position = Vector3Add(camera.position, Vector3Scale(cam_dir_horiz_left, cam_move_speed));
        }
        if(IsKeyDown(KEY_S)) {
            camera.target = Vector3Add(camera.target, Vector3Scale(cam_dir_horiz_back, cam_move_speed));
            camera.position = Vector3Add(camera.position, Vector3Scale(cam_dir_horiz_back, cam_move_speed));
        }
        if(IsKeyDown(KEY_D)) {
            camera.target = Vector3Add(camera.target, Vector3Scale(cam_dir_horiz_right, cam_move_speed));
            camera.position = Vector3Add(camera.position, Vector3Scale(cam_dir_horiz_right, cam_move_speed));
        }
        if(IsKeyDown(KEY_SPACE)) {
            camera.target = Vector3Add(camera.target, {0, cam_move_speed, 0});
            camera.position = Vector3Add(camera.position, {0, cam_move_speed, 0});
        }
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            camera.target = Vector3Add(camera.target, {0, -cam_move_speed, 0});
            camera.position = Vector3Add(camera.position, {0, -cam_move_speed, 0});
        }


        // load new chunks
        chunk_manager.load(camera.position.x, camera.position.z);


        // drawing
        BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(camera);
        chunk_manager.draw();
        EndMode3D();
        DrawFPS(5, 5);
        string coords_text = "coords: " + to_string(camera.position.x) + ", " + to_string(camera.position.y) + ", " + to_string(camera.position.z);
        DrawText(coords_text.c_str(), 5, GetScreenHeight()-75, 20, ORANGE);
        string speed_text = "speed: " + to_string(cam_move_speed);
        DrawText(speed_text.c_str(), 5, GetScreenHeight()-50, 20, ORANGE);
        string draw_text = "frame time (ms): " + to_string(GetFrameTime()*1000);
        DrawText(draw_text.c_str(), 5, GetScreenHeight()-25, 20, ORANGE);
        EndDrawing();

    }
    CloseWindow();
}