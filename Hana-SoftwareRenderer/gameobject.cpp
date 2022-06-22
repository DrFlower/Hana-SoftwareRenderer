#include "gameobject.h"

GameObject::GameObject(Vector3f position, Vector3f rotation, Vector3f scale) {
	transform.position = position;
	transform.rotation = rotation;
	transform.scale = scale;
}

void GameObject::tick(float delta_time) {
}

Matrix4x4 GameObject::GetModelMatrix() {
	Matrix4x4 m = Matrix4x4::identity();
	
	return Matrix4x4::identity();
}


GameObject_RotateAround::GameObject_RotateAround(Vector3f position, Vector3f rotation, Vector3f scale) :GameObject(position, rotation, scale) {}

GameObject_StaticModel::GameObject_StaticModel(const char* filename, Vector3f position, Vector3f rotation, Vector3f scale) :GameObject(position, rotation, scale) {
	this->model = new Model(filename);
}

GameObject_StaticModel::~GameObject_StaticModel() {
	delete model;
}


GameObject_MovingModel::GameObject_MovingModel(const char* filename, Vector3f position, Vector3f rotation, Vector3f scale) :GameObject_StaticModel(filename, position, rotation, scale) {}

GameObject_AnimationModel::GameObject_AnimationModel(const char* filename, Vector3f position, Vector3f rotation, Vector3f scale) :GameObject_StaticModel(filename, position, rotation, scale) {}
