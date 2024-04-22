#include "Enemy.h"
#include <vector>
#include <PxPhysicsAPI.h>

Enemy::Enemy(std::vector<physx::PxVec3> controlPoints)
	:controlPoints(controlPoints)
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
