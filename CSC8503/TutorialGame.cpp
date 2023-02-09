#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "MeshGeometry.h"
#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"
#include "NavigationGrid.h"

using namespace NCL;
using namespace CSC8503;




TutorialGame::TutorialGame()	{
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);
	state = new StateMachine();

	forceMagnitude	= 10.0f;
	//grid = new NavigationGrid(4);
	useGravity		= false;
	goosePathfinding = false;
	inSelectionMode = false;
    


	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("goose.msh");
	bonusMesh	= renderer->LoadMesh("coin.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
	delete player;
	delete goose;
	delete enemy;
	delete water;
	delete firstLevel;

	delete[] movingObstacle;
	delete[] item;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (player != nullptr) {
		Vector3 objPos = player->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPlayerPosition(objPos);
		//world->GetMainCamera()->SetPosition(camPos);
		//world->GetMainCamera()->SetPitch(angles.x);
		//world->GetMainCamera()->SetYaw(angles.y);
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	//renderer->DrawString("Items:" + std::to_string(numOfItems), Vector2(10, 42));
	Debug::Print("Items: " + std::to_string(numOfItems), Vector2(5, 40), Debug::RED);
	Debug::Print("Powerups: " + std::to_string(numOfPowerups), Vector2(5, 30), Debug::RED);
	Debug::Print("Player Score: " + std::to_string(playerScore), Vector2(5, 20), Debug::RED);
	Debug::Print("Time Remaining: " + std::to_string(timer), Vector2(5, 10), Debug::RED);


	player->Collided() == CollisionHandle::WATER ? player->GetPhysicsObject()->SetInverseMass(0.35f) : player->GetPhysicsObject()->SetInverseMass(1.0f);

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	state->Update(dt);

	//SetPoints(dt);
	pathTime += dt;
	time += dt;
	if (time >= 1.0f && timer > 0) {
		timer--;
		time = 0;
	}

	if (timer <= 0 || playerScore >= 35) {
		//Restart();
	}

	/*
	for (GameObject* x : item) {
		if (x->ItemCollected()) {
			numOfItems++;
			itemsList.emplace_back(x);
			x->SetItemCollected(false);
		}
	}
	*/
	/*
	for (GameObject* x : item) {
		if (x->ItemCollected()) {
			numOfItems++;
			itemsList.emplace_back(x);
			player->SetHasObject(true);
			x->SetItemCollected(false);
		}
	}
	*/
	if (player->Collided() == CollisionHandle::SAFE && (numOfItems > 0 || numOfPowerups > 0)) {
		playerScore += numOfItems;
		playerScore += numOfPowerups * powerupPoints;
		numOfItems = numOfPowerups = 0;
		player->SetHasObject(false);
		goose->GetTransform().SetPosition(GOOSE_POS);
		enemy->GetTransform().SetPosition(ENEMY_POS);
		itemsList.clear();
		powerupsList.clear();
	}

	if (player->Collided() == CollisionHandle::BOUNCE) {
		player->SetCollision(CollisionHandle::START);
		player->GetPhysicsObject()->AddForce(Vector3(0.0f, 1000.0f, 0.0f));
	}

	if (player->Collided() == CollisionHandle::ENEMY) {
		player->SetCollision(CollisionHandle::START);
		player->SetHasObject(false);
		for (GameObject* x : powerupsList) {
			x->GetTransform().SetPosition(x->GetNewPosition());

		}
		powerupsList.clear();
		numOfPowerups = 0;
	}



	
	if (goosePathfinding) {
		GoosePatrol();

	}
	

	//distToPlayer = (player->GetTransform().GetPosition() - goose->GetTransform().GetPosition()).Length();
	//Vector3 displacement = { player->GetTransform().GetPosition().x - goose->GetTransform().GetPosition().x, goose->GetTransform().GetPosition().y - player->GetTransform().GetPosition().y, goose->GetTransform().GetPosition().z - player->GetTransform().GetPosition().z };

	/*
	

	*/


	MovingObstacles();
	//Move(); 

	

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

float TutorialGame::DistToPlayer(const Vector3& playerPos, const Vector3& objectPos) {
	//playerPos = player->GetTransform().GetPosition();
	//objectPos = goose->GetTransform().GetPosition();
	Vector3 displacement = { objectPos.x - playerPos.x, objectPos.y - playerPos.y, objectPos.z - playerPos.z };
	return sqrt(pow(displacement.x, 2) + pow(displacement.y, 2) + pow(displacement.z, 2));
}

void TutorialGame::MovingObstacles() {
	if (movingObstacle[0]->Collided() == CollisionHandle::WALLS) {
		obstacleDir[0] *= -1.0f;
		movingObstacle[0]->SetCollision(CollisionHandle::START);
	}
	movingObstacle[0]->GetPhysicsObject()->AddForce(Vector3(-100000.0f * obstacleDir[0], 0.0f, 0.0f));

	//for (int x = 1; x < 3; ++x) {
	if (movingObstacle[0]->Collided() == CollisionHandle::WALLS) {
		obstacleDir[0] *= -1.0f;
	}
	//movingObstacle[0]->GetPhysicsObject()->AddForce(Vector3(0.0f, 0.0f, 100000.0f * obstacleDir[0]));
	//}
}

void TutorialGame::Restart() {
	selectionObject = nullptr;
	delete state;
	state = new StateMachine();
	goosePathfinding = false;
	InitWorld();
}
//bool CollectItem(const GameObject& player, const GameObject& item) {
	//float x = player.GetTransform();
	//float dx = player.GetTransform().GetPosition().x - item.GetTransform().GetPosition().x;
	//Vector3 playerPos = player.GetTransform().GetPosition();
	
//}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void  TutorialGame::PlayerCamera() {
	if (lockedObject != nullptr) {
		Vector3 object = lockedObject->GetTransform().GetPosition();
		Vector3 camera = object + lockedOffset;
		Matrix4 swap = Matrix4::BuildViewMatrix(camera, object, Vector3(0, 1, 0));
		Matrix4 mod = swap.Inverse();
		Quaternion quat(mod);
		Vector3 ang = quat.ToEuler();
		world->GetMainCamera()->SetPosition(camera + Vector3(0, 10, 10));
		world->GetMainCamera()->SetPitch(ang.x);
		world->GetMainCamera()->SetYaw(ang.y);
	}
}

/*
void TutorialGame::Move() {
	if(inSelectionMode) {
		float moveSpeed = 150.0f;
		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::W)) {
			player->GetPhysicsObject()->AddForceAtPosition(Vector3(0, 0, -moveSpeed), player->GetTransform().GetPosition());
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::A)) {
			player->GetPhysicsObject()->AddForce(Vector3(-moveSpeed, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::S)) {
			player->GetPhysicsObject()->AddForce(Vector3(0, 0, moveSpeed));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::D)) {
			player->GetPhysicsObject()->AddForce(Vector3(moveSpeed, 0, 0));
		}

	}
	
}
*/

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);

	InitGameExamples();
	InitDefaultFloor();

	numOfItems = 0;
	itemsCollected = 0;
	numOfPowerups = 0;
	powerupsCollected = 0;
	playerScore = 0;
	timer = 180;
	water = AddWaterToWorld(Vector3(0, 0, -40), Vector3(40, 1, 50));

	//AddWallsToWorld(Vector3(0, 4, -100), Vector3(40, 6, 2));
	AddWallsToWorld(Vector3(-100, 4, -48), Vector3(1, 6, 50));
	AddWallsToWorld(Vector3(100, 4, -48), Vector3(1, 6, 50));

	AddFloorToWorld(Vector3(0, -20, -290), Vector3(220, 2, 200));
	AddWallsToWorld(Vector3(-101, 4, -101), Vector3(2, 10, 200));
	AddWallsToWorld(Vector3(101, 4, -47), Vector3(2, 10, 200));
	AddWallsToWorld(Vector3(0, 4, -150), Vector3(200, 10, 2));
	//AddWallsToWorld(Vector3(122, 8, -88), Vector3(80, 10, 2));
	//AddWallsToWorld(Vector3(-122, 8, -88), Vector3(80, 10, 2));
	//item[0] = AddBonusToWorld(Vector3(0, 5, 0));

    AddBonusToWorld(Vector3(-50, 1, 60));
	AddBonusToWorld(Vector3(30, 1, 0));
	AddBonusToWorld(Vector3(30, 1, 60));
	AddBonusToWorld(Vector3(-50, 1, 100));
	AddBonusToWorld(Vector3(30, 1, 0));
	AddBonusToWorld(Vector3(5, 5, 1));
	AddCubeToWorld(Vector3(0, 1, 1), Vector3(0.8, 0.8, 0.8), 10.0f);
	AddCubeToWorld(Vector3(-50, 1, 50), Vector3(0.8, 0.8, 0.8), 10.0f);
	AddCubeToWorld(Vector3(30, 1, 10), Vector3(0.8, 0.8, 0.8), 10.0f);
	AddCubeToWorld(Vector3(-50, 1, 110), Vector3(0.8, 0.8, 0.8), 10.0f);
	AddCubeToWorld(Vector3(40, 1, 60), Vector3(0.8, 0.8, 0.8), 10.0f);
	AddCubeToWorld(Vector3(5, 5, 10), Vector3(0.8, 0.8, 0.8), 10.0f);
	

	//firstLevel = AddFirstLevelToWorld(Vector3(-50, 4, 60), Vector3(0.5, 2, 4), Quaternion(Vector3(0, -0.8, 0), 0.34f));

	AddWallsToWorld(Vector3(-50, 4, 70), Vector3(1, 2, 26));
	AddWallsToWorld(Vector3(-50, 4, 11), Vector3(1, 2, 15));
	AddWallsToWorld(Vector3(24.5, 4, -5), Vector3(75.5, 2, 1));

	AddRotatingObstacleToWorld(Vector3(-10, 4, 56), Vector3(0.5, 2, 4));
	AddRotatingObstacleToWorld(Vector3(-10, 4, 26), Vector3(0.5, 2, 4));
	AddRotatingObstacleToWorld(Vector3(10, 4, 36), Vector3(0.5, 2, 4));
	AddRotatingObstacleToWorld(Vector3(40, 4, 56), Vector3(0.5, 2, 4));
	AddRotatingObstacleToWorld(Vector3(40, 4, 26), Vector3(0.5, 2, 4));
	AddRotatingObstacleToWorld(Vector3(60, 4, 36), Vector3(0.5, 2, 4));



	AddWallsToWorld(Vector3(-60, 5, 50), Vector3(60, 3, 1));
	AddWallsToWorld(Vector3(-50, 5, 100), Vector3(60, 3, 1));
	AddWallsToWorld(Vector3(-80, 5, 80), Vector3(1, 6, 50));
	AddRampToWorld(Vector3(-60, 4, 80), Vector3(8, 1, 9));

	movingObstacle[0] = AddMovingObstacleToWorld(Vector3(-70, 6, 70), Vector3(7, 4, 6), 0.001f);
	movingObstacle[1] = AddMovingObstacleToWorld(Vector3(-30, 6, 70), Vector3(7, 4, 8), 0.001f);

	AddBounceAreaToWorld(Vector3(0, 3, 100), Vector3(6, 0.01, 7));
	AddBounceAreaToWorld(Vector3(25, 2.5, 100), Vector3(7, 0.01, 6));

	AddRunAreaToWorld(Vector3(-60, 5, 0), Vector3(5, 0.4, 5));
	AddRunAreaToWorld(Vector3(-60, 5, 30), Vector3(5, 0.4, 5));
	AddRunAreaToWorld(Vector3(-60, 5, -30), Vector3(5, 0.4, 5));
	AddRunAreaToWorld(Vector3(-60, 5, -60), Vector3(5, 0.4, 5));
	AddRunAreaToWorld(Vector3(-60, 5, -90), Vector3(5, 0.4, 5));

	//AddMazeToWorld();

	//GooseStateMachine();

	//AddWallsToWorld(Vector3(0, 5, -30), Vector3(1, 3, 85));
	/*
	AddWallsToWorld(Vector3(-120, 5, -160), Vector3(60, 3, 1));
	AddWallsToWorld(Vector3(-61, 5, -246), Vector3(1, 3, 85));

	AddWallsToWorld(Vector3(-160, 5, -180), Vector3(40, 3, 1));
	// maze block stopper
	AddWallsToWorld(Vector3(-180, 5, -178), Vector3(1, 3, 1));
	AddWallsToWorld(Vector3(-121, 5, -186), Vector3(1, 3, 5));
	AddWallsToWorld(Vector3(-111, 5, -192), Vector3(11, 3, 1));
	AddWallsToWorld(Vector3(-101, 5, -186), Vector3(1, 3, 5));
	AddWallsToWorld(Vector3(-92, 5, -180), Vector3(10, 3, 1));

	AddWallsToWorld(Vector3(-122, 5, -215), Vector3(60, 3, 1));
	AddWallsToWorld(Vector3(-181, 5, -246), Vector3(1, 3, 30));
	AddWallsToWorld(Vector3(-172, 5, -277), Vector3(10, 3, 1));
	AddWallsToWorld(Vector3(-161, 5, -286), Vector3(1, 3, 10));
	AddWallsToWorld(Vector3(-131, 5, -297), Vector3(51, 3, 1));

	AddWallsToWorld(Vector3(-140, 5, -310), Vector3(60, 3, 1));
	AddWallsToWorld(Vector3(-81, 5, -321), Vector3(1, 3, 10));
	AddWallsToWorld(Vector3(-71, 5, -332), Vector3(11, 3, 1));
	AddWallsToWorld(Vector3(-74, 5, -329), Vector3(6, 3, 2));

	AddWallsToWorld(Vector3(-81, 5, -266), Vector3(1, 3, 30));
	AddWallsToWorld(Vector3(-90, 5, -235), Vector3(10, 3, 1));
	AddWallsToWorld(Vector3(-101, 5, -226), Vector3(1, 3, 10));
	
	// maze block stopper
	AddWallsToWorld(Vector3(-79, 5, -235), Vector3(1, 3, 1));
	*/
	
	//movingObstacle[1] = AddMovingObstacleToWorld(Vector3(-191, 6, -200), Vector3(8, 4, 8), 0.001f);
	//movingObstacle[2] = AddMovingObstacleToWorld(Vector3(-71, 6, -305), Vector3(8, 4, 8), 0.001f);
	//AddCubeToWorld(Vector3(0, 6, 0), Vector3(1, 1, 1), 0.001f);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, Vector3 dimensions, string objectName, CollisionHandle col){
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddMovingObstacleToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	string type = "MovingObstacle";
	GameObject* obstacle = new GameObject(type);
	AABBVolume* vol = new AABBVolume(dimensions);
	obstacle->SetBoundingVolume((CollisionVolume*)vol);
	obstacle->GetTransform().SetWorldPos(position);
	obstacle->GetTransform().SetGlobalScale(dimensions);

	obstacle->SetPhysicsObject(new PhysicsObject(&obstacle->GetTransform(), obstacle->GetBoundingVolume()));
	obstacle->SetRenderObject(new RenderObject(&obstacle->GetTransform(), cubeMesh, basicTex, basicShader));
	obstacle->GetPhysicsObject()->SetInverseMass(inverseMass);
	obstacle->GetPhysicsObject()->InitCubeInertia();
	
	world->AddGameObject(obstacle);
	return obstacle;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;
	float moveSpeed = 150.0f;

	GameObject* character = new GameObject();
	SphereVolume* volume  = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	//Move Player clicking on goat and moving arrow keys
	if (inSelectionMode) {

		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::UP)) {
			player->GetPhysicsObject()->AddForce(Vector3(0, 0, -moveSpeed));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::DOWN)) {
			player->GetPhysicsObject()->AddForce(Vector3(-moveSpeed, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::LEFT)) {
			player->GetPhysicsObject()->AddForce(Vector3(0, 0, moveSpeed));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::RIGHT)) {
			player->GetPhysicsObject()->AddForce(Vector3(moveSpeed, 0, 0));
		}

	}

	//float dx = character->GetTransform().GetPosition().x - bonus->GetTransform().GetPosition().x;
	

	
	
	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetPhysicsObject()->SetCollision(CollisionHandle::ENEMY);

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position, string objName) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	//apple->SetCanCollect(true);
	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	PlayerCollision(apple);
	world->AddGameObject(apple);

	return apple;
}



