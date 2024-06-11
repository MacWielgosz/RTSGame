#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include <vector>
#include <cmath>
#include "rlgl.h"
//#define DEBUG
using namespace std;
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#include "Map.h"
#include "UnitArmy.h"

int main(void)
{
    const int screenWidth = 1500;
    const int screenHeight = 700;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "RTS");

    Map* map = Map::getMap();

    
    BoundingBox nonWalkableColliders[] = {
        { Vector3 { 5.0f, -0.2f, 5.0f }, Vector3 { 8.0f, 1.0f, 8.0f } }, // Przyk³adowa przeszkoda 1
        { Vector3 { 7.0f, -0.2f, 7.0f }, Vector3 { 12.0f, 1.0f, 12.0f } } // Przyk³adowa przeszkoda 2
    };

    int colliderCount = ARRAY_SIZE(nonWalkableColliders);


    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            map->FindNeighbors(i,j, nonWalkableColliders, colliderCount);
        }
    }

    
    Camera camera = { 0 };
    camera.position = Vector3{ 5.0f, 5.0f, 5.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 3.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 50.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    ShowCursor();

    Model model = LoadModel("model/cock.glb");
    
    int numberofanimiation = 3;
    ModelAnimation* modelAnim = LoadModelAnimations("model/cock.glb", &numberofanimiation);



    vector<UnitArmy> Units = {
        UnitArmy(Vector3{ 0.0f, 0.0f, 0.0f }, model, modelAnim[2], false),
        UnitArmy(Vector3{ 3.0f, 0.0f, 0.0f }, model, modelAnim[2], false),
        UnitArmy(Vector3{ 6.0f, 0.0f, 0.0f }, model, modelAnim[2], false)
    };

    Vector2 mouseStart = { 0.0f, 0.0f };
    Vector2 mouseEnd = { 0.0f, 0.0f };
    bool isSelecting = false;

    Vector3 p3 = { map->getTerrainNode( 0,0)->position.x , 0.0f , map->getTerrainNode(0,0)->position.y };
    Vector3 p2 = { map->getTerrainNode( 0, WIDTH-1)->position.x , 0.0f ,  map->getTerrainNode(0, WIDTH - 1)->position.y };
    Vector3 p1 = { map->getTerrainNode(HEIGHT - 1, WIDTH - 1)->position.x , 0.0f , map->getTerrainNode(HEIGHT - 1, WIDTH - 1)->position.y };
    Vector3 p4 = { map->getTerrainNode(HEIGHT - 1, 0)->position.x , 0.0f , map->getTerrainNode(HEIGHT - 1, 0)->position.y };
    #ifdef DEBUG

    printf("P1: (%.2f, %.2f, %.2f)\n", p1.x, p1.y, p1.z);
    printf("P2: (%.2f, %.2f, %.2f)\n", p2.x, p2.y, p2.z);
    printf("P3: (%.2f, %.2f, %.2f)\n", p3.x, p3.y, p3.z);
    printf("P4: (%.2f, %.2f, %.2f)\n", p4.x, p4.y, p4.z);

    #endif // DEBUG

    while (!WindowShouldClose())
    {

        // obs³uga gracza
        // Obrót kamery
        float camera_rotation_speed = 1.0f;
        if (IsKeyDown(KEY_Q))
        {
            Vector3 rotation = { - camera_rotation_speed , 0.0f, 0.0f };
            UpdateCameraPro(&camera, Vector3Zero(), rotation, 0.0f);
        }
        if (IsKeyDown(KEY_E))
        {
            Vector3 rotation = { camera_rotation_speed , 0.0f, 0.0f };
            UpdateCameraPro(&camera, Vector3Zero(), rotation, 0.0f);
        }

        Vector3 movement = { 0.0f, 0.0f, 0.0f };

        // Poruszanie kamery
        float camera_move_speed = 0.3f;
        if (IsKeyDown(KEY_W)) movement.x += camera_move_speed;
        if (IsKeyDown(KEY_S)) movement.x -= camera_move_speed;
        if (IsKeyDown(KEY_D)) movement.y += camera_move_speed;
        if (IsKeyDown(KEY_A)) movement.y -= camera_move_speed;

        // Kamera Zoom
        float camera_zoom_speed = 0.3f;
        float wheelMove = GetMouseWheelMove();
        if (wheelMove != 0)
        {
            camera.position = Vector3Add(camera.position,
                Vector3Scale(
                    Vector3Normalize(
                        Vector3Subtract(camera.target, camera.position)), wheelMove * camera_zoom_speed));
        }
        //Updatowanie pozycji kamery
        UpdateCameraPro(&camera, movement, Vector3Zero(), 0.0f);
        camera.target = Vector3Add(camera.position, Vector3Normalize(Vector3Subtract(camera.target, camera.position)));
       
        // Zaznaczanie jednostek
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
                RayCollision collision = GetRayCollisionBox(ray, box);
                if (collision.hit)
                {
                    Units[i].setIsSelected(!Units[i].getIsSelected());
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            Vector2 mousePos = GetMousePosition();
            Ray ray = GetMouseRay(mousePos, camera);

            RayCollision collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);
            if (collision.hit)
            {
                Vector3 baseGoal = collision.point;
                #ifdef DEBUG

                    printf("Wykryto kolizjê. Punkt kolizji: (%.2f, %.2f, %.2f)\n", baseGoal.x, baseGoal.y, baseGoal.z);

                #endif // DEBUG

                int selectedUnitCount = 0;

                // Liczenie zaznaczonych jednostek
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
                        Vector2 goalNode = map->FindNearestPathNode(Vector2{ formationGoal.x, formationGoal.z });
                        Vector3 position = Units[i].getPosition();

                        Vector2 positionNode = map->FindNearestPathNode(Vector2{ position.x, position.z });

                        AStarNode* path = map->AStar(positionNode.y,
                            positionNode.x,
                            goalNode.y,
                            goalNode.x
                        );

                        Units[i].setPath(path);
                        unitIndex++;
                    }
                }
            }
        }

        // Renderowanie gry
        BeginDrawing();

        ClearBackground(LIGHTGRAY);

        BeginMode3D(camera);

            map->drawTerrainGrid();
            //DrawGrid(WIDTH , SIZE_NODE);
            for (size_t i = 0; i < colliderCount; i++)
            {
                DrawBoundingBox(nonWalkableColliders[i], BLACK);
            }

            // Renderowanie jednostek
            for (int i = 0; i < Units.size(); i++)
            {
                Units[i].anim();
            }

        EndMode3D();
        
        //Rysowanie pola wyboru
        if (isSelecting)
        {
            float x = mouseEnd.x;
            float y = mouseEnd.y;
            float w = mouseStart.x - mouseEnd.x;
            float h = mouseStart.y - mouseEnd.y;
            if (w < 0) { x = mouseStart.x; w *= -1; }
            if (h < 0) { y = mouseStart.y; h *= -1; }
            DrawRectangleLinesEx(Rectangle{ x, y, w, h }, 1, Fade(BLACK, 0.8f));
            DrawRectangleRec(Rectangle{ x, y, w, h }, Fade(VIOLET, 0.3f));
        }
        
        // Rysowanie gui
        DrawText("RTS By MW", 10, 10, 20, DARKGRAY);
        DrawFPS( 10, 30);
        DrawText("WSAD - poruszanie kamery", 10, 100, 35, BLACK);
        DrawText("QE - obroy kamery", 10, 135, 35, BLACK);
        DrawText("Scrool Wheel - zoom kamery", 10, 170, 35, BLACK);
        DrawText("LPM - zaznaczanie jednostek", 10, 205, 35, BLACK);
        DrawText("PPM - danie rozkazu", 10, 240, 35, BLACK);

        EndDrawing();
    }

    UnloadModel(model);
    UnloadModelAnimations(modelAnim, numberofanimiation);
    CloseWindow();

    return 0;
}
