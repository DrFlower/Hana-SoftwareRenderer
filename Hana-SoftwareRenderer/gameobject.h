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

//围绕旋转物体
class GameObject_RotateAround : public GameObject
{
public:
	GameObject_RotateAround(Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
};


//静态模型
class GameObject_StaticModel : public GameObject
{
public:
	Model* model;
	GameObject_StaticModel(const char* filename, Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
	~GameObject_StaticModel();
};


//可移动物体
class GameObject_MovingModel : public GameObject_StaticModel
{
public:
	GameObject_MovingModel(const char* filename, Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
};

//动画物体
class GameObject_AnimationModel : public GameObject_StaticModel
{
public:
	GameObject_AnimationModel(const char* filename, Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);
};