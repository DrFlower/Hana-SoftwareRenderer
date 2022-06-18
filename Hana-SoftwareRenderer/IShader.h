#pragma once

#include "math.h"
#include "tgaimage.h"
#include "model.h"
#include "color.h"
#include "camera.h"

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
	float intensity;
};

class Matrial;

class DrawData {
public:
	DrawData() {};
	DrawData(Model* model, Matrial* matrial, camera_t* camera) :model(model), matrial(matrial), camera(camera) {}
	Model* model;
	Matrial* matrial;
	camera_t* camera;
};

struct IShader {
	DrawData* draw_data;
	IShader(DrawData* dd) :draw_data(dd) {};

	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) = 0;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) = 0;
};


class Matrial {
public:
	Matrial(IShader* shader, MaterialProperty* mp) :texture_shader(shader), material_property(mp) {}
	IShader* texture_shader;
	MaterialProperty* material_property;
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

static float saturate(float value){
	return value > 1 ? 1 : value < 0 ? 0 : value;
}

static Vector3f get_camera_pos(camera_t* camera){
	return camera_get_position(camera);
}

//static Vector3f get_world_light_dir() {
//	return light_dir.normalize();
//}

struct GroundShader : public IShader{
	GroundShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct ToonShader : public IShader{
	ToonShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct TextureShader : public IShader{
	TextureShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct TextureWithLightShader : public IShader{
	TextureWithLightShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct BlinnShader : public IShader{
	BlinnShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};