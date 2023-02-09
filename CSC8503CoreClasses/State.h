#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		typedef std::function<void(float)> StateUpdateFunction;

		class  State		{
		public:
			State() {}
			State(StateUpdateFunction someFunc) {
				func		= someFunc;
			}
			void Update(float dt)  {
				if (func != nullptr) {
					func(dt);
				}
			}
		protected:
			StateUpdateFunction func;
		};

		typedef void(*NewState)(void*);
		typedef void(*GooseMach)(GameObject* sentry, GameObject* player);

		class NormalState : public State {
		public:
			NormalState(NewState func, void* data) {
				function = func;
				newData = data;
			}
			virtual void Update() {
				if (newData != nullptr) {
					function(newData);
				}

			}
		protected:
			NewState function;
			void* newData;
		};

		class GooseState : public State {
		public:
			GooseState(GooseMach mach, GameObject* goose, GameObject* player) {
				stat = mach;
				this->goose = goose;
				this->player = player;
			}

			virtual void Update() {
				if (goose == nullptr || player == nullptr) {
					return;
					stat(goose, player);
				}
			}
		protected:
			GooseMach stat;
			GameObject* goose;
			GameObject* player;
		};
	}
}