void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -20, 0), Vector3(10, 0.2, 10), "Safe", CollisionHandle::SAFE);
}

void TutorialGame::InitGameExamples() {
	player = AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	
	AddBonusToWorld(Vector3(0, 1, 0));
	AddBonusToWorld(Vector3(0, 5, 2));
}
/*
StateGameObject* AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	//apple->SetCanCollect(true);
	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;

}
*/

/*
void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}
*/
/*
void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}
*/

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::NewSelect() { 
	selectionObject = player;
	lockedObject = player;

	if (inSelectionMode) {
		Window::GetWindow()->ShowOSPointer(true);
		Window::GetWindow()->LockMouseToWindow(false);
	}
}

/*
void TutorialGame::GooseStateMachine() {
	GooseMach idleState = [](GameObject* goose, GameObject* player) {
		goose->SetState("idleState");
		Vector3 dirVec = player->GetTransform().GetPosition() - goose->GetTransform().GetPosition();
		float playerAng = atan2(dirVec.x, dirVec.z);
		Quaternion rotation = Quaternion(0.0f, sin(playerAng * 0.5f), 0.0f, cos(playerAng * 0.5f));
		goose->GetTransform().SetOrientation(rotation);
	};

	GooseMach attackState = [](GameObject* goose, GameObject* player) {
		goose->SetState("attackState");
		Vector3 dirVec = player->GetTransform().GetPosition() - goose->GetTransform().GetPosition();
		float playerAng = atan2(dirVec.x, dirVec.z);
		Quaternion rotation = Quaternion(0.0f, sin(playerAng * 0.5f), 0.0f, cos(playerAng * 0.5f));
		goose->GetTransform().SetOrientation(rotation);
		if (player->HasObject()) {
			goose->GetPhysicsObject()->AddForce(dirVec * 5.0f); 
		}
	};

		GooseState* idle = new GooseState(idleState, goose, player);
		GooseState* attack = new GooseState(attackState, goose, player);

		state->AddState(idle);
		state->AddState(attack);
		float playerDist = (player->GetTransform().GetPosition() - goose->GetTransform().GetPosition()).Length();

		GenericTransition<float&, float>* attackTrans = new GenericTransition<float&, float>(GenericTransition<float&, float>::LessThanTransition, playerDist, 40.0f, idle, attack);
		GenericTransition<float&, float>* idleTrans = new GenericTransition<float&, float>(GenericTransition<float&, float>::GreaterThanTransition, playerDist, 60.0f, attack, idle);

		state->AddTransition(idleTrans);
		state->AddTransition(attackTrans);
}
*/

