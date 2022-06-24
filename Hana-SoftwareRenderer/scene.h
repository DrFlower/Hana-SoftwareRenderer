#pragma once

#include "api.h"
#include "gameobject.h"
#include "camera.h"
#include "input.h"
#include "platform.h"

static Vector3f light_dir = Vector3f(1, 1, 1).normalize();
static Color AMBIENT = Color(54.f / 255, 58.f / 255, 66.f / 255);
static Color LightColor = Color(255.f / 255, 244.f / 255, 214.f / 255);

//Matrix4x4 camera_get_light_view_matrix(Vector3f position, Vector3f target, Vector3f UP) {
//	Matrix4x4 m = lookat(position, target, UP);
//	return m;
//}
//
//static Matrix4x4 get_light_proj_matrix(float aspect, float size,
//	float z_near, float z_far) {
//	return orthographic(aspect, size, z_near, z_far);
//}


class DrawModel {
public:
	GameObject_StaticModel* gameobject;
	Matrial* material;
	ShaderData* shader_data;
	DrawData* shadow_draw_data;
	DrawData* draw_data;
	RenderBuffer* shdaow_map;
	IShader* shader;
	ShadowShader* shadow_shader;


	DrawModel(GameObject_StaticModel* go, Matrial* material, IShader* shader)
	{
		this->gameobject = go;
		this->material = material;
		this->shader = shader;

		shader_data = new ShaderData();
		shader_data->matrial = material;

		shadow_draw_data = nullptr;

		draw_data = new DrawData();
		draw_data->model = gameobject->model;
		draw_data->shader = shader;
		draw_data->shader->shader_data = shader_data;
	}

	~DrawModel()
	{
		delete shader_data;
		delete draw_data;
		if (shadow_shader) delete shadow_shader;
		if (shdaow_map)  delete shdaow_map;
		if (shadow_draw_data)  delete shadow_draw_data;
	}

	void draw(Camera* camera, RenderBuffer* frameBuffer, bool enable_shadow)
	{
		Matrix4x4 view_matrix = camera->get_view_matrix();
		Matrix4x4 projection_matrix = camera->get_proj_matrix();
		Matrix4x4 model_matrix = gameobject->GetModelMatrix();
		Matrix4x4 model_matrix_I = model_matrix.invert();

		shader_data->view_Pos = camera->get_position();
		shader_data->light_dir = light_dir;
		shader_data->light_color = LightColor;
		shader_data->ambient = AMBIENT;
		shader_data->model_matrix = model_matrix;
		shader_data->model_matrix_I = model_matrix_I;
		shader_data->view_matrix = view_matrix;
		shader_data->projection_matrix = projection_matrix;
		float aspect = (float)frameBuffer->width / (float)frameBuffer->height;
		shader_data->light_vp_matrix = orthographic(aspect, 1, 0, 5) * lookat(Vector3f(1, 1, 1), Vector3f(0, 0, 0), { 0, 1, 0 });
		shader_data->camera_vp_matrix = projection_matrix * view_matrix;

		if (enable_shadow)
		{
			if (!shadow_draw_data)
			{
				shadow_shader = new ShadowShader();
				shdaow_map = new RenderBuffer(frameBuffer->width, frameBuffer->height);
				shadow_draw_data = new DrawData();
				shadow_draw_data->model = gameobject->model;
				shadow_draw_data->shader = shadow_shader;
				shadow_draw_data->shader->shader_data = shader_data;
				shadow_draw_data->renderbuffer = shdaow_map;
			}

			graphics_draw_triangle(shadow_draw_data);
			shader_data->shadow_map = shdaow_map;
		}

		draw_data->renderbuffer = frameBuffer;
		graphics_draw_triangle(draw_data);


		if (enable_shadow)
		{
			shdaow_map->renderbuffer_clear_color(Color::Black);
			shdaow_map->renderbuffer_clear_depth(std::numeric_limits<float>::max());
		}
	}
};

class Scene {
public:
	Camera* camera;
	RenderBuffer* frameBuffer;
	callbacks_t callbacks;
	Scene(RenderBuffer* frameBuffer);
	~Scene();

	virtual void tick(float delta_time);
};

class SingleModelScene :public Scene {
private:
	GameObject_StaticModel* gameobject;
	Matrial* material;
	IShader* shader;
	DrawModel* draw_model;
	bool enable_shadow;
public:
	SingleModelScene(const char* modelName, RenderBuffer* renderBuffer);
	~SingleModelScene();

	void tick(float delta_time) override;
};

class MultiModelScene :public Scene {
private:
	GameObject_StaticModel* gameobject;
	Matrial* material;
	IShader* shader;
	DrawModel* draw_model;
	bool enable_shadow;
public:
	MultiModelScene(RenderBuffer* renderBuffer);
	~MultiModelScene();

	void AddModel(int index, const char* filename, Matrial* material, IShader* shader, Vector3f position = Vector3f::Zero, Vector3f rotation = Vector3f::Zero, Vector3f scale = Vector3f::One);

	void tick(float delta_time) override;
};