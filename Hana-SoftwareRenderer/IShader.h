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

struct IShader;

struct Matrial {
	Matrial(IShader* shader, MaterialProperty* mp) :shader(shader), material_property(mp) {}
	IShader* shader;
	MaterialProperty* material_property;
};


struct DrawData {
	Model* model;
	Matrial* matrial;
	camera_t* camera;
	Vector3f light_dir;
	Matrix4x4 model_matrix;
	Matrix4x4 model_matrix_I;
	Matrix4x4 view_matrix;
	Matrix4x4 projection_matrix;
};


struct shader_struct_a2v {
	Vector3f obj_pos;
	Vector3f obj_normal;
	Vector2f uv;
};

struct shader_struct_v2f {
	Vector4f clip_pos;
	Vector3f world_pos;
	Vector3f world_normal;
	Vector2f uv;
	float intensity;
};

struct IShader {
	DrawData* draw_data;
	IShader(DrawData* dd) :draw_data(dd) {};

	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) = 0;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) = 0;

	Vector4f ObjectToClipPos(Vector3f pos) {
		return draw_data->projection_matrix * draw_data->view_matrix * draw_data->model_matrix * embed<4>(pos);
	}

	Vector3f ObjectToWorldPos(Vector3f pos) {
		return proj<3>(draw_data->model_matrix * embed<4>(pos));
	}

	Vector3f ObjectToWorldDir(Vector3f dir) {
		return proj<3>(draw_data->model_matrix * embed<4>(dir, 0.f));
	}

	Vector3f ObjectToWorldNormal(Vector3f normal) {
		Matrix<1, 4, float> m_normal;
		m_normal[0] = embed<4>(normal);
		return proj<3>((m_normal * draw_data->model_matrix_I)[0]);
	}

	Vector3f WorldSpaceViewDir(Vector3f worldPos) {
		return camera_get_position(draw_data->camera) - worldPos;
	}

	Vector3f WorldLightDir() {
		return draw_data->light_dir;
	}

	Color tex_diffuse(const Vector2f& uv) {
		TGAImage* tex = draw_data->matrial->material_property->diffuse_map;
		Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
		return (Color)(tex->get(_uv[0], _uv[1]));
	}

	Vector3f tex_normal(const Vector2f& uv) {
		TGAImage* tex = draw_data->matrial->material_property->normal_map;
		Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
		TGAColor c = tex->get(_uv[0], _uv[1]);
		Vector3f res;
		for (int i = 0; i < 3; i++)
			res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
		return res;
	}

	float tex_specular(const Vector2f& uv) {
		TGAImage* tex = draw_data->matrial->material_property->specular_map;
		Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
		return tex->get(_uv[0], _uv[1])[0] / 1.f;
	}
};

static float saturate(float value) {
	return value > 1 ? 1 : value < 0 ? 0 : value;
}

struct GroundShader : public IShader {
	GroundShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct ToonShader : public IShader {
	ToonShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct TextureShader : public IShader {
	TextureShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct TextureWithLightShader : public IShader {
	TextureWithLightShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct BlinnShader : public IShader {
	BlinnShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct NormalMapShader : public IShader {
	NormalMapShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct ShadowShader : public IShader {
	ShadowShader(DrawData* dd);
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};