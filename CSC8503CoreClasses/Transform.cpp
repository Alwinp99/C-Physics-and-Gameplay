#include "Transform.h"

using namespace NCL::CSC8503;

Transform::Transform()	{
	parentObject = nullptr;
	scale = Vector3(1, 1, 1);
}

Transform::~Transform()	{

}

void Transform::UpdateMatrix() {
	matrix =
		Matrix4::Translation(position) *
		Matrix4(orientation) *
		Matrix4::Scale(scale);
}

Transform& Transform::SetPosition(const Vector3& worldPos) {
	position = worldPos;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetScale(const Vector3& worldScale) {
	scale = worldScale;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetOrientation(const Quaternion& worldOrientation) {
	orientation = worldOrientation;
	UpdateMatrix();
	return *this;
}

void Transform::SetWorldPos(const Vector3& worldPos) {
	if (parentObject) { 
		Vector3 parentPos = parentObject->GetGlobalMatrix().GetPositionVector();
		Vector3 posDiff = parentPos - worldPos;

		localPos = posDiff;
		matrix.SetPositionVector(posDiff);
	}
	else {
		localPos = worldPos;

		globalMatrix.SetPositionVector(worldPos);
	}
}

void Transform::SetLocalPos(const Vector3& local) {
	localPos = local;
}

void Transform::SetGlobalScale(const Vector3& globalScale) {
	if (parentObject) {

	}
	else {
		scale = globalScale;
	}
}
