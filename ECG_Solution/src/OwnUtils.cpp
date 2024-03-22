#include "OwnUtils.h"


PxVec3 OwnUtils::glmModelMatrixToPxVec3(glm::mat4 modelMatrix) {

	return PxVec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);

}


glm::vec3 OwnUtils::calcFront(float yaw, float pitch) {
	
	glm::vec3 tmp;
	tmp.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	tmp.y = sin(glm::radians(pitch));
	tmp.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	return tmp;


}

PxMat33 OwnUtils::getOriMat(glm::mat4 modelMatrix) {
	
	return PxMat33(PxVec3(modelMatrix[0].x, modelMatrix[0].y, modelMatrix[0].z),
				   PxVec3(modelMatrix[1].x, modelMatrix[1].y, modelMatrix[1].z),
				   PxVec3(modelMatrix[2].x, modelMatrix[2].y, modelMatrix[2].z));

}
