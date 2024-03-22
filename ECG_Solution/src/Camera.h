/*
* Copyright 2020 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the ECG Lab Framework and must not be redistributed.
*/
#pragma once


#include <memory>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <windows.h>
#include "OwnUtils.h"


// Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 10.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

/*!
 * Arc ball camera, modified by mouse input
 */
class Camera
{	
public:
	//
	int _mouseX, _mouseY;

	//euler angles
	float _yaw, _pitch;

	//camera vectors
	glm::vec3 _position;
	glm::vec3 _front;
	glm::vec3 _up;
	glm::vec3 _right;
	glm::vec3 _worldUp;
	glm::vec3 _strafe;
	glm::mat4 _projMatrix;

	//camera options
	float _movementSpeed;
	float _mouseSensitivity;
	float _zoom;

	/*!
	 * Camera constructor
	 * @param fov: field of view, in degrees
	 * @param aspect: aspect ratio
	 * @param near_plane_distance: near_plane_distance plane
	 * @param far_plane_distance: far_plane_distance plane
	 */

	 Camera(float fov, float aspect, float near, float far);

	//constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	//constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);


	~Camera();

	//returns view matrix 
	glm::mat4 GetViewMatrix();

	
	//getter methods for camera attributes and vectors
	glm::vec3 getPosition();

	glm::vec3 getFront();

	glm::vec3 getRight();

	glm::vec3 getUp();

	float getYaw();

	float getPitch();

	glm::mat4 getProjectionMatrix();

	//set new position of camera
	void setPosition(glm::vec3 position);

	void setProjectionMatrix(glm::mat4& projMat);


	//TODO: after gameover the camera should pan away from the player 
	void deathPan();

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	// process input revieved from the mousewheel. Expects the offset value in the y direction.
	void ProcessMouseScroll(float yoffset);
	

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
};