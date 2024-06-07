#pragma once

#include "Enemy.h"
#include "PhysicsWorld.h"
#include <vector>

Enemy::Enemy(std::vector<physx::PxVec3> controlPoints, Model* enemyModel)
	:controlPoints(controlPoints), enemyModel(*enemyModel)
{
}

std::vector<physx::PxVec3> Enemy::getControlPoints()
{
	return controlPoints;
}

uint16_t Enemy::getControlPointIndex()
{
	return controlPoint_index;
}

void Enemy::updateControlPointIndex()
{
	if (controlPoint_index < sizeof(controlPoints))
	{
		controlPoint_index += 1;
	}
	return;
}

physx::PxVec3 Enemy::getControllPointByIndex(int index) {
	return controlPoints[index];
}
