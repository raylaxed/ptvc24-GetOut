#include "Player.h"
#include "Timer.h"
#include <glm/gtx/string_cast.hpp>

	


	Player::Player(Camera& camera) : _camera(camera) {
		
	}

	Player::~Player() {
	
	}

	void Player::setHand(Model& hand) {
		_skeleton_arm = &hand;
	}



	Camera* Player::getCamera() {
		return &_camera;
	}

	
	/*!
	* Updates the Position of the player
	* e.g. event like collision detection
	*/
	void Player::UpdatePosition(glm::vec3 position) {
		
		_camera.setPosition(position);
		UpdateHand();
		
	}


	void Player::ProcessMouseScroll(float offset){
		_camera.ProcessMouseScroll(offset);
	}

	void Player::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
		_camera.ProcessMouseMovement(xoffset, yoffset);
		UpdateHand();
	}

	void Player::UpdateHand(float degree)
	{
		
		glm::vec3 front = _camera.getFront();
		
		float yaw = _camera.getYaw() + 30.0f;
		float pitch = _camera.getPitch() - 15.0f;

		glm::vec3 position = _camera.getPosition();
		glm::vec3 frontWithOffset = OwnUtils::calcFront(yaw,pitch);
		position += frontWithOffset*0.5f;
		
		
		glm::mat4 mat = glm::rotate(glm::translate(glm::mat4(1.0f), position ),glm::radians(-yaw + 50.0f),glm::vec3(0.0f,1.0f,0.0f));
		mat = glm::rotate(mat, glm::radians(pitch + 15.0f - degree),glm::vec3(0.0f,0.0f,1.0f));
		mat = glm::rotate(mat, glm::radians(180.f),glm::vec3(0.0f,1.0f,0.0f));
	
		glm::vec3 size = glm::vec3(0.3f, 0.3f, 0.3f);
		_skeleton_arm->setModel(glm::scale(glm::mat4(1.f)*mat,size));

	}

	void Player::HandAnimation()
	{
		static Timer AnimationTimer = Timer();
		static float rotationdeg = 90.0f;

			
			if (AnimationTimer.Duration() ) {

				UpdateHand(rotationdeg);
				AnimationTimer.Reset();

			}
			else {
				rotationdeg = 45.f;
			}
		

			
			
		
	}

