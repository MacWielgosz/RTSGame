#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "UnitArmy.h"

//#define DEBUG
using namespace std;
constexpr auto SIZE_NODE = 1.0f;
constexpr auto HEIGHT = 50;
constexpr auto WIDTH = 50;
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

struct PathNode {
    Vector2 position;
    int connections[8][2];
    int connectionCount;
};

struct AStarNode {
    int nodeIndexX, nodeIndexY;
    float gCost;
    float fCost;
    AStarNode* parentNode;
};

PathNode terrainGrid[HEIGHT][WIDTH];


class UnitArmy {
private:
    Vector3 position;
    Model model;
    ModelAnimation animData;
    bool isSelected;
    float collisionRadius = 0.7f;
    AStarNode* path = nullptr;
    Vector3 goal;
    float animFrame = 0.0f;
    float animSpeed = 1.0f;

    void rotateTowardsGoal() {
        Vector3 direction = Vector3Subtract(position, goal );
        if (Vector3Length(direction) > 0.1f) {
            direction = Vector3Normalize(direction);
            float angle = atan2f(direction.x, direction.z) * RAD2DEG;
            model.transform = MatrixRotateY(angle * DEG2RAD);
        }
    }

public:
    bool getIsSelected() const { return isSelected; }
    void setIsSelected(bool isSelected) { this->isSelected = isSelected; }
    Vector3 getPosition() const { return position; }

    void setPath(AStarNode* path) {
        this->path = path;
        if (path != nullptr) {
            goal = Vector3{ terrainGrid[path->nodeIndexX][path->nodeIndexY].position.x, 0.0f, terrainGrid[path->nodeIndexX][path->nodeIndexY].position.y };
        }
    }

    float getCollisionRadius() const { return collisionRadius; }

    UnitArmy(const Vector3& position, const Model& model, const ModelAnimation& animData, bool isSelected)
        : position(position), model(model), animData(animData), isSelected(isSelected) {
        goal = position;
        //model.transform = MatrixIdentity(); // Inicjalizacja transformacji modelu
    }

    void updateAnimation() {
        if (animData.frameCount > 0) {
            animFrame += animSpeed;
            if (animFrame >= animData.frameCount) animFrame = 0.0f;
            UpdateModelAnimation(model, animData, (int)animFrame);
        }
    }

    void anim() {
        float moveSpeed = 0.075f;

        if (path != nullptr) {
            Vector3 direction = Vector3Subtract(goal, position);
            float distance = Vector3Length(direction);

            if (!Vector3Equals(position, goal) && distance >= moveSpeed) {
                position = Vector3Add(position, Vector3Scale(Vector3Normalize(direction), moveSpeed));
    }
            else {
                path = path->parentNode;
                if (path != nullptr) {
                    goal = Vector3{ terrainGrid[path->nodeIndexX][path->nodeIndexY].position.x, 0.0f, terrainGrid[path->nodeIndexX][path->nodeIndexY].position.y };
#ifdef DEBUG
                    printf("Node Index: (%d, %d), gCost: %f, fCost: %f\n", path->nodeIndexX, path->nodeIndexY, path->gCost, path->fCost);
                    printf("Nowy cel: (%.2f, %.2f)\n", goal.x, goal.z);
#endif // DEBUG
                }
            }
}

        updateAnimation();
        rotateTowardsGoal();
        DrawModel(model, position, .2f, isSelected ? WHITE : LIGHTGRAY);
    }
};

// Funkcja heurystyczna - u¿ywamy odleg³oœci Manhattan
float Heuristic(Vector2 a, Vector2 b) {
    return fabsf(a.x - b.x) + fabsf(a.y - b.y);
}

// Znajdowanie wêz³a z najni¿szym fCost na liœcie
int GetLowestFCostIndex(AStarNode* nodes, int count) {
    int lowestIndex = 0;
    for (int i = 1; i < count; i++) {
        if (nodes[i].fCost < nodes[lowestIndex].fCost) {
            lowestIndex = i;
        }
    }
    return lowestIndex;
}

// Sprawdza, czy lista zawiera wêze³ o danym indeksie
bool ContainsNode(AStarNode* nodes, int count, int nodeIndexX, int nodeIndexY) {
    for (int i = 0; i < count; i++) {
        if (nodes[i].nodeIndexX == nodeIndexX && nodes[i].nodeIndexY == nodeIndexY) {
            return true;
        }
    }
    return false;
}

