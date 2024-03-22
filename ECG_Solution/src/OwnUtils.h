#pragma once

#include "Geometry.h"
#include "PxPhysicsAPI.h"
#include <algorithm>
#include <vector>

using namespace physx;

/*
this class is used to change datatypes between physx and glm
aswell as give methods for repetative calculations
*/
class OwnUtils {

public:

	//method to turn a modelmatrix into a position vector in PhysX
	static PxVec3 glmModelMatrixToPxVec3(glm::mat4 modelMatrix);

	static PxMat33 getOriMat(glm::mat4 modelMatrix);
	
	//calculate the front vector of the camera/arm
	static glm::vec3 calcFront(float yaw, float pitch);

};