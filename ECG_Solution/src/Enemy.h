#pragma once
#pragma once

#include <vector>
#include <PxPhysicsAPI.h>
#include "PhysicsWorld.h"

class Enemy
{
private:
	std::vector<physx::PxVec3> controlPoints;
	uint16_t controlPoint_index = 0;

public:
	Enemy(std::vector<physx::PxVec3> controlPoints, PxScene scene);

	std::vector<physx::PxVec3> getControlPoints();

	uint16_t getControlPointIndex();

	void updateControlPointIndex();
};
