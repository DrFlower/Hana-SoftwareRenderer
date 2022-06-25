#pragma once

#include "api.h"
#include "model.h"

struct Transform {
	Vector3f position;
	Vector3f rotation;
	Vector3f scale;
};

class GameObject {
public:
	Transform transform;
	GameObject(Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
	virtual void tick(float delta_time);
	Matrix4x4 GetModelMatrix();
};

//¾²Ì¬Ä£ÐÍ
class GameObject_StaticModel : public GameObject
{
public:
	Model* model;
	GameObject_StaticModel(Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
	GameObject_StaticModel(const char* filename, Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
	~GameObject_StaticModel();
};