void TutorialGame::GoosePatrol() {
	NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;

	Vector3 diff = Vector3(300.0f, 0.0f, 500.0f);

	Vector3 startPos = goose->GetTransform().GetPosition() + diff;
	Vector3 endPos = player->GetTransform().GetPosition() + diff;

	bool f = false;
	if (pathTime > 0.50f) {
		f = grid.FindPath(startPos, endPos, outPath);
	}

	if (f) {
		pathTime = 0.0f;
		outPath.PopWaypoint(lastPos);
		outPath.PopWaypoint(position);

		pathDir = position - lastPos;
		pathDir.Normalise();
	}

	if (pathTime > 0.50f) {
		goose->GetPhysicsObject()->ClearForces();
	}
	else {
		goose->GetPhysicsObject()->AddForce(pathDir * 200.0f);
	}
}

GameObject* TutorialGame::AddWaterToWorld(const Vector3& position, Vector3 dimensions, string objectName) {
	GameObject* water = new GameObject(objectName);
	AABBVolume* volume = new AABBVolume(dimensions);
	water->SetBoundingVolume((CollisionVolume*)volume);
	water->GetTransform().SetScale(dimensions);
	water->GetTransform().SetPosition(position);

	water->SetRenderObject(new RenderObject(&water->GetTransform(), cubeMesh, basicTex, basicShader));
	water->SetPhysicsObject(new PhysicsObject(&water->GetTransform(), water->GetBoundingVolume()));

	water->GetPhysicsObject()->SetInverseMass(0);
	water->GetPhysicsObject()->InitCubeInertia();
	water->GetPhysicsObject()->SetCollision(CollisionHandle::WATER);

	world->AddGameObject(water);

	return water;



}

