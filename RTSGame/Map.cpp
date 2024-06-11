#include "Map.h"

// Inicjalizacja statycznych zmiennych cz³onkowskich
Map* Map::map = nullptr;
PathNode Map::terrainGrid[HEIGHT][WIDTH];

void Map::drawTerrainGrid() {
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++) {
            PathNode* node = &terrainGrid[i][j];
            Vector3 nodePos = { node->position.x, 0.0f, node->position.y };

            // Rysowanie ko³a w miejscu wêz³a
            DrawCircle3D(nodePos, 0.1f, { 1.0f, 0.0f, 0.0f }, 90.0f, BLUE);

            // Rysowanie po³¹czeñ miêdzy s¹siadami
            for (int k = 0; k < node->connectionCount; k++) {
                int nx = node->connections[k][0];
                int ny = node->connections[k][1];
                PathNode* neighbour = &terrainGrid[nx][ny];
                Vector3 neighbourPos = { neighbour->position.x, 0.0f, neighbour->position.y };
                DrawLine3D(nodePos, neighbourPos, DARKGRAY);
            }
        }
    }
}

AStarNode* Map::AStar(int startX, int startY, int goalX, int goalY) {
    AStarNode* openList = (AStarNode*)malloc(HEIGHT * WIDTH * sizeof(AStarNode));
    AStarNode* closedList = (AStarNode*)malloc(HEIGHT * WIDTH * sizeof(AStarNode));
    int openCount = 0;
    int closedCount = 0;

    // Inicjalizacja wêz³a startowego
    openList[openCount++] = AStarNode{ startX, startY, 0.0f, Heuristic(terrainGrid[startX][startY].position, terrainGrid[goalX][goalY].position) };

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
        for (int i = 0; i < terrainGrid[currentNode.nodeIndexX][currentNode.nodeIndexY].connectionCount; i++) {
            int neighborX = terrainGrid[currentNode.nodeIndexX][currentNode.nodeIndexY].connections[i][0];
            int neighborY = terrainGrid[currentNode.nodeIndexX][currentNode.nodeIndexY].connections[i][1];

            // Jeœli s¹siad jest ju¿ w closedList, pomiñ
            if (ContainsNode(closedList, closedCount, neighborX, neighborY)) continue;

            float tentativeGCost = currentNode.gCost + Heuristic(terrainGrid[currentNode.nodeIndexX][currentNode.nodeIndexY].position, terrainGrid[neighborX][neighborY].position);

            AStarNode* neighborNode = FindNode(openList, openCount, neighborX, neighborY);
            if (neighborNode == NULL) {
                // Dodaj nowy wêze³ do openList
                openList[openCount++] = AStarNode{
                    neighborX, neighborY,
                    tentativeGCost,
                    tentativeGCost + Heuristic(terrainGrid[neighborX][neighborY].position, terrainGrid[goalX][goalY].position),
                    &closedList[closedCount - 1]
                };
            }
            else if (tentativeGCost < neighborNode->gCost) {
                // Aktualizuj koszt i rodzica istniej¹cego wêz³a
                neighborNode->gCost = tentativeGCost;
                neighborNode->fCost = tentativeGCost + Heuristic(terrainGrid[neighborX][neighborY].position, terrainGrid[goalX][goalY].position);
                neighborNode->parentNode = &closedList[closedCount - 1];
            }
        }
    }

    free(openList);
    free(closedList);
    return NULL;  // Brak œcie¿ki
}

void Map::FindNeighbors(int nodeIndexX, int nodeIndexY, BoundingBox* nonWalkableColliders, int colliderCount) {
    int x = nodeIndexX;
    int y = nodeIndexY;
    PathNode* node = &terrainGrid[x][y];
    Vector2 position = node->position;

    static const int delta[8][2] = {
        { -1, -1 },{ 0, -1 },{ 1, -1 },
        { -1,  0 },{ 1,  0 },
        { -1,  1 },{ 0,  1 },{ 1,  1 }
    };

    int connectionIndex = 0;

    for (int i = 0; i < 8; i++) {
        int nx = x + delta[i][0];
        int ny = y + delta[i][1];
        bool hasCollision = false;

        if (nx >= 0 && ny >= 0 && nx < WIDTH && ny < HEIGHT) {
            PathNode* neighbour = &terrainGrid[nx][ny];
            Vector2 neighbourPosition = neighbour->position;

            float distance = Vector2Distance(position, neighbourPosition);
            Vector2 rayDir = Vector2Normalize(Vector2Subtract(neighbourPosition, position));
            Ray connectionRay = { Vector3{ position.x, 0.0f, position.y }, Vector3{ rayDir.x, 0.0f, rayDir.y } };

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

Vector2 Map::FindNearestPathNode(Vector2 position) {
    Vector2 nearestIndex = { -1, -1 };
    float nearestDistance = FLT_MAX;

    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            float distance = Vector2Distance(position, terrainGrid[x][y].position);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestIndex.x = x;
                nearestIndex.y = y;
            }
        }
    }

    return nearestIndex;
}

void Map::initTerrainGrid() {
    for (size_t i = 0; i < HEIGHT; i++) {
        for (size_t j = 0; j < WIDTH; j++) {
            Vector2 position = { (float)i * SIZE_NODE - (HEIGHT * SIZE_NODE / 2), (float)j * SIZE_NODE - (WIDTH * SIZE_NODE / 2) };
            terrainGrid[i][j].position = position;
        }
    }
}

// Znajdowanie wêz³a z najni¿szym fCost na liœcie

int Map::GetLowestFCostIndex(AStarNode* nodes, int count) {
    int lowestIndex = 0;
    for (int i = 1; i < count; i++) {
        if (nodes[i].fCost < nodes[lowestIndex].fCost) {
            lowestIndex = i;
        }
    }
    return lowestIndex;
}

// Sprawdza, czy lista zawiera wêze³ o danym indeksie

bool Map::ContainsNode(AStarNode* nodes, int count, int nodeIndexX, int nodeIndexY) {
    for (int i = 0; i < count; i++) {
        if (nodes[i].nodeIndexX == nodeIndexX && nodes[i].nodeIndexY == nodeIndexY) {
            return true;
        }
    }
    return false;
}

// Znajdowanie wêz³a w liœcie

AStarNode* Map::FindNode(AStarNode* nodes, int count, int nodeIndexX, int nodeIndexY) {
    for (int i = 0; i < count; i++) {
        if (nodes[i].nodeIndexX == nodeIndexX && nodes[i].nodeIndexY == nodeIndexY) {
            return &nodes[i];
        }
    }
    return NULL;
}

// Funkcja heurystyczna - u¿ywamy odleg³oœci Manhattan

float Map::Heuristic(Vector2 a, Vector2 b) {
    return fabsf(a.x - b.x) + fabsf(a.y - b.y);
}
