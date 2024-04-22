#include "PhysicsWorld.h"
#include "Timer.h"
#include <ctime>
#include <glm/gtx/string_cast.hpp>

//TODO
//improve ball homing ability
//improve jumping

PhysicsWorld::PhysicsWorld() {}

void PhysicsWorld::initPhysics() {

	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f,-9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);
	PxTolerancesScale scale;
	scale.length = 3;        // typical length of an object
	scale.speed = 9.81;
	
	//must be set on the scene for the active actors array to be generated.
	PxSceneFlag::eENABLE_ACTIVE_ACTORS;

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.2f);

	//PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	//gScene->addActor(*groundPlane);

	gManager = PxCreateControllerManager(*gScene);
}


PxScene* PhysicsWorld::getScene() {

	return gScene;

}

int PhysicsWorld::getHitCounter() {

	return _hitCounter;
}

int PhysicsWorld::getScoreCounter() {

	return _scoreCounter;
}


void PhysicsWorld::setScoreCounter(unsigned int newScore) {

	_scoreCounter = newScore;

}

void PhysicsWorld::setHitCounter(unsigned int newHits) {

	_hitCounter = newHits;
	
}

bool PhysicsWorld::playerHasDashed() {

	return _hasDashed;
}


/*
* createShape takes the volume
* PxTransform the position
* hitboxes are from now on either spheres or cubes
* TODO implement cylinders eventually
*/
void PhysicsWorld::addCubeToPWorld(Geometry& obj, glm::vec3 measurements, bool isStatic, bool isTorchHitbox) {

	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModelMatrix());
	
	PxShape* tmpShape = gPhysics->createShape(PxBoxGeometry(measurements.x, measurements.y, measurements.z), *gMaterial);

	//add the object to the physx object
	if (isStatic) {
		if (!isTorchHitbox)
		{
			gObjects.push_back(&obj);
		}
		PxTransform x = PxTransform(position, PxQuat(OwnUtils::getOriMat(obj.getModelMatrix())));
		PxRigidStatic* cube = PxCreateStatic(*gPhysics, x , *tmpShape);
		cube->userData = (void*)&obj;
		gScene->addActor(*cube);
		pStaticObjects.push_back(cube);
	}
	//this sets the player, it is going to be the only dynamic cube in our world
	// disabling x and z axis for not falling over
	else {


		PxMaterial* playerMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
		PxShape* playerShape = gPhysics->createShape(PxBoxGeometry(measurements.x, measurements.y, measurements.z), *playerMaterial);
		PxRigidDynamic* playerHitbox = PxCreateDynamic(*gPhysics, PxTransform(position), *playerShape, 1);
		playerHitbox->userData = (void*)&obj;	
		playerHitbox->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
		playerHitbox->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);
		playerHitbox->setMaxLinearVelocity(10.0f);

		gScene->addActor(*playerHitbox);
		pDynamicObjects.push_back(playerHitbox);
		pPlayer = playerHitbox;
	}


}
void PhysicsWorld::addCubeToPWorld(Model& obj, glm::vec3 measurements, bool isStatic, bool isTorchHitbox) {

	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModel());

	PxShape* tmpShape = gPhysics->createShape(PxBoxGeometry(measurements.x, measurements.y, measurements.z), *gMaterial);

	//add the object to the physx object
	if (isStatic) {
		if (!isTorchHitbox)
		{
			gModels.push_back(&obj);
		}
		PxTransform x = PxTransform(position, PxQuat(OwnUtils::getOriMat(obj.getModel())));
		PxRigidStatic* cube = PxCreateStatic(*gPhysics, x, *tmpShape);
		cube->userData = (void*)&obj;
		gScene->addActor(*cube);
		pStaticObjects.push_back(cube);
	}
	//this sets the player, it is going to be the only dynamic cube in our world
	// disabling x and z axis for not falling over
	else {


		PxMaterial* playerMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
		PxShape* playerShape = gPhysics->createShape(PxBoxGeometry(measurements.x, measurements.y, measurements.z), *playerMaterial);
		PxRigidDynamic* playerHitbox = PxCreateDynamic(*gPhysics, PxTransform(position), *playerShape, 1);
		playerHitbox->userData = (void*)&obj;
		playerHitbox->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
		playerHitbox->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);
		playerHitbox->setMaxLinearVelocity(10.0f);

		gScene->addActor(*playerHitbox);
		pDynamicObjects.push_back(playerHitbox);
		pPlayer = playerHitbox;
	}
}

void PhysicsWorld::addPlayerToPWorld(Player& player, glm::vec3 measurements) {

	PxBoxControllerDesc desc;
	desc.halfSideExtent = measurements.x;
	desc.halfHeight = measurements.y;
	desc.halfForwardExtent = measurements.z;
	desc.stepOffset = 0.2f;
	desc.position = PxExtendedVec3(0.0f, 7.0f, 0.0f);
	desc.material = gMaterial;
	desc.userData = (void*)&player;
	controllerPlayer = gManager->createController(desc);
}


void PhysicsWorld::addSphereToPWorld(Geometry& obj, float radius, bool isStatic) {

	gObjects.push_back(&obj);
	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModelMatrix());
	PxShape* tmpShape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);

	//add the object to the physx object
	if (isStatic) {
		PxRigidStatic* sphere = PxCreateStatic(*gPhysics, PxTransform(position), *tmpShape);
		sphere->userData = (void*)&obj;
		gScene->addActor(*sphere);
		pStaticObjects.push_back(sphere);
	}
	//else clause sets THEHELLISHDODGEBALL
	else {
		PxRigidDynamic* THEHELLISHDODGEBALL = PxCreateDynamic(*gPhysics, PxTransform(position), *tmpShape, 1);
		pBall = THEHELLISHDODGEBALL;
		THEHELLISHDODGEBALL->setAngularVelocity(PxVec3(0.5f, 0.5f, 0.5f));
		THEHELLISHDODGEBALL->userData = (void*)&obj;
		THEHELLISHDODGEBALL->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		gScene->addActor(*THEHELLISHDODGEBALL);
		pDynamicObjects.push_back(THEHELLISHDODGEBALL);
	}
}


void PhysicsWorld::addSphereToPWorld(Model& obj, float radius, bool isStatic) {

	gModels.push_back(&obj);
	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModel());
	PxShape* tmpShape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);


	//add the object to the physx object
	if (isStatic) {
		PxRigidStatic* sphere = PxCreateStatic(*gPhysics, PxTransform(position), *tmpShape);
		sphere->userData = (void*)&obj;
		gScene->addActor(*sphere);
		pStaticObjects.push_back(sphere);
	}
	//else clause sets THEHELLISHDODGEBALL
	else {

		PxRigidDynamic* THEHELLISHDODGEBALL = PxCreateDynamic(*gPhysics, PxTransform(position), *tmpShape, 1);
		pBall = THEHELLISHDODGEBALL;
		THEHELLISHDODGEBALL->setAngularVelocity(PxVec3(0.5f, 0.5f, 0.5f));
		THEHELLISHDODGEBALL->userData = (void*)&obj;
		THEHELLISHDODGEBALL->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		gScene->addActor(*THEHELLISHDODGEBALL);
		pDynamicObjects.push_back(THEHELLISHDODGEBALL);
	}
}


//void PhysicsWorld::addEnemyToPWorld(Model& obj, Enemy& enem, float radius) {
//	gModels.push_back(&obj);
//	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModel());
//	PxShape* tmpShape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);
//
//	//add the object to the physx object
//	PxRigidDynamic* THEHELLISHDODGEBALL = PxCreateDynamic(*gPhysics, PxTransform(position), *tmpShape, 1);
//	pBall = THEHELLISHDODGEBALL;
//	THEHELLISHDODGEBALL->setAngularVelocity(PxVec3(0.5f, 0.5f, 0.5f));
//	THEHELLISHDODGEBALL->userData = (void*)&obj;
//	THEHELLISHDODGEBALL->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
//	gScene->addActor(*THEHELLISHDODGEBALL);
//	pDynamicObjects.push_back(THEHELLISHDODGEBALL);
//}

