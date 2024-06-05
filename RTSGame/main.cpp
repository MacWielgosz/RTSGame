#include "raylib.h"
#include "raymath.h"
#include <vector>
#include "UnitArmy.h"
#include <random>
using namespace std;



bool CheckCollisionRayBox(Ray ray, BoundingBox box) {
    float tmin = (box.min.x - ray.position.x) / ray.direction.x;
    float tmax = (box.max.x - ray.position.x) / ray.direction.x;

    if (tmin > tmax) {
        float temp = tmin;
        tmin = tmax;
        tmax = temp;
    }

    float tymin = (box.min.y - ray.position.y) / ray.direction.y;
    float tymax = (box.max.y - ray.position.y) / ray.direction.y;

    if (tymin > tymax) {
        float temp = tymin;
        tymin = tymax;
        tymax = temp;
    }

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (box.min.z - ray.position.z) / ray.direction.z;
    float tzmax = (box.max.z - ray.position.z) / ray.direction.z;

    if (tzmin > tzmax) {
        float temp = tzmin;
        tzmin = tzmax;
        tzmax = temp;
    }

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    return true;
}

#define SIZE_NODE 1.0f
#define HEIGHT 25
#define WIDTH 25


PathNode terrainGrid[HEIGHT][WIDTH];

void initTerrainGrid() {
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++) {
            Vector2 position = { i * SIZE_NODE-(HEIGHT*SIZE_NODE/2) , j * SIZE_NODE - (WIDTH * SIZE_NODE / 2) };
            PathNode* node = &terrainGrid[i][j];
            node->position = position;
        }
    }
}
void drawTerrainGrid() {
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++) {
            PathNode* node = &terrainGrid[i][j];
            Vector3 nodePos = { node->position.x, 0.0f, node->position.y };

            // Rysowanie ko³a w miejscu wêz³a
            DrawCircle3D(nodePos, 0.1f, Vector3 { 1.0f, 0.0f, 0.0f }, 90.0f, BLUE);

            // Rysowanie po³¹czeñ miêdzy s¹siadami
            for (int k = 0; k < node->connectionCount; k++) {
                int nx = node->connections[k][0];
                int ny = node->connections[k][1];
                PathNode* neighbour = &terrainGrid[ny][nx];
                Vector3 neighbourPos = { neighbour->position.x, 0.0f, neighbour->position.y };

                DrawLine3D(nodePos, neighbourPos, DARKGRAY);
            }
        }
    }
}

bool PointOutsideMap(Vector3 point) {
    return point.x < 0 || point.y < 0 || point.x >= WIDTH * SIZE_NODE || point.y >= HEIGHT * SIZE_NODE;
}

void FindNeighbors(int nodeIndex, BoundingBox* nonWalkableColliders, int colliderCount) {
    int x = nodeIndex % WIDTH;
    int y = nodeIndex / WIDTH;
    PathNode* node = &terrainGrid[y][x];
    Vector2 position = node->position;

    static const int delta[8][2] = {
        { -1, -1 }, { 0, -1 }, { 1, -1 },
        { -1,  0 },           { 1,  0 },
        { -1,  1 }, { 0,  1 }, { 1,  1 }
    };

    int connectionIndex = 0;

    for (int i = 0; i < 8; i++) {
        int nx = x + delta[i][0];
        int ny = y + delta[i][1];

        if (nx >= 0 && ny >= 0 && nx < WIDTH && ny < HEIGHT) {
            PathNode* neighbour = &terrainGrid[ny][nx];
            Vector2 neighbourPosition = neighbour->position;

            float distance = Vector2Distance(position, neighbourPosition);
            Vector2 rayDir = Vector2Normalize(Vector2Subtract(neighbourPosition, position));
            Ray connectionRay = { Vector3 { position.x, 0.0f, position.y }, Vector3 { rayDir.x, 0.0f, rayDir.y } };

            bool hasCollision = false;
            for (int j = 0; j < colliderCount; j++) {
                RayCollision collision = GetRayCollisionBox(connectionRay, nonWalkableColliders[j]);
                if (collision.hit && collision.distance <= distance) {
                    hasCollision = true;
                    break;
                }
            }

            if (!hasCollision) {
                node->connections[connectionIndex][0] = nx;
                node->connections[connectionIndex][1] = ny;
                node->connectionCount++;
                connectionIndex++;
            }
        }
    }
}

