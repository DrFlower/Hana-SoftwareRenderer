#pragma once

#include "math.h"
#include "tgaimage.h"
#include "color.h"
#include "renderbuffer.h"

struct Matrial {
	TGAImage* diffuse_map;
	TGAImage* normal_map;
	TGAImage* specular_map;
	Color color;
	Color specular;
	float gloss;
	float bump_scale;
};


struct ShaderData {
	Matrial* matrial;
	renderbuffer* targetBuffer;
	renderbuffer* shadow_map;
	Vector3f view_Pos;
	Vector3f light_dir;
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

	Vector3f viewport_transform(int width, int height, Vector3f ndc_coord) {
		float x = (ndc_coord.x + 1) * 0.5f * (float)width;   /* [-1, 1] -> [0, w] */
		float y = (ndc_coord.y + 1) * 0.5f * (float)height;  /* [-1, 1] -> [0, h] */
		float z = (ndc_coord.z + 1) * 0.5f;                  /* [-1, 1] -> [0, 1] */
		return Vector3f(x, y, z);
	}

	int is_in_shadow(Vector4f depth_pos, float n_dot_l) {

		if (shader_data->shadow_map)
		{
			Vector3f ndc_coords;
			ndc_coords = proj<3>(depth_pos / depth_pos[3]);
			Vector3f pos = viewport_transform(shader_data->shadow_map->width, shader_data->shadow_map->height, ndc_coords);
			//Vector3f pos = depth_pos;
			float depth_bias = 0.05f * (1 - n_dot_l);
			if (depth_bias < 0.005f) depth_bias = 0.01f;
			float current_depth = depth_pos[2] - depth_bias;

			if (pos.x > shader_data->shadow_map->width || pos.y > shader_data->shadow_map->height)
				return 0;

			float closest_depth = shader_data->shadow_map->get_color(pos.x, pos.y).r;

			//std::cout << "current_depth:" << current_depth << "   closest_depth" << closest_depth << std::endl;
			return current_depth < closest_depth;
		}

		return 1;
	}
};

static float saturate(float value) {
	return value > 1 ? 1 : value < 0 ? 0 : value;
}

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