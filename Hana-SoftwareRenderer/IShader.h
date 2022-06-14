#pragma once

#include "math.h"
#include "tgaimage.h"
#include "model.h"

struct shader_env
{
	Vector3f world_ligt_dir;
	TGAColor ambient;
};

struct shader_struct_a2v
{
	Vector3f obj_pos;
	Vector3f obj_normal;
	Vector2f uv;
};

struct shader_struct_v2f
{
	Vector3f clip_pos;
	Vector3f world_pos;
	Vector3f world_normal;
	Vector2f uv;
};

class IShader2 {
private:
	Model* model;
public:
	IShader2(Model* model) :model(model) {}

	virtual shader_struct_v2f vertex(const shader_struct_a2v& a2v, const shader_env&) = 0;
	virtual bool fragment(const shader_struct_v2f& v2f, const shader_env&, TGAColor& color) = 0;
protected:
	TGAColor tex2d(Vector2f uv)
	{
		return model->diffuse(uv);
	}
};

