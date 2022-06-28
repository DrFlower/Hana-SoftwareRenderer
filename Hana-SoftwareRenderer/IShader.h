#pragma once

#include "tgaimage.h"
#include "renderbuffer.h"
#include "mathapi.h"

struct Material {
	TGAImage* diffuse_map;
	TGAImage* normal_map;
	TGAImage* specular_map;
	Color color;
	Color specular;
	float gloss;
	float bump_scale;
};

struct ShaderData {
	Material* matrial;
	RenderBuffer* targetBuffer;
	RenderBuffer* shadow_map;
	bool enable_shadow;
	Vector3f view_Pos;
	Vector3f light_dir;
	Color light_color;
	Color ambient;
	Matrix4x4 model_matrix;
	Matrix4x4 model_matrix_I;
	Matrix4x4 view_matrix;
	Matrix4x4 projection_matrix;
	Matrix4x4 light_vp_matrix;
	Matrix4x4 camera_vp_matrix;
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
	ShaderData* shader_data;

	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) = 0;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) = 0;

	Vector4f ObjectToClipPos(Vector3f pos) {
		return shader_data->camera_vp_matrix * shader_data->model_matrix * embed<4>(pos);
	}

	Vector4f ObjectToViewPos(Vector3f pos) {
		return shader_data->light_vp_matrix * shader_data->model_matrix * embed<4>(pos);
	}

	Vector3f ObjectToWorldPos(Vector3f pos) {
		return proj<3>(shader_data->model_matrix * embed<4>(pos));
	}

	Vector3f ObjectToWorldDir(Vector3f dir) {
		return proj<3>(shader_data->model_matrix * embed<4>(dir, 0.f));
	}

	Vector3f ObjectToWorldNormal(Vector3f normal) {
		Matrix<1, 4, float> m_normal;
		m_normal[0] = embed<4>(normal);
		return proj<3>((m_normal * shader_data->model_matrix_I)[0]);
	}

	Vector3f WorldSpaceViewDir(Vector3f worldPos) {
		return shader_data->view_Pos - worldPos;
	}

	Vector3f WorldLightDir() {
		return shader_data->light_dir;
	}

	Color tex_diffuse(const Vector2f& uv) {
		TGAImage* tex = shader_data->matrial->diffuse_map;
		Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
		return (Color)(tex->get(_uv[0], _uv[1]));
	}

	Vector3f tex_normal(const Vector2f& uv) {
		TGAImage* tex = shader_data->matrial->normal_map;
		Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
		TGAColor c = tex->get(_uv[0], _uv[1]);
		Vector3f res;
		for (int i = 0; i < 3; i++)
			res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
		return res;
	}

	float tex_specular(const Vector2f& uv) {
		TGAImage* tex = shader_data->matrial->specular_map;
		Vector2i _uv(uv[0] * tex->get_width(), uv[1] * tex->get_height());
		return tex->get(_uv[0], _uv[1])[0] / 1.f;
	}

	int is_in_shadow(Vector4f depth_pos, float n_dot_l) {

		if (shader_data->enable_shadow && shader_data->shadow_map)
		{
			float widht = shader_data->shadow_map->width;
			float height = shader_data->shadow_map->height;

			Vector3f ndc_coords;
			ndc_coords = proj<3>(depth_pos / depth_pos[3]);
			Vector3f pos = viewport_transform(widht, height, ndc_coords);
			float depth_bias = 0.05f * (1 - n_dot_l);
			if (depth_bias < 0.005f) depth_bias = 0.01f;
			float current_depth = depth_pos[2] - depth_bias;

			if (pos.x < 0 || pos.y < 0 || pos.x >= widht || pos.y >= height)
				return 0;

			float closest_depth = shader_data->shadow_map->get_color(pos.x, pos.y).r;
			return current_depth < closest_depth;
		}

		return 1;
	}
};

struct GroundShader : public IShader {
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct ToonShader : public IShader {
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct TextureShader : public IShader {
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct TextureWithLightShader : public IShader {
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct BlinnShader : public IShader {
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct NormalMapShader : public IShader {
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};

struct ShadowShader : public IShader {
	virtual shader_struct_v2f vertex(shader_struct_a2v* a2v) override;
	virtual bool fragment(shader_struct_v2f* v2f, Color& color) override;
};