// Znajdowanie wêz³a w liœcie
AStarNode* FindNode(AStarNode* nodes, int count, int nodeIndexX, int nodeIndexY) {
    for (int i = 0; i < count; i++) {
        if (nodes[i].nodeIndexX == nodeIndexX && nodes[i].nodeIndexY == nodeIndexY) {
            return &nodes[i];
        }
    }
    return NULL;
}

AStarNode* AStar(int startX, int startY, int goalX, int goalY) {
    AStarNode* openList = (AStarNode*)malloc(HEIGHT * WIDTH * sizeof(AStarNode));
    AStarNode* closedList = (AStarNode*)malloc(HEIGHT * WIDTH * sizeof(AStarNode));
    int openCount = 0;
    int closedCount = 0;

    // Inicjalizacja wêz³a startowego
    openList[openCount++] = AStarNode{ startX, startY, 0.0f, Heuristic(terrainGrid[startY][startX].position, terrainGrid[goalY][goalX].position), NULL };

    while (openCount > 0) {
        // ZnajdŸ wêze³ z najni¿szym fCost
        int currentIndex = GetLowestFCostIndex(openList, openCount);
        AStarNode currentNode = openList[currentIndex];

        // Przenieœ currentNode z openList do closedList
        openList[currentIndex] = openList[--openCount];
        closedList[closedCount++] = currentNode;

        // Jeœli osi¹gnêliœmy cel, zwróæ œcie¿kê
        if (currentNode.nodeIndexX == goalX && currentNode.nodeIndexY == goalY) {
            AStarNode* path = (AStarNode*)malloc(closedCount * sizeof(AStarNode));
            AStarNode* node = &closedList[closedCount - 1];
            int pathCount = 0;
            while (node != NULL) {
                path[pathCount++] = *node;
                node = node->parentNode;
            }

            // Ustaw wskaŸniki parentNode, aby wskazywa³y na nastêpny wêze³
            for (int i = pathCount - 1; i > 0; i--) {
                path[i].parentNode = &path[i - 1];
            }
            path[0].parentNode = NULL;

            free(openList);
            free(closedList);
            return &path[pathCount - 1]; // Zwracamy wskaŸnik do pierwszego elementu œcie¿ki
        }

        // Przetwarzaj s¹siadów
        for (int i = 0; i < terrainGrid[currentNode.nodeIndexY][currentNode.nodeIndexX].connectionCount; i++) {
            int neighborX = terrainGrid[currentNode.nodeIndexY][currentNode.nodeIndexX].connections[i][0];
            int neighborY = terrainGrid[currentNode.nodeIndexY][currentNode.nodeIndexX].connections[i][1];

            // Jeœli s¹siad jest ju¿ w closedList, pomiñ
            if (ContainsNode(closedList, closedCount, neighborX, neighborY)) continue;

            float tentativeGCost = currentNode.gCost + Heuristic(terrainGrid[currentNode.nodeIndexY][currentNode.nodeIndexX].position, terrainGrid[neighborY][neighborX].position);

            AStarNode* neighborNode = FindNode(openList, openCount, neighborX, neighborY);
            if (neighborNode == NULL) {
                // Dodaj nowy wêze³ do openList
                openList[openCount++] = AStarNode{
                    neighborX, neighborY,
                    tentativeGCost,
                    tentativeGCost + Heuristic(terrainGrid[neighborY][neighborX].position, terrainGrid[goalY][goalX].position),
                    &closedList[closedCount - 1]
                };
            }
            else if (tentativeGCost < neighborNode->gCost) {
                // Aktualizuj koszt i rodzica istniej¹cego wêz³a
                neighborNode->gCost = tentativeGCost;
                neighborNode->fCost = tentativeGCost + Heuristic(terrainGrid[neighborY][neighborX].position, terrainGrid[goalY][goalX].position);
                neighborNode->parentNode = &closedList[closedCount - 1];
            }
        }
    }

    free(openList);
    free(closedList);
    return NULL;  // Brak œcie¿ki
}



