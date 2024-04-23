#pragma once

#include <vector>
#include <PxPhysicsAPI.h>
#include "Model.h"

class Enemy
{
private:
	std::vector<physx::PxVec3> controlPoints;
	uint16_t controlPoint_index = 0;
	Model enemyModel;

public:
	Enemy(std::vector<physx::PxVec3> controlPoints, Model* enemyModel);

	std::vector<physx::PxVec3> getControlPoints();

	uint16_t getControlPointIndex();

	void updateControlPointIndex();
};
