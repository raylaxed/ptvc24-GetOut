#pragma once

#include "Camera.h"
#include "Geometry.h"
#include "Model.h"


/*!
* the player class is more or less a wrapper class
* it has a body (for collision detection), an arm and the main camera to apply changes to them
*/
class Player
{

private:
	Camera _camera;
	
	Model* _skeleton_arm;

public:

	Player(Camera& camera);
	
	~Player();

	void setHand(Model& hand);

	Model* getHand();

	Camera* getCamera();


	//Updates the Position of the player after an input	
	void UpdatePosition(glm::vec3 position);

	//Updates the Hand to move in relation to the position and camera
	void UpdateHand(float degree = 0);

	//animates the left click of the hand
	void HandAnimation();

	//currently not used, but could be used for a zoom feature
	void ProcessMouseScroll(float offset);

	//used to process mouse input
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	

};