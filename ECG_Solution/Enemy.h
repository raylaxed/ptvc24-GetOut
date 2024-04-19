#pragma once

#include <vector>
#include <PxPhysicsAPI.h>

class Enemy
{
private:
	std::vector<physx::PxVec3> controlPoints;
	uint16_t controlPoint_index = 0;

public:
	Enemy(std::vector<physx::PxVec3> controlPoints);

	std::vector<physx::PxVec3> getControlPoints();

	void updateControlPointIndex();
};
