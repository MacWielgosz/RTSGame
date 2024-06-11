#pragma once
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <cfloat>
#include <vector>
#ifndef SIZE_NODE
#define SIZE_NODE 1.0f
#endif // !1
#ifndef HEIGHT
#define HEIGHT 50
#endif
#ifndef WIDTH
#define WIDTH 50
#endif

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

class Map
{
private:
    static Map* map;
    static PathNode terrainGrid[HEIGHT][WIDTH];

public:
    PathNode* getTerrainNode(unsigned int x, unsigned int y) {
        return &terrainGrid[y][x];
    }
    static Map* getMap() {
        if (map == nullptr)
        {
            map = new Map();
        }
        return map;
    }

    void drawTerrainGrid();

    AStarNode* AStar(int startX, int startY, int goalX, int goalY);

    void FindNeighbors(int nodeIndexX, int nodeIndexY, BoundingBox* nonWalkableColliders, int colliderCount);

    Vector2 FindNearestPathNode(Vector2 position);
private:
    Map() {
        initTerrainGrid();
    }
    static void initTerrainGrid();

    bool PointOutsideMap(Vector3 point) {
        return point.x < 0 || point.y < 0 || point.x >= WIDTH * SIZE_NODE || point.y >= HEIGHT * SIZE_NODE;
    }
    // Znajdowanie wêz³a z najni¿szym fCost na liœcie
    int GetLowestFCostIndex(AStarNode* nodes, int count);

    // Sprawdza, czy lista zawiera wêze³ o danym indeksie
    bool ContainsNode(AStarNode* nodes, int count, int nodeIndexX, int nodeIndexY);

    // Znajdowanie wêz³a w liœcie
    AStarNode* FindNode(AStarNode* nodes, int count, int nodeIndexX, int nodeIndexY);

    // Funkcja heurystyczna - u¿ywamy odleg³oœci Manhattan
    float Heuristic(Vector2 a, Vector2 b);

};