Vector2 FindNearestPathNode(Vector2 position) {
    Vector2 nearestIndex = { -1, -1 };
    float nearestDistance = FLT_MAX;

    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            float distance = Vector2Distance(position, terrainGrid[y][x].position);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestIndex.x = x;
                nearestIndex.y = y;
            }
        }
    }

    return nearestIndex;
}

void initTerrainGrid() {
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++) {
            Vector2 position = { (float)i * SIZE_NODE - (HEIGHT * SIZE_NODE / 2), (float)j * SIZE_NODE - (WIDTH * SIZE_NODE / 2) };
            terrainGrid[i][j].position = position;
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

void FindNeighbors(int nodeIndexX, int nodeIndexY, BoundingBox* nonWalkableColliders, int colliderCount) {
    int x = nodeIndexX;
    int y = nodeIndexY;
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
            bool hasCollision = false;

        if (nx >= 0 && ny >= 0 && nx < WIDTH && ny < HEIGHT) {
            PathNode* neighbour = &terrainGrid[ny][nx];
            Vector2 neighbourPosition = neighbour->position;

            float distance = Vector2Distance(position, neighbourPosition);
            Vector2 rayDir = Vector2Normalize(Vector2Subtract(neighbourPosition, position));
            Ray connectionRay = { Vector3 { position.x, 0.0f, position.y }, Vector3 { rayDir.x, 0.0f, rayDir.y } };

            for (int j = 0; j < colliderCount; j++) {
                RayCollision collision = GetRayCollisionBox(connectionRay, nonWalkableColliders[j]);
                if (collision.hit && collision.distance<= distance) {
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


    BoundingBox nonWalkableColliders[] = {
        { Vector3 { 5.0f, -0.2f, 5.0f }, Vector3 { 8.0f, 1.0f, 8.0f } }, // Przyk³adowa przeszkoda 1
        { Vector3 { 7.0f, -0.2f, 7.0f }, Vector3 { 12.0f, 1.0f, 12.0f } } // Przyk³adowa przeszkoda 2
    };

    int colliderCount = ARRAY_SIZE(nonWalkableColliders);


    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            FindNeighbors(i,j, nonWalkableColliders, colliderCount);
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
        UnitArmy(Vector3{ 3.0f, 0.0f, 0.0f }, model,modelAnim[2], false),
        UnitArmy(Vector3{ 6.0f, 0.0f, 0.0f }, model,modelAnim[2], false)
    };

    Vector2 mouseStart = { 0.0f, 0.0f };
    Vector2 mouseEnd = { 0.0f, 0.0f };
    bool isSelecting = false;

    Vector3 p3 = { terrainGrid[0][0].position.y , 0.0f , terrainGrid[0][0].position.x };
    Vector3 p2 = { terrainGrid[0][WIDTH-1].position.y , 0.0f , terrainGrid[0][WIDTH - 1].position.x };
    Vector3 p1 = { terrainGrid[HEIGHT - 1][WIDTH - 1].position.y , 0.0f , terrainGrid[HEIGHT - 1][WIDTH - 1].position.x };
    Vector3 p4 = { terrainGrid[HEIGHT - 1][0].position.y , 0.0f , terrainGrid[HEIGHT - 1][0].position.x };
    #ifdef DEBUG

    printf("P1: (%.2f, %.2f, %.2f)\n", p1.x, p1.y, p1.z);
    printf("P2: (%.2f, %.2f, %.2f)\n", p2.x, p2.y, p2.z);
    printf("P3: (%.2f, %.2f, %.2f)\n", p3.x, p3.y, p3.z);
    printf("P4: (%.2f, %.2f, %.2f)\n", p4.x, p4.y, p4.z);

    #endif // DEBUG

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
                        Vector2 goalNode = FindNearestPathNode(Vector2{ formationGoal.x, formationGoal.z });
                        Vector3 position = Units[i].getPosition();

                        Vector2 positionNode = FindNearestPathNode(Vector2{ position.x, position.z });

                        AStarNode* path = AStar(positionNode.y,
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


        BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        drawTerrainGrid();
        for (size_t i = 0; i < colliderCount; i++)
        {
            DrawBoundingBox(nonWalkableColliders[i], BLACK);

        }
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

    UnloadModel(model);
    UnloadModelAnimations(modelAnim, numberofanimiation);
    CloseWindow();

    return 0;
}
