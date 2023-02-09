#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"


using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		GameObject(std::string name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		void SetCanCollect(bool canCollect) {
			this->canCollect = canCollect; 
		}

		void SetItemCollected(bool collect) { 
			this->collect = collect; 
		}
		bool ItemCollected() const {
			return collect; 
		}

		void SetHasObject(bool item) {
			this->item = item; 
		}

		bool HasObject() const {
			return hasObject; 
		}


		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		void SetCollision(CollisionHandle collisionObject) { this->collisionObject = collisionObject; }

		CollisionHandle Collided() { 
			return collisionObject; 
		}

		Vector3 GetNewPosition() const { 
			return newPosition; 
		}

		void SetState(string stat) { state = stat; }

		void SetAlive(bool alive)
		{
			isActive = alive;
		}


	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		bool        item;
		int			worldID;
		bool	canCollect;
		bool	collect;
		bool hasObject = false;
		float   objectRadius;
		CollisionHandle collisionObject;
		string	name;
		string	state;


		Vector3 broadphaseAABB;
		Vector3 newPosition;
	};
}

