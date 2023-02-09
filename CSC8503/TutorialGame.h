#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "State.h"
#include "StateMachine.h"
#include "PushdownState.h"
#include "PushdownMachine.h"
#include "NavigationGrid.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame {
		public:
			
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);



		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on).
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			bool CollectItem(const GameObject& player, const GameObject& item);
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void PlayerCamera();
			void MovingObstacles();
			void Restart();
			void Move();
			void GoosePatrol();
			void GooseStateMachine();
			void NewSelect();
			float DistToPlayer(const Vector3& playerPos, const Vector3& objectPos);

			void PlayerGainScore(int amount) {
				playerScore += amount;
			}
			void PlayerLoseScore(int amount) {
				playerScore -= amount;
			}
			int GetPlayerScore() {
				return playerScore;
			}

			void Menu();
			void UpdateMenu(float dt);
			void SetPoints(float dt);

			GameObject* AddFloorToWorld(const Vector3& position, Vector3 dimensions = Vector3(100, 2, 100), string objectName = "Ground", CollisionHandle col = CollisionHandle::GROUND);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddRampToWorld(const Vector3& position, Vector3 dimensions = Vector3(100, 2, 100), string objectName = "Ground", CollisionHandle col = CollisionHandle::GROUND);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddMovingObstacleToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddRotatingObstacleToWorld(const Vector3& position, Vector3 dimensions, string objectName = "Rotate");
			GameObject* AddWaterToWorld(const Vector3& position, Vector3 dimensions, string objectName = "Water");
			GameObject* AddWallsToWorld(const Vector3& position, Vector3 dimensions = Vector3(100, 2, 100), string objectName = "Walls");
			GameObject* AddFirstLevelToWorld(const Vector3& position, Vector3 dimensions, Quaternion rotation, string objectName = "First");
			GameObject* AddBounceAreaToWorld(const Vector3& position, Vector3 dimensions, string objectName = "Bounce");
			GameObject* AddRunAreaToWorld(const Vector3& position, Vector3 dimensions, string objectName = "Run");
			GameObject* CreateDoor(const Vector3& position, Vector3 dimensions, float inverseMass, MeshGeometry* cubeMesh, ShaderBase* basicShader, TextureBase* basicTex);
			void AddMazeToWorld();

			//GameObject* AddItemToWorld(const Vector3& position);
			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position, string objectName = "Item");
			//StateGameObject* AddStateObjectToWorld(const Vector3& position);

			void PlayerCollision(GameObject* otherObject);
				



#ifdef USEVULKAN
			GameTechVulkanRenderer* renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;

			bool useGravity;
			bool inSelectionMode;
			bool goosePathfinding;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			MeshGeometry* capsuleMesh = nullptr;
			MeshGeometry* cubeMesh = nullptr;
			MeshGeometry* sphereMesh = nullptr;

			TextureBase* basicTex = nullptr;
			ShaderBase* basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry* charMesh = nullptr;
			MeshGeometry* enemyMesh = nullptr;
			//MeshGeometry* item = nullptr;
			MeshGeometry* bonusMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* player = nullptr;
			GameObject* goose = nullptr;
			GameObject* enemy = nullptr;
			GameObject* water = nullptr;
			GameObject* firstLevel = nullptr;
			GameObject* movingObstacle[3];
			GameObject* item[5];
			GameObject* rotatingObstacle[6];

			const Vector3 GOOSE_POS = Vector3(-180, 3, -470);
			const Vector3 ENEMY_POS = Vector3(100, 3, -250);

			int numOfItems = 0;
			int itemsCollected = 0;
			int numOfPowerups = 0;
			int powerupsCollected = 0;
			//int playerScore = 0;

			const int points = 1;
			const int powerupPoints = 5;

			int timer = 180;
			float time = 0;
			float playerForce;
			float rotation;

			std::vector<GameObject*> itemsList;
			std::vector<GameObject*> powerupsList;
			float obstacleDir[3] = { 1.0f, 1.0f, 1.0f };
			StateMachine* state;
			float pathTime = 0.51f;
			//float distToPlayer = 0;

			bool pauseGame = false;
			bool startGame;

			int playerScore;


			Vector3 lastPos;
			Vector3 position;
			Vector3 pathDir;
			GameObject* objClosest = nullptr;
			//StateGameObject* testStateObject;
			int menuState;
			NavigationGrid* grid;
		};


	}

}