int main(void)
{
    const int screenWidth = 1500;
    const int screenHeight = 700;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "RTS");
    initTerrainGrid();
    BoundingBox nonWalkableColliders[1] = {
        { Vector3 { 5.0f, -0.1f, 5.0f }, Vector3 { 6.0f, 1.0f, 6.0f } } // Przyk³adowa przeszkoda
    };
    int colliderCount = 1;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            FindNeighbors(i * WIDTH + j, nonWalkableColliders, colliderCount);
        }
    }
    Camera camera = { 0 };
    camera.position = Vector3{ 5.0f, 5.0f, 5.0f };
    camera.target = Vector3{ 0.0f, 2.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 50.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    ShowCursor();

    Model modelArch = LoadModel("model/Archer_Unit.glb");

    vector<UnitArmy> Units = {
        UnitArmy(Vector3{ 0.0f, 0.0f, 0.0f }, modelArch, false),
        UnitArmy(Vector3{ 3.0f, 0.0f, 0.0f }, modelArch, false),
        UnitArmy(Vector3{ 6.0f, 0.0f, 0.0f }, modelArch, false)
    };

    Vector2 mouseStart = { 0.0f, 0.0f };
    Vector2 mouseEnd = { 0.0f, 0.0f };
    bool isSelecting = false;
    while (!WindowShouldClose())
    {

        if (IsKeyDown(KEY_Q))
        {
            Vector3 rotation = { -0.8f, 0.0f, 0.0f };
            UpdateCameraPro(&camera, Vector3Zero(), rotation, 0.0f);
        }
        if (IsKeyDown(KEY_E))
        {
            Vector3 rotation = { 0.8f, 0.0f, 0.0f };
            UpdateCameraPro(&camera, Vector3Zero(), rotation, 0.0f);
        }

        Vector3 movement = { 0.0f, 0.0f, 0.0f };
        if (IsKeyDown(KEY_W)) movement.x += 0.1f;
        if (IsKeyDown(KEY_S)) movement.x -= 0.1f;
        if (IsKeyDown(KEY_A)) movement.y -= 0.1f;
        if (IsKeyDown(KEY_D)) movement.y += 0.1f;

        UpdateCameraPro(&camera, movement, Vector3Zero(), 0.0f);

        camera.target = Vector3Add(camera.position, Vector3Normalize(Vector3Subtract(camera.target, camera.position)));

        float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0)
        {
            camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), wheelMove * 0.1f));
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            mouseStart = GetMousePosition();
            isSelecting = true;

            for (int i = 0; i < Units.size(); i++)
            {
                Units[i].setIsSelected(false);
            }
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && isSelecting)
        {
            mouseEnd = GetMousePosition();
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isSelecting)
        {
            isSelecting = false;

            for (int i = 0; i < Units.size(); i++)
            {
                Vector3 modelPos = Units[i].getPosition();
                Vector3 modelMin = Vector3{ modelPos.x - 1.0f, modelPos.y - 1.0f, modelPos.z - 1.0f };
                Vector3 modelMax = Vector3{ modelPos.x + 1.0f, modelPos.y + 1.0f, modelPos.z + 1.0f };

                Vector2 modelScreenPos = GetWorldToScreen(Vector3{ modelPos.x, modelPos.y, modelPos.z }, camera);

                if ((modelScreenPos.x > mouseStart.x && modelScreenPos.x < mouseEnd.x || modelScreenPos.x < mouseStart.x && modelScreenPos.x > mouseEnd.x) &&
                    (modelScreenPos.y > mouseStart.y && modelScreenPos.y < mouseEnd.y || modelScreenPos.y < mouseStart.y && modelScreenPos.y > mouseEnd.y))
                {
                    Units[i].setIsSelected(true);
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Ray ray = GetMouseRay(GetMousePosition(), camera);

            for (int i = 0; i < Units.size(); i++)
            {
                Vector3 modelPos = Units[i].getPosition();
                BoundingBox box = { Vector3{ modelPos.x - 1.0f, modelPos.y - 1.0f, modelPos.z - 1.0f }, Vector3{ modelPos.x + 1.0f, modelPos.y + 1.0f, modelPos.z + 1.0f } };

                if (CheckCollisionRayBox(ray, box))
                {
                    Units[i].setIsSelected(!Units[i].getIsSelected());
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            Vector2 mousePos = GetMousePosition();
            Ray ray = GetMouseRay(mousePos, camera);

            Vector3 p1 = { -50.0f, 0.0f, -50.0f };
            Vector3 p2 ={ 50.0f, 0.0f, -50.0f };
            Vector3 p3 = { 50.0f, 0.0f, 50.0f };
            Vector3 p4 = { -50.0f, 0.0f, 50.0f };

            RayCollision collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);

            if (collision.hit)
            {
                Vector3 baseGoal = collision.point;
                int selectedUnitCount = 0;
                for (int i = 0; i < Units.size(); i++)
                {
                    if (Units[i].getIsSelected())
                    {
                        selectedUnitCount++;
                    } 
                }

                int unitIndex = 0;
                for (int i = 0; i < Units.size(); i++)
                {
                    if (Units[i].getIsSelected()) {
                        float offset = (float)unitIndex - (selectedUnitCount - 1) / 2.0f;
                        Vector3 formationGoal = Vector3Add(baseGoal, Vector3{ offset * 2.0f, 0.0f, 0.0f });

                        // Ustaw now¹ œcie¿kê
                        Units[i].setGoal(formationGoal);
                        unitIndex++;
                    }

                }
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        drawTerrainGrid();
        DrawBoundingBox(nonWalkableColliders[0], BLACK);
        for (int i = 0; i < Units.size(); i++)
        {
            Units[i].anim();
        }

        EndMode3D();

        if (isSelecting)
        {
            float x = mouseEnd.x;
            float y = mouseEnd.y;
            float w = mouseStart.x - mouseEnd.x;
            float h = mouseStart.y - mouseEnd.y;
            if (w < 0) { x = mouseStart.x; w *= -1; }
            if (h < 0) { y = mouseStart.y; h *= -1; }
            DrawRectangleLinesEx(Rectangle{ x, y, w, h }, 1, Fade(BLUE, 0.5f));
        }

        DrawText("RTS", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    UnloadModel(modelArch);
    CloseWindow();

    return 0;
}
