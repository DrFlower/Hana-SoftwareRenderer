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
#include "scene.h"

static const char* const WINDOW_TITLE = "Hana-SoftwareRenderer";
static const int WINDOW_WIDTH = 1000;
static const int WINDOW_HEIGHT = 600;

Model* model = NULL;

//Matrix4x4 camera_get_light_view_matrix(Vector3f position, Vector3f target, Vector3f UP) {
//	Matrix4x4 m = lookat(position, target, UP);
//	return m;
//}
//
//static Matrix4x4 get_light_proj_matrix(float aspect, float size,
//	float z_near, float z_far) {
//	return orthographic(aspect, size, z_near, z_far);
//}

int main()
{
	platform_initialize();
	window_t* window;
	RenderBuffer* framebuffer = nullptr;
	RenderBuffer* shdaow_map = nullptr;

	float aspect;
	float prev_time;
	float print_time;
	int num_frames;

	//light_dir.normalize();

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = new RenderBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

	num_frames = 0;
	prev_time = platform_get_time();
	print_time = prev_time;

	SingleModelScene scene = SingleModelScene("african_head.obj", window, framebuffer);

	while (!window_should_close(window)) {
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time;

		scene.tick(delta_time);

		num_frames += 1;
		if (curr_time - print_time >= 1) {
			int sum_millis = (int)((curr_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;
			printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
			num_frames = 0;
			print_time = curr_time;
		}
		prev_time = curr_time;

		window_draw_buffer(window, framebuffer);

		scene.reset_camera_record();

		framebuffer->renderbuffer_clear_color(Color::Black);
		framebuffer->renderbuffer_clear_depth(std::numeric_limits<float>::max());

		input_poll_events();
	}

	window_destroy(window);
	delete framebuffer;
}