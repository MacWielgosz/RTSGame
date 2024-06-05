#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include "raylib.h"
#include "raymath.h"
#include <iostream>
using namespace std;

#ifndef UNITS_ARMY
#define UNITS_ARMY

struct PathNode {
    Vector2 position;// Pozycja wêz³a
    int connections[8][2];
    int connectionCount;
};

struct AStarNode
{
    int nodeIndex;
    float gCost;
    float fCost;
    int pathIndex;
    AStarNode* parentNode;
};

class UnitArmy {
private:
    Vector3 position;
    Model model;
    bool isSelected;
    Vector3 goal;
    float collisionRadius = 0.7f; // Promieñ kolizji dla ka¿dej jednostki
    ModelAnimation* modelAnim;
    size_t currentPathIndex = 0; // Indeks aktualnego punktu na œcie¿ce

    void rotateTowardsGoal() {
        Vector3 direction = Vector3Subtract(goal, position);
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
    void setGoal(Vector3 goal) {
        this->goal=goal;
    }

    float getCollisionRadius() const { return collisionRadius; }

    UnitArmy(const Vector3& position, const Model& model, bool isSelected)
        : position(position), model(model), isSelected(isSelected) {
        this->goal = this->position;
    }

    void anim() {
        float moveSpeed = 0.075f;

        if (!Vector3Equals(position, goal)) {
            Vector3 direction = Vector3Subtract(goal, position);
            float distance = Vector3Length(direction);

            if (distance >= moveSpeed) {
                position = Vector3Add(position, Vector3Scale(Vector3Normalize(direction), moveSpeed));
            }
        }
        rotateTowardsGoal();
        DrawModel(model, position, 1.0f, isSelected ? WHITE : LIGHTGRAY);
    }

};

#endif // UNITS_ARMY
