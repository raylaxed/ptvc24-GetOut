#pragma once

#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include <vector>
#include <memory>
#include "Enemy.h"
#include "PxPhysicsAPI.h"
#include "Geometry.h"
#include "OwnUtils.h"
#include "Player.h"
using namespace physx;

//Abstraction of player movement 
enum Movement {

	PFORWARD,
	PBACKWARD,
	PLEFT,
	PRIGHT,
	PJUMP,
	PNOMOVEMENT,
	PSPRINT
};

class PhysicsWorld
{
private:

	//necessary init variables for PhysX Foundation
	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;

	PxFoundation* gFoundation = nullptr;
	PxPhysics* gPhysics = nullptr;

	PxDefaultCpuDispatcher* gDispatcher = nullptr;
	PxScene* gScene = nullptr;

	PxMaterial* gMaterial = nullptr;
	PxPvd* gPvd = nullptr;

	PxControllerManager* gManager = nullptr;
	PxController* controllerPlayer = nullptr;

	std::vector<Geometry*> gObjects;
	std::vector<Model*> gModels;
	std::vector<PxRigidStatic*> pStaticObjects;
	std::vector<PxRigidDynamic*> pDynamicObjects;

	//the rigidbody dynamics 
	PxRigidDynamic* pPlayer;
	PxRigidDynamic* pTestEnemy;

	// these vectors are for storing an Enemy and the corresponding rigidbody dynamics
	std::vector<Enemy*> movingEnemies;
	std::vector<PxRigidDynamic*> enemyDynamics;

	//variables for keeping track
	float _downForce = -10.f;
	float _UpForce = 40.f;
	int _hitCounter = 0;
	int _scoreCounter = 0;
	boolean foundKey = false;
	PxVec3 keyPosition = PxVec3(0.0f, 0.0f, 0.0f);
	
public:
	PhysicsWorld();
	
	//initializes PhysX context
	void initPhysics();

	//returns the simulation scene
	PxScene* getScene();

	// sets the positon for the key in the Physics World
	void setKeyPosition(PxVec3 position);

	//add a Cube Geometry object into the simulation as a rigidbody
	void addCubeToPWorld(Geometry& obj, glm::vec3 measurements, bool isStatic = true,bool isTorchHitbox = false);
	
	void addCubeToPWorld(Model& obj, glm::vec3 measurements, bool isStatic = true, bool isTorchHitbox = false);

	void addPlayerToPWorld(Player& player, glm::vec3 measurements);

	//add a Sphere Geometry object into the simulation as a rigidbody
	void addSphereToPWorld(Geometry& obj, float radius, bool isStatic = true);

	void addSphereToPWorld(Model& obj, float radius, bool isStatic);

	void addEnemyToPWorld(Model& obj, Enemy& enem, float radius);

	//updates the Player in the rendered World
	void updatePlayer(Movement movement, float deltaTime);

	// updates the patroling enemies
	void updateEnemies(float deltaTime);

	// updates the single brain enemy
	void updateEnemy();

	//boolean wether the player is dead or alive 
	boolean isPlayerDead();

	glm::vec3 getEnemyPosition();

	//checks if player collides with the ball
	boolean isPlayerHit();

	// checks if the player found the key and got close enough to win
	boolean playerFoundKey();

	//calculates the vector from ball to player
	PxVec3 calcDirectionEnemyPlayer();

	//calculates the vector from enemy on path to player
	PxVec3 calcDirectionEnemyPlayer(PxVec3 position);

	//draws all added objects in the rendered world
	void draw();

	//resets ball, player and ball velocity 
	void resetGame();
};

#endif // PHYSICS_WORLD_H
