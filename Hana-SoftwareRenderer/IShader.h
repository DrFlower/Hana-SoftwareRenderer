#pragma once

#include "math.h"
#include "tgaimage.h"
#include "model.h"
#include "color.h"

struct MaterialProperty {
	TGAImage* diffuse_map;
	TGAImage* normal_map;
	TGAImage* specular_map;
	Color color;
	Color specular;
	float gloss;
	float bump_scale;
};

struct shader_struct_a2v
{
	Vector3f obj_pos;
	Vector3f obj_normal;
	Vector2f uv;
};

struct shader_struct_v2f
{
	Vector4f clip_pos;
	Vector3f world_pos;
	Vector3f world_normal;
	Vector2f uv;
};

struct IShader2 {
	MaterialProperty* material_property;
	IShader2(MaterialProperty* mp) :material_property(mp) {};

	virtual shader_struct_v2f vertex(const shader_struct_a2v& a2v) = 0;
	virtual bool fragment(const shader_struct_v2f& v2f, Color& color) = 0;
};


class Matrial {
public:
	Matrial(IShader2* shader, MaterialProperty* mp) :shader(shader), material_property(mp) {}
	IShader2* shader;
	MaterialProperty* material_property;
};

class AppData {
public:
	AppData(Model* model, Matrial* matrial) :model(model), matrial(matrial) {}
	Model* model;
	Matrial* matrial;
};

static Color tex_diffuse(TGAImage* tex, const Vector2f& uv) {
	Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
	return (Color)(tex->get(_uv[0], _uv[1]));
}

static Vector3f tex_normal(TGAImage* tex, const Vector2f& uv) {
	Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
	TGAColor c = tex->get(_uv[0], _uv[1]);
	Vector3f res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
	return res;
}

static float tex_specular(TGAImage* tex, const Vector2f& uv) {
	Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
	return tex->get(_uv[0], _uv[1])[0] / 1.f;
}