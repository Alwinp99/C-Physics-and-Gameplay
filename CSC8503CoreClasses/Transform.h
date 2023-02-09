#pragma once

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);
			void SetWorldPos(const Vector3& worldPos);
			void SetLocalPos(const Vector3& localPos);
			void SetGlobalScale(const Vector3& worldScale);

			Vector3 GetPosition() const {
				return position;
			}

			Vector3 GetScale() const {
				return scale;
			}

			Matrix4 GetGlobalMatrix() const {
				return globalMatrix;
			} 

			Vector3 GetLocalPos() const {
				return localPos;
			}
			Vector3 GetGlobalPosition() const {
				return globalMatrix.GetPositionVector();
			}

			void SetLocalRotation(const Quaternion& rotate) {
				localRotation = rotate;
			}


			Quaternion GetLocalRotation() const {
				return localRotation;
			}
			Quaternion GetOrientation() const {
				return orientation;
			}

			Quaternion GetGlobalOrientation() const {
				return globalOrientation;
			}

			Matrix4 GetMatrix() const {
				return matrix;
			}
			void UpdateMatrix();
		protected:
			Matrix4		matrix;
			Matrix4		globalMatrix;
			Quaternion	orientation;
			Quaternion  globalOrientation;
			Quaternion	localRotation;
			Vector3		position;
			Vector3		localPos;
			Transform* parentObject;

			Vector3		scale;
		};
	}
}

