
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include "Map.h"

#pragma once

using namespace std;
class UnitArmy {
private:
    Vector3 position;
    Model model;
    ModelAnimation animData;
    bool isSelected;
    AStarNode* path = nullptr;
    Vector3 goal;
    float animFrame = 0.0f;
    float animSpeed = 1.0f;

    void rotateTowardsGoal() {
        Vector3 direction = Vector3Subtract(position, goal);
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
        Map* map = Map::getMap();
        this->path = path;
        if (path != nullptr) {
            goal = Vector3{ map->getTerrainNode(path->nodeIndexX,path->nodeIndexY)->position.x, 0.0f, map->getTerrainNode(path->nodeIndexX,path->nodeIndexY)->position.y };
        }
    }

    UnitArmy(const Vector3& position, const Model& model, const ModelAnimation& animData, bool isSelected)
        : position(position), model(model), animData(animData), isSelected(isSelected) {
        goal = position;
    }

    void updateAnimation();

    void anim();
};
