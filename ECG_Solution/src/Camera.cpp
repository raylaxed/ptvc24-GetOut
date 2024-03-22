#include "camera.h"

	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : _front(glm::vec3(0.0f, 0.0f, -1.0f)), _movementSpeed(SPEED), _mouseSensitivity(SENSITIVITY), _zoom(ZOOM)
	{
		_position = position;
		_worldUp = up;
		_yaw = yaw;
		_pitch = pitch;
		updateCameraVectors();
	}


	Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : _front(glm::vec3(0.0f, 0.0f, -1.0f)), _movementSpeed(SPEED), _mouseSensitivity(SENSITIVITY), _zoom(ZOOM)
	{
		_position = glm::vec3(posX, posY, posZ);
		_worldUp = glm::vec3(upX, upY, upZ);
		_yaw = yaw;
		_pitch = pitch;
		updateCameraVectors();
	}


	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(_position, _position + _front, _up);
	}

	glm::vec3 Camera::getPosition() 
	{
		return _position;
	};

	glm::vec3 Camera::getFront()
	{
		return _front;
	}
	glm::vec3 Camera::getRight()
	{
		return _right;
	}
	glm::vec3 Camera::getUp()
	{
		return _up;
	}


	float Camera::getYaw() 
	{
		return _yaw;
	}

	float Camera::getPitch()
	{
		return _pitch;
	}

	glm::mat4 Camera::getProjectionMatrix() {
	
		return _projMatrix;
	
	}

	void Camera::setPosition(glm::vec3 position){
		_position = position;
	
	}

	void Camera::setProjectionMatrix(glm::mat4& projMat) {
	
		_projMatrix = projMat;
	
	}


	void Camera::deathPan() {
	
		_position.z -= 1.f;
	
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
	{
		xoffset *= _mouseSensitivity;
		yoffset *= _mouseSensitivity;

		_yaw += xoffset;
		_pitch += yoffset;


		if (constrainPitch)
		{
			if (_pitch > 89.0f)
				_pitch = 89.0f;
			if (_pitch < -89.0f)
				_pitch = -89.0f;
		}

		updateCameraVectors();
	}

	void Camera::ProcessMouseScroll(float yoffset)
	{
		_zoom -= (float)yoffset;
		if (_zoom < 1.0f)
			_zoom = 1.0f;
		if (_zoom > 45.0f)
			_zoom = 45.0f;
	}



	void Camera::updateCameraVectors()
		{

			glm::vec3 front = OwnUtils::calcFront(_yaw,_pitch);
			_front = glm::normalize(front);
			_right = glm::normalize(glm::cross(_front, _worldUp));
			_up = glm::normalize(glm::cross(_right, _front));
		}