#include "UnitArmy.h"

void UnitArmy::updateAnimation() {
    if (animData.frameCount > 0) {
        animFrame += animSpeed;
        if (animFrame >= animData.frameCount) animFrame = 0.0f;
        UpdateModelAnimation(model, animData, (int)animFrame);
    }
}

void UnitArmy::anim() {
    float moveSpeed = 0.075f;
    Map* map = Map::getMap();

    if (path != nullptr) {
        Vector3 direction = Vector3Subtract(goal, position);
        float distance = Vector3Length(direction);

        if (!Vector3Equals(position, goal) && distance >= moveSpeed) {
            position = Vector3Add(position, Vector3Scale(Vector3Normalize(direction), moveSpeed));
        }
        else {
            path = path->parentNode;
            if (path != nullptr) {
                goal = Vector3{ map->getTerrainNode(path->nodeIndexX,path->nodeIndexY)->position.x, 0.0f, map->getTerrainNode(path->nodeIndexX,path->nodeIndexY)->position.y };
#ifdef DEBUG
                printf("Node Index: (%d, %d), gCost: %f, fCost: %f\n", path->nodeIndexX, path->nodeIndexY, path->gCost, path->fCost);
                printf("Nowy cel: (%.2f, %.2f)\n", goal.x, goal.z);
#endif // DEBUG
            }
        }
    }

    updateAnimation();
    rotateTowardsGoal();
    DrawModel(model, position, 0.2f, isSelected ? WHITE : LIGHTGRAY);
    DrawCircle3D(position, 0.1f, { 1.0f, 0.0f, 0.0f }, 90, YELLOW); // Draw a circle in 3D world space

}
