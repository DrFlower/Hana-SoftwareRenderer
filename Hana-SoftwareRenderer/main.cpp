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

int scene_count = 2;
Scene* scene = nullptr;
RenderBuffer* framebuffer = nullptr;

Scene* load_scene(int scene_index) {
	switch (scene_index)
	{
	case 0:
		return new SingleModelScene("african_head.obj", framebuffer);
	case 1:
		return new SingleModelScene("diablo3_pose.obj", framebuffer); 
	default:
		return new SingleModelScene("african_head.obj", framebuffer);
		break;
	}
}


void key_callback(window_t* window, keycode_t key, int pressed) {
	if (scene)
	{
		scene->on_key_input(key, pressed);
	}
}

int main()
{
	platform_initialize();
	window_t* window;
	Record record = Record();
	float aspect;
	float prev_time;
	float print_time;
	int num_frames;

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = new RenderBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

	num_frames = 0;
	prev_time = platform_get_time();
	print_time = prev_time;

	int scene_index = 0;

	scene = load_scene(scene_index);

	callbacks_t callbacks = callbacks_t();

	callbacks.button_callback = button_callback;
	callbacks.scroll_callback = scroll_callback;
	callbacks.key_callback = key_callback;

	window_set_userdata(window, &record);
	input_set_callbacks(window, callbacks);


	while (!window_should_close(window)) {
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time;

		update_camera(window, (*scene).camera, &record);

		if (input_key_pressed(window, KEY_W)) {
			scene_index = (scene_index - 1 + scene_count) % scene_count;
			scene = load_scene(scene_index);
		}
		else if (input_key_pressed(window, KEY_S)) {
			scene_index = (scene_index + 1 + scene_count) % scene_count;
			scene = load_scene(scene_index);
		}

		scene->tick(delta_time);

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

		record.orbit_delta = Vector2f(0, 0);
		record.pan_delta = Vector2f(0, 0);
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;

		framebuffer->renderbuffer_clear_color(Color::Black);
		framebuffer->renderbuffer_clear_depth(std::numeric_limits<float>::max());

		input_poll_events();
	}

	delete scene;
	delete framebuffer;
	window_destroy(window);
}