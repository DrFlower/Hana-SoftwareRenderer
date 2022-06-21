#include <vector>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "graphics.h"
#include "iostream"
#include "platform.h"
#include "math.h"
#include "camera.h"
#include "input.h"
#include "IShader.h"
#include "drawdata.h"

static const char* const WINDOW_TITLE = "Hana-SoftwareRenderer";
static const int WINDOW_WIDTH = 1000;
static const int WINDOW_HEIGHT = 600;

static Vector3f light_dir = Vector3f(1, 1, 1);
static Color AMBIENT = Color(54.f / 255, 58.f / 255, 66.f / 255);
static Color LightColor = Color(255.f / 255, 244.f / 255, 214.f / 255);

Model* model = NULL;

Matrix4x4 camera_get_light_view_matrix(Vector3f position, Vector3f target, Vector3f UP) {
	Matrix4x4 m = lookat(position, target, UP);
	return m;
}

static Matrix4x4 get_light_proj_matrix(float aspect, float size,
	float z_near, float z_far) {
	return orthographic(aspect, size, z_near, z_far);
}

int main()
{
	platform_initialize();
	window_t* window;
	renderbuffer* framebuffer = nullptr;
	renderbuffer* shdaow_map = nullptr;
	Camera* camera;
	record_t record;
	callbacks_t callbacks;
	float aspect;
	float prev_time;
	float print_time;
	int num_frames;

	light_dir.normalize();

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = new renderbuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
	aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	camera = new Camera(CAMERA_POSITION, CAMERA_TARGET, aspect);

	record = record_t();

	memset(&callbacks, 0, sizeof(callbacks_t));
	callbacks.button_callback = button_callback;
	callbacks.scroll_callback = scroll_callback;

	model = new Model("african_head.obj");

	Matrial material;
	material.diffuse_map = model->get_diffuse_map();
	material.normal_map = model->get_normal_map();
	material.specular_map = model->get_specular_map();
	material.color = Color::White;
	material.specular = Color::White;
	material.gloss = 50;
	material.bump_scale = 1;

	ShaderData* shader_data = new ShaderData();
	shader_data->matrial = &material;

	GroundShader ground_shader = GroundShader();
	ToonShader toon_shader = ToonShader();
	TextureShader texture_shader = TextureShader();
	TextureWithLightShader text_with_light_shader = TextureWithLightShader();
	BlinnShader blinn_shader = BlinnShader();
	NormalMapShader normalmap_shader = NormalMapShader();
	ShadowShader shadow_shader = ShadowShader();

	bool enable_shadow = true;

	DrawData* shadow_draw_data = NULL;
	DrawData* draw_data = new DrawData();


	draw_data->model = model;
	draw_data->shader = &normalmap_shader;
	draw_data->shader->shader_data = shader_data;
	draw_data->renderbuffer = framebuffer;

	window_set_userdata(window, &record);
	input_set_callbacks(window, callbacks);

	num_frames = 0;
	prev_time = platform_get_time();
	print_time = prev_time;
	while (!window_should_close(window)) {
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time;

		update_camera(window, camera, &record);
		Matrix4x4 view_matrix = camera->get_view_matrix();
		Matrix4x4 projection_matrix = camera->get_proj_matrix();
		Matrix4x4 model_matrix = Matrix4x4::identity();
		Matrix4x4 model_matrix_I = model_matrix.invert();

		shader_data->view_Pos = camera->get_position();
		shader_data->light_dir = light_dir;
		shader_data->light_color = LightColor;
		shader_data->ambient = AMBIENT;
		shader_data->model_matrix = Matrix4x4::identity();
		shader_data->model_matrix_I = model_matrix_I;
		shader_data->view_matrix = view_matrix;
		shader_data->projection_matrix = projection_matrix;
		shader_data->light_vp_matrix = get_light_proj_matrix(aspect, 1, 0, 5) * camera_get_light_view_matrix(Vector3f(1, 1, 1), Vector3f(0, 0, 0), { 0, 1, 0 });
		shader_data->camera_vp_matrix = projection_matrix * view_matrix;

		if (enable_shadow)
		{
			if (!shadow_draw_data)
			{
				shdaow_map = new renderbuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
				shadow_draw_data = new DrawData();
				shadow_draw_data->model = model;
				shadow_draw_data->shader = &shadow_shader;
				shadow_draw_data->shader->shader_data = shader_data;
				shadow_draw_data->renderbuffer = shdaow_map;
			}

			graphics_draw_triangle(shadow_draw_data);
			shader_data->shadow_map = shdaow_map;
		}

		graphics_draw_triangle(draw_data);

		window_draw_buffer(window, framebuffer);
		num_frames += 1;
		if (curr_time - print_time >= 1) {
			int sum_millis = (int)((curr_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;
			printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
			num_frames = 0;
			print_time = curr_time;
		}
		prev_time = curr_time;

		record.orbit_delta = Vector2f(0, 0);
		record.pan_delta = Vector2f(0, 0);
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;

		framebuffer->renderbuffer_clear_color(Color::Black);
		framebuffer->renderbuffer_clear_depth(std::numeric_limits<float>::max());

		if (enable_shadow)
		{
			shdaow_map->renderbuffer_clear_color(Color::Black);
			shdaow_map->renderbuffer_clear_depth(std::numeric_limits<float>::max());
		}

		input_poll_events();
	}

	if (model)
	{
		delete model;
	}

	if (shadow_draw_data)
	{
		delete shadow_draw_data;
	}

	delete shader_data;

	window_destroy(window);

	delete framebuffer;
	if (shdaow_map) delete shdaow_map;
	delete camera;
}