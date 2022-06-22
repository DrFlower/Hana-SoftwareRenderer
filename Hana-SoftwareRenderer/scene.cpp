#include "scene.h"

Scene::Scene(window_t* window, RenderBuffer* renderBuffer) {
	this->window = window;
	this->frameBuffer = renderBuffer;
	float aspect = (float)this->frameBuffer->width / (float)this->frameBuffer->height;
	camera = new Camera(CAMERA_POSITION, CAMERA_TARGET, aspect);

	record = Record();
	callbacks = callbacks_t();

	callbacks.button_callback = button_callback;
	callbacks.scroll_callback = scroll_callback;

	window_set_userdata(window, &record);
	input_set_callbacks(window, callbacks);

	light_dir.normalize();
}

void Scene::tick(float delta_time) {
	update_camera(window, camera, &record);
}

Scene::~Scene() {
	delete camera;
}

void Scene::reset_camera_record()
{
	record.orbit_delta = Vector2f(0, 0);
	record.pan_delta = Vector2f(0, 0);
	record.dolly_delta = 0;
	record.single_click = 0;
	record.double_click = 0;
}

SingleModelScene::SingleModelScene(const char* modelName, window_t* window, RenderBuffer* renderBuffer) :Scene(window, renderBuffer) {
	gameobject = new GameObject_StaticModel(modelName);

	material = new Matrial();
	material->diffuse_map = gameobject->model->get_diffuse_map();
	material->normal_map = gameobject->model->get_normal_map();
	material->specular_map = gameobject->model->get_specular_map();
	material->color = Color::White;
	material->specular = Color::White;
	material->gloss = 50;
	material->bump_scale = 1;

	shader_data = new ShaderData();
	shader_data->matrial = material;

	//GroundShader ground_shader = GroundShader();
	//ToonShader toon_shader = ToonShader();
	//TextureShader texture_shader = TextureShader();
	//TextureWithLightShader text_with_light_shader = TextureWithLightShader();
	//BlinnShader blinn_shader = BlinnShader();
	shader = new BlinnShader();
	shadow_shader = new ShadowShader();

	enable_shadow = false;

	shadow_draw_data = nullptr;
	draw_data = new DrawData();

	draw_data->model = gameobject->model;
	draw_data->shader = shader;
	draw_data->shader->shader_data = shader_data;
	draw_data->renderbuffer = this->frameBuffer;
}

SingleModelScene::~SingleModelScene() {
	delete gameobject;
	delete material;
	delete shader_data;
	delete shader;
	delete shadow_shader;
	delete draw_data;

	if (shadow_draw_data)  delete shadow_draw_data;
	if (shdaow_map)  delete shdaow_map;
}

void SingleModelScene::tick(float delta_time) {
	Scene::tick(delta_time);

	Matrix4x4 view_matrix = this->camera->get_view_matrix();
	Matrix4x4 projection_matrix = this->camera->get_proj_matrix();
	Matrix4x4 model_matrix = gameobject->GetModelMatrix();
	Matrix4x4 model_matrix_I = model_matrix.invert();

	shader_data->view_Pos = this->camera->get_position();
	shader_data->light_dir = light_dir;
	shader_data->light_color = LightColor;
	shader_data->ambient = AMBIENT;
	shader_data->model_matrix = model_matrix;
	shader_data->model_matrix_I = model_matrix_I;
	shader_data->view_matrix = view_matrix;
	shader_data->projection_matrix = projection_matrix;
	float aspect = (float)this->frameBuffer->width / (float)this->frameBuffer->height;
	shader_data->light_vp_matrix = orthographic(aspect, 1, 0, 5) * lookat(Vector3f(1, 1, 1), Vector3f(0, 0, 0), { 0, 1, 0 });
	shader_data->camera_vp_matrix = projection_matrix * view_matrix;

	if (enable_shadow)
	{
		if (!shadow_draw_data)
		{
			shdaow_map = new RenderBuffer(this->frameBuffer->width, this->frameBuffer->height);
			shadow_draw_data = new DrawData();
			shadow_draw_data->model = gameobject->model;
			shadow_draw_data->shader = shadow_shader;
			shadow_draw_data->shader->shader_data = shader_data;
			shadow_draw_data->renderbuffer = shdaow_map;
		}

		graphics_draw_triangle(shadow_draw_data);
		shader_data->shadow_map = shdaow_map;
	}

	graphics_draw_triangle(draw_data);


	if (enable_shadow)
	{
		shdaow_map->renderbuffer_clear_color(Color::Black);
		shdaow_map->renderbuffer_clear_depth(std::numeric_limits<float>::max());
	}
}