void PhysicsWorld::updatePlayer(Movement movement, float deltaTime) {

	static bool airborne = false;
	static bool gravityEnabled = true;
	static float dashFactor = 1.f;
	static Timer JumpTimer = Timer();
	static Timer DashTimer = Timer();
	static float velocity = -10.f *deltaTime;
	float gravity = -9.81f *deltaTime;

	if (airborne) {

		gravity += gravity *0.001f;
		
		if (gravityEnabled && velocity > -0.1) 
		{
			velocity += gravity / 15;
		}
	}
	else {
		gravity = -9.81f * 0.005f ;
		velocity = gravity * 2;
	}

	if (movement == PDASH && !_hasDashed) {
	
		dashFactor = 5.f;
		DashTimer.Reset();
		_hasDashed = true;
	}
	
	Player* playerObject = (Player*)controllerPlayer->getUserData();
	Camera* playerCamera = playerObject->getCamera();

	glm::vec3 frontVector = playerCamera->getFront();
	PxVec3 forward = PxVec3(frontVector.x, 0, frontVector.z).getNormalized() * deltaTime * 12.f;
	PxVec3 backward = PxVec3(-frontVector.x, 0, -frontVector.z).getNormalized() * deltaTime * 12.f;

	glm::vec3 rightVector = playerCamera->getRight();
	PxVec3 right = PxVec3(rightVector.x, 0, rightVector.z).getNormalized() * deltaTime * 12.f;
	PxVec3 left = PxVec3(-rightVector.x, 0, -rightVector.z).getNormalized() * deltaTime * 12.f;

	
	if (movement == PFORWARD) {
		controllerPlayer->move(dashFactor * forward,0.02f,deltaTime,NULL);
		
	}
	if (movement == PBACKWARD) {
		controllerPlayer->move(dashFactor * backward, 0.02f, deltaTime, NULL);
	}
	if (movement == PLEFT) {
		controllerPlayer->move(dashFactor * left, 0.02f, deltaTime, NULL);
	}
	if (movement == PRIGHT) {
		controllerPlayer->move(dashFactor * right, 0.02f, deltaTime, NULL);
	}
	if (movement == PJUMP && !airborne) {
		velocity = -7 * gravity;
		controllerPlayer->move(PxVec3(0.f, velocity, 0.f), 0.02f, deltaTime, NULL);
		JumpTimer.Reset();
		airborne = true;
	}

	if (DashTimer.Duration() > 0.2f) 
	{
		dashFactor = 1;

		if (DashTimer.Duration() > 2.5f) 
		{
			_hasDashed = false;
		}
	}

	if (JumpTimer.Duration() > 1.5f) {
		airborne = false;
		
		if (!gravityEnabled) {
			gravityEnabled = true;
		}
	}

	if (gravityEnabled) {
		controllerPlayer->move(PxVec3(0.f, velocity, 0.f), 0.02f, deltaTime, NULL);
	}
	
	PxExtendedVec3 position = controllerPlayer->getPosition();

	glm::vec3 newPos = glm::vec3(position.x, position.y, position.z);
	playerObject->UpdatePosition(newPos);
}

void updateEnemies(float deltaTime) {

}

glm::vec3 PhysicsWorld::getBallPosition() {

	PxVec3 tmp = pBall->getGlobalPose().p;
	return glm::vec3(tmp.x, tmp.y, tmp.z);
}

boolean PhysicsWorld::isPlayerHit() {
	
	float distance = calcDirectionBallPlayer().magnitude();
	//TODO dont hardcode ballradius
	return distance < 2.0;
}

boolean PhysicsWorld::isPlayerDead() {

	PxExtendedVec3 position = controllerPlayer->getFootPosition();
	return position.y <= 0.25f;

}

PxVec3 PhysicsWorld::calcDirectionBallPlayer() {

	PxVec3 ballPos = pBall->getGlobalPose().p;
	
	PxExtendedVec3 tmp = controllerPlayer->getPosition();
	PxVec3 playerPos = PxVec3(tmp.x,tmp.y,tmp.z);
	PxVec3 directionToPlayer =playerPos - ballPos;

	return directionToPlayer;
}


void PhysicsWorld::updateEnemy() {

	PxVec3 directionToPlayer = calcDirectionBallPlayer();

	pBall->addForce(directionToPlayer / (directionToPlayer.magnitude() * (20 - _hitCounter)), PxForceMode::eIMPULSE);
	PxVec3 position = pBall->getGlobalPose().p;
	
	glm::vec3 newPos = glm::vec3(position.x, position.y, position.z);

	Model* enemy = (Model*)pBall->userData;
	enemy->resetModelMatrix();
	enemy->setModel(glm::translate(glm::mat4(1.0f), newPos));
}


void PhysicsWorld::Animate(Player& player) {

	player.HandAnimation();

	PxVec3 directionToPlayer = calcDirectionBallPlayer();
	float distance = directionToPlayer.magnitude();

	if (distance < 3) {
		updateEnemy();
		//updateBall(true);
	}
}

void PhysicsWorld::draw() {

	Geometry* tmp;
	std::vector<Geometry*>::iterator it;
	
	for (it = gObjects.begin(); it != gObjects.end(); it++) {	
		tmp = *it;
		tmp->draw();	
	}
}

void PhysicsWorld::resetGame() {

	controllerPlayer->setPosition(PxExtendedVec3(5.0f, 7.0f, 35.0f));
	pBall->setAngularVelocity(PxVec3(0, 0, 0));
	pBall->setLinearVelocity(PxVec3(0, 0, 0));
	pBall->setGlobalPose(PxTransform(PxVec3(0.0f, 6.5f, -10.0f)));
	_hasDashed = false;
	setScoreCounter(0);
	setHitCounter(0);
}



