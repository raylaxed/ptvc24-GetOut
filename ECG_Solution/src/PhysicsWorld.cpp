#include "PhysicsWorld.h"
#include "Timer.h"
#include <ctime>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>



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
	desc.position = PxExtendedVec3(0.0f, 3.0f, 0.0f);
	desc.material = gMaterial;
	desc.userData = (void*)&player;
	controllerPlayer = gManager->createController(desc);
}


void PhysicsWorld::addSphereToPWorld(Geometry& obj, float radius, bool isStatic) {

	gObjects.push_back(&obj);
	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModelMatrix());
	PxShape* tmpShape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);

	if (isStatic) {
		PxRigidStatic* sphere = PxCreateStatic(*gPhysics, PxTransform(position), *tmpShape);
		sphere->userData = (void*)&obj;
		gScene->addActor(*sphere);
		pStaticObjects.push_back(sphere);
	}
	else {
		PxRigidDynamic* Enemy = PxCreateDynamic(*gPhysics, PxTransform(position), *tmpShape, 1);
		pTestEnemy = Enemy;
		Enemy->setAngularVelocity(PxVec3(0.5f, 0.5f, 0.5f));
		Enemy->userData = (void*)&obj;
		Enemy->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		gScene->addActor(*Enemy);
		pDynamicObjects.push_back(Enemy);
	}
}


void PhysicsWorld::addSphereToPWorld(Model& obj, float radius, bool isStatic) {

	gModels.push_back(&obj);
	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModel());
	PxShape* tmpShape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);

	if (isStatic) {
		PxRigidStatic* sphere = PxCreateStatic(*gPhysics, PxTransform(position), *tmpShape);
		sphere->userData = (void*)&obj;
		gScene->addActor(*sphere);
		pStaticObjects.push_back(sphere);
	}
	
	else {

		PxRigidDynamic* Enemy = PxCreateDynamic(*gPhysics, PxTransform(position), *tmpShape, 1);
		pTestEnemy = Enemy;
		Enemy->setAngularVelocity(PxVec3(0.5f, 0.5f, 0.5f));
		Enemy->userData = (void*)&obj;
		Enemy->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		gScene->addActor(*Enemy);
		pDynamicObjects.push_back(Enemy);
	}
}


void PhysicsWorld::addEnemyToPWorld(Model& obj, Enemy& enem, float radius) {

	//Model
	gModels.push_back(&obj);
	PxVec3 position = OwnUtils::glmModelMatrixToPxVec3(obj.getModel());
	PxShape* tmpShape = gPhysics->createShape(PxSphereGeometry(radius), *gMaterial);

	//Enemy
	movingEnemies.push_back(&enem);

	//add the object to the physx object
	PxRigidDynamic* dyn = PxCreateDynamic(*gPhysics, PxTransform(position), *tmpShape, 1);
	dyn->userData = (void*)&obj;
	dyn->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	dyn->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	gScene->addActor(*dyn);
	enemyDynamics.push_back(dyn);
}

void PhysicsWorld::updatePlayer(Movement movement, float deltaTime) {

	static bool airborne = false;
	static bool gravityEnabled = true;

	static Timer JumpTimer = Timer();
	
	static float velocity = 0.f;
	float gravity = -9.81f;

	// Update gravity effect if airborne
	if (airborne) {
		velocity += gravity * deltaTime; // Properly scale velocity by deltaTime
	}
	else {
		velocity = 0; // Reset velocity if not airborne
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
		controllerPlayer->move(forward,0.02f,deltaTime,NULL);
		
	}
	if (movement == PBACKWARD) {
		controllerPlayer->move( backward, 0.02f, deltaTime, NULL);
	}
	if (movement == PLEFT) {
		controllerPlayer->move(left, 0.02f, deltaTime, NULL);
	}
	if (movement == PRIGHT) {
		controllerPlayer->move(right, 0.02f, deltaTime, NULL);
	}
	if (movement == PJUMP && !airborne) {
		velocity = 7.0f; // Set initial jump velocity
		airborne = true;
		JumpTimer.Reset();
	}

	// Apply gravity
	if (gravityEnabled) {
		controllerPlayer->move(PxVec3(0.f, velocity * deltaTime, 0.f), 0.02f, deltaTime, NULL);
	}

	// Check if the player is no longer airborne
	if (controllerPlayer->getPosition().y <= 3.5) {
		airborne = false;
		gravityEnabled = true;
	}
	
	PxExtendedVec3 position = controllerPlayer->getPosition();

	glm::vec3 newPos = glm::vec3(position.x, position.y, position.z);
	playerObject->UpdatePosition(newPos);
}