GameObject* TutorialGame::AddWallsToWorld(const Vector3& position, Vector3 dimensions, string objectName) {
	GameObject* walls = new GameObject(objectName);
	AABBVolume* volume = new AABBVolume(dimensions);
	walls->SetBoundingVolume((CollisionVolume*)volume);
	walls->GetTransform().SetScale(dimensions);
	walls->GetTransform().SetPosition(position);

	walls->SetRenderObject(new RenderObject(&walls->GetTransform(), cubeMesh, basicTex, basicShader));
	walls->SetPhysicsObject(new PhysicsObject(&walls->GetTransform(), walls->GetBoundingVolume()));

	walls->GetPhysicsObject()->SetInverseMass(0);
	walls->GetPhysicsObject()->InitCubeInertia();
	walls->GetPhysicsObject()->SetCollision(CollisionHandle::WALLS);

	world->AddGameObject(walls);

	return walls;

}

GameObject* TutorialGame::AddFirstLevelToWorld(const Vector3& position, Vector3 dimensions, Quaternion rotation, string objectName) {
	GameObject* first = new GameObject(objectName);
	OBBVolume* volume = new OBBVolume(dimensions);
	firstLevel->SetBoundingVolume((CollisionVolume*)volume);
	firstLevel->GetTransform().SetScale(dimensions);
	firstLevel->GetTransform().SetPosition(position);
	rotation.Normalise();
	firstLevel->GetTransform().SetOrientation(rotation);

	firstLevel->SetRenderObject(new RenderObject(&firstLevel->GetTransform(), cubeMesh, basicTex, basicShader));
	firstLevel->SetPhysicsObject(new PhysicsObject(&firstLevel->GetTransform(), firstLevel->GetBoundingVolume()));

	firstLevel->GetPhysicsObject()->SetInverseMass(0.1);
	//firstLevel->GetPhysicsObject()->SetElastic(0.0);
	
	firstLevel->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(firstLevel);

	return firstLevel;
}

