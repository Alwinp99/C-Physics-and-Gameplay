/*
#pragma once
#include "GameObject.h"
#include "TutorialGame.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject();
            ~StateGameObject();

            virtual void Update(float dt);

            void SetPlayerPosition(Transform& player)
            {
                this->playerPos = player;
            }

            void OnCollisionBegin(GameObject* col)
            {
                if (col->GetName() == "Item")
                {
                    TutorialGame::playerScore++;
                    col->SetAlive(false);

                }
            }

            GameWorld* gameWorld;

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);
            void Move(Vector3 position);
            void Walk(float dt);
            void RunAway();
            void PathFind();

            void Collision();
            void Restart();
            void PlayerNear();
            

            StateMachine* stateMachine;
            StateGameObject* testStateObject;

            float walkSpeed;
            NavigationGrid* grid;
            NavigationPath path;
            Vector3 pos;
            Vector3 secondPos;
            Vector3 start;
            bool spawn = false;

            float counter;
            Transform playerPos;
        };
    }
}
*/