void PhysicsWorld::updateEnemies(float deltaTime) {

	for (size_t i = 0; i < movingEnemies.size(); ++i) {
		Enemy* currentEnemy = movingEnemies[i];
		std::vector<physx::PxVec3>& path = currentEnemy->getControlPoints();
		physx::PxRigidDynamic* actor = enemyDynamics[i];

		//size_t currentPointIndex = currentEnemy->getControlPointIndex();
		static size_t currentPointIndex = 0;

		// Calculate the direction to the next control point
		physx::PxVec3 direction = path[currentPointIndex] - actor->getGlobalPose().p;
		direction.normalize();

		// Move the actor towards the next control point
		physx::PxVec3 newPos = actor->getGlobalPose().p + direction * deltaTime;
		// Calculate the rotation needed to face the direction
		physx::PxVec3 forward = path[1] - path[0]; // initial forward direction
		forward.normalize();
		physx::PxQuat rotationQuat;

		if (direction.magnitude() > 0) {
			physx::PxVec3 cross = forward.cross(direction);
			float dot = forward.dot(direction);
			float angle = acos(dot);

			rotationQuat = physx::PxQuat(angle, cross.getNormalized());
		}

		// Set the new position and rotation
		physx::PxTransform newTransform(newPos, rotationQuat);
		actor->setKinematicTarget(newTransform);

		// Update model position and rotation
		Model* currentEnemy_model = (Model*)actor->userData;
		currentEnemy_model->resetModelMatrix();
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(newPos.x, newPos.y, newPos.z));
		glm::quat glmQuat(rotationQuat.w, rotationQuat.x, rotationQuat.y, rotationQuat.z);
		modelMatrix *= glm::mat4_cast(glmQuat);
		currentEnemy_model->setModel(modelMatrix);

		// Check if the actor has reached the current control point
		float distanceToNextPoint = (path[currentPointIndex] - actor->getGlobalPose().p).magnitude();
		if (distanceToNextPoint < 0.1f) {
			// Move to the next control point
			currentPointIndex = (currentPointIndex + 1) % path.size();
			//currentEnemy->updateControlPointIndex();
		}
	}
}

glm::vec3 PhysicsWorld::getEnemyPosition() {

	PxVec3 tmp = pTestEnemy->getGlobalPose().p;
	return glm::vec3(tmp.x, tmp.y, tmp.z);
}

boolean PhysicsWorld::isPlayerHit() {
	
	float distance = calcDirectionEnemyPlayer().magnitude();
	//TODO dont hardcode ballradius
	//for each (Enemy* enemy in movingEnemies)
	//{

	//}
	return distance < 2.5;
}
boolean PhysicsWorld::isPlayerDead() {

	PxExtendedVec3 position = controllerPlayer->getFootPosition();
	return position.y <= 0.25f;

}


PxVec3 PhysicsWorld::calcDirectionEnemyPlayer() {

	PxVec3 ballPos = pTestEnemy->getGlobalPose().p;
	
	PxExtendedVec3 tmp = controllerPlayer->getPosition();
	PxVec3 playerPos = PxVec3(tmp.x,tmp.y,tmp.z);
	PxVec3 directionToPlayer =playerPos - ballPos;
	//directionToPlayer.normalize();

	return directionToPlayer;
}


void PhysicsWorld::updateEnemy() {

	PxVec3 directionToPlayer = calcDirectionEnemyPlayer();

	pTestEnemy->addForce(directionToPlayer / (directionToPlayer.magnitude() * (10)), PxForceMode::eIMPULSE);
	PxVec3 position = pTestEnemy->getGlobalPose().p;
	
	glm::vec3 newPos = glm::vec3(position.x, 3.0f, position.z);

	directionToPlayer.normalize();
	// Calculate the rotation needed to look at the player
	PxVec3 forward = directionToPlayer;
	PxVec3 up(0.0f, 1.0f, 0.0f); // Assuming Y-up world
	PxVec3 right = up.cross(forward).getNormalized();
	up = forward.cross(right);

	PxMat33 rotationMatrix(right, up, forward);
	PxQuat rotationQuat(rotationMatrix);

	// Get the enemy model
	Model* enemy = (Model*)pTestEnemy->userData;
	enemy->resetModelMatrix();

	// Convert PxQuat to glm::quat
	glm::quat glmQuat(rotationQuat.w, rotationQuat.x, rotationQuat.y, rotationQuat.z);

	// Create the model matrix with the new position and rotation
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), newPos) * glm::toMat4(glmQuat);
	enemy->setModel(modelMatrix);
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

	controllerPlayer->setPosition(PxExtendedVec3(0.0f, 3.5f, 0.0f));
	

}