GameObject* TutorialGame::AddRotatingObstacleToWorld(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* rotater = new GameObject(name);
	OBBVolume* volume = new OBBVolume(dimensions);
	rotater->SetBoundingVolume((CollisionVolume*)volume);
	rotater->GetTransform().SetScale(dimensions);
	rotater->GetTransform().SetPosition(position);

	rotater->SetRenderObject(new RenderObject(&rotater->GetTransform(), cubeMesh, basicTex, basicShader));
	rotater->SetPhysicsObject(new PhysicsObject(&rotater->GetTransform(), rotater->GetBoundingVolume()));

	rotater->GetPhysicsObject()->SetInverseMass(0.001f);
	rotater->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(rotater);

	return rotater;

}

GameObject* TutorialGame::AddRampToWorld(const Vector3& position, Vector3 dimensions, string objectName, CollisionHandle col) {
	GameObject* ground = new GameObject(objectName);
	OBBVolume* volume = new OBBVolume(dimensions);
	ground->SetBoundingVolume((CollisionVolume*)volume);
	ground->GetTransform().SetScale(dimensions);
	ground->GetTransform().SetPosition(position);
	Quaternion rotate = Quaternion(Vector3(1.0f, 0.0f, 0.0f), 0.18f);
	rotate.Normalise();
	ground->GetTransform().SetOrientation(rotate);

	ground->SetRenderObject(new RenderObject(&ground->GetTransform(), cubeMesh, basicTex, basicShader));
	ground->SetPhysicsObject(new PhysicsObject(&ground->GetTransform(), ground->GetBoundingVolume()));

	ground->GetPhysicsObject()->SetInverseMass(0);
	ground->GetPhysicsObject()->InitCubeInertia();
	ground->GetPhysicsObject()->SetCollision(col);

	world->AddGameObject(ground);

	return ground;
}

GameObject* TutorialGame::AddBounceAreaToWorld(const Vector3& position, Vector3 dimensions, string objectName) {
	GameObject* bounce = new GameObject(objectName);
	AABBVolume* volume = new AABBVolume(dimensions);
	bounce->SetBoundingVolume((CollisionVolume*)volume);
	bounce->GetTransform().SetScale(dimensions);
	bounce->GetTransform().SetPosition(position);
	bounce->SetRenderObject(new RenderObject(&bounce->GetTransform(), cubeMesh, basicTex, basicShader));
	bounce->SetPhysicsObject(new PhysicsObject(&bounce->GetTransform(), bounce->GetBoundingVolume()));

	bounce->GetPhysicsObject()->SetInverseMass(0);
	bounce->GetPhysicsObject()->InitCubeInertia();
	bounce->GetPhysicsObject()->SetCollision(CollisionHandle::BOUNCE);

	world->AddGameObject(bounce);
	return bounce;


}

GameObject* TutorialGame::AddRunAreaToWorld(const Vector3& position, Vector3 dimensions, string objectName) {
	GameObject* run = new GameObject(objectName);

	AABBVolume* volume = new AABBVolume(dimensions);
	run->SetBoundingVolume((CollisionVolume*)volume);
	run->GetTransform().SetScale(dimensions);
	run->GetTransform().SetPosition(position);

	run->SetRenderObject(new RenderObject(&run->GetTransform(), cubeMesh, basicTex, basicShader));
	run->SetPhysicsObject(new PhysicsObject(&run->GetTransform(), run->GetBoundingVolume()));

	run->GetPhysicsObject()->SetInverseMass(0);
	run->GetPhysicsObject()->InitCubeInertia();
	run->GetPhysicsObject()->SetCollision(CollisionHandle::GROUND);

	world->AddGameObject(run);
	return run;



}

void TutorialGame::Menu()
{
	pauseGame = true;

	state = new StateMachine();

	State* menu = new State([&](float dt)->void
		{
			Debug::Print("Press P to Play Game", Vector2(6, 90));
			Debug::Print("Press Esc to Leave Session", Vector2(5, 90));
			pauseGame = true;
			//system("pause");
		}
	);

	State* play = new State([&](float dt)->void
		{
			pauseGame = false;
		}
	);


	State* pause = new State([&](float dt)->void
		{
			Debug::Print("Paused, Press ENTER to resume ", Vector2(5, 85));
			pauseGame = true;
			//system("pause");
		}
	);

	State* resume = new State([&](float dt)->void
		{
			std::cout << "Resume session" << std::endl;
			pauseGame = false;
		}
	);

	StateTransition* unpauseA = new StateTransition(resume, pause, [&](void)->bool
		{
			return Window::GetKeyboard()->KeyDown(KeyboardKeys::P);
		}
	);

	StateTransition* pauseA = new StateTransition(pause, resume, [&](void)->bool
		{
			return Window::GetKeyboard()->KeyDown(KeyboardKeys::A);
		}
	);

	StateTransition* startA = new StateTransition(menu, play, [&](void)->bool
		{
			return Window::GetKeyboard()->KeyDown(KeyboardKeys::S);
		}
	);

	StateTransition* pauseB = new StateTransition(play, pause, [&](void)->bool
		{
			return Window::GetKeyboard()->KeyDown(KeyboardKeys::B);
		}
	);

	//stateMachine->AddState(startScreen);
	state->AddState(play);
	state->AddState(resume);
	state->AddState(pause);

    state->AddTransition(pauseA);
	state->AddTransition(unpauseA);
	state->AddTransition(startA);
	state->AddTransition(pauseB);


}

void TutorialGame::UpdateMenu(float dt)
{
	state->Update(dt);
}

void TutorialGame::PlayerCollision(GameObject* otherObject) {
	if (otherObject->GetName() == "Item")
	{
		otherObject->SetAlive(false);
		playerScore++;
	}

}

/*
void TutorialGame::AddMazeToWorld()
{
	int mazeHeight = 50;
	int mazeWidth = 50;
	int mazeSize = 2;
	
	

	
	AddWallsToWorld(Vector3(0, 0, -(mazeHeight - 1) * mazeSize), Vector3((mazeWidth)*mazeSize, 5, 0.5f));
	AddWallsToWorld(Vector3(0, 0, (mazeHeight)*mazeSize), Vector3((mazeWidth)*mazeSize, 5, 0.5f));

	AddWallsToWorld(Vector3((mazeWidth)*mazeSize, 0, 0), Vector3(0.5f, 5, (mazeHeight)*mazeSize));
	AddWallsToWorld(Vector3(-(mazeWidth - 1) * mazeSize, 0, 0), Vector3(0.5f, 5, (mazeHeight)*mazeSize));

	AddWallsToWorld(Vector3(0, 0, 0), Vector3(mazeWidth - 1, 5, 0.5f));
	AddWallsToWorld(Vector3(0, 0, 0), Vector3(0.5f, 5, mazeWidth - 1));
}
*/

			










