#pragma once

#include "api.h"
#include "gameobject.h"
#include "camera.h"
#include "input.h"
#include "platform.h"

static Vector3f light_dir = Vector3f(1, 1, 1);
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

class Scene {
protected:
	Camera* camera;
	window_t* window;
	RenderBuffer* frameBuffer;
	Record record;
	callbacks_t callbacks;
public:
	Scene(window_t* window, RenderBuffer* frameBuffer);
	~Scene();

	virtual void tick(float delta_time);
	void reset_camera_record();
};

class SingleModelScene :public Scene {
private:
	GameObject_StaticModel* gameobject;
	ShaderData* shader_data;
	DrawData* shadow_draw_data;
	DrawData* draw_data;
	RenderBuffer* shdaow_map = nullptr;
	bool enable_shadow;
	IShader* shader;
	ShadowShader* shadow_shader;
	Matrial* material;
public:
	SingleModelScene(const char* modelName, window_t* window, RenderBuffer* renderBuffer);
	~SingleModelScene();

	void tick(float delta_time) override;
};