#include "graphics.h"
#include "iostream"
#include "platform.h"
#include "scene.h"

static const char* const WINDOW_TITLE = "Hana-SoftwareRenderer";
static const int WINDOW_WIDTH = 1000;
static const int WINDOW_HEIGHT = 600;
static const int WINDOW_TEXT_WIDTH = 250;
static const int WINDOW_TEXT_HEIGHT = 310;

struct SceneInfo {
	const char* name;
	Scene* scene;
};

int scene_count = 2;
SceneInfo scene_info;
RenderBuffer* frame_buffer = nullptr;

SceneInfo load_scene(int scene_index) {
	if (scene_info.scene)
	{
		delete scene_info.scene;
	}

	SceneInfo ret;

	switch (scene_index)
	{
	case 0:
		ret.name = "1.african_head";
		ret.scene = new SingleModelScene("african_head.obj", frame_buffer);
		break;
	case 1:
		ret.name = "2.diablo3_pose";
		ret.scene = new SingleModelScene("diablo3_pose.obj", frame_buffer);
		break;
	default:
		ret.name = "1.african_head";
		ret.scene = new SingleModelScene("african_head.obj", frame_buffer);
		break;
	}

	return ret;
}


void key_callback(window_t* window, keycode_t key, int pressed) {
	if (scene_info.scene)
	{
		scene_info.scene->on_key_input(key, pressed);
	}
}

int main()
{
	platform_initialize();
	window_t* window;
	Record record = Record();
	callbacks_t callbacks = callbacks_t();
	float prev_time = platform_get_time();
	float print_time = prev_time;
	int num_frames = 0;
	const int text_size = 500;
	char screen_text[text_size];
	int show_num_frames = 0;
	int show_avg_millis = 0;
	float refresh_screen_text_timer = 0;
	const float REFRESH_SCREEN_TEXT_TIME = 0.1;
	snprintf(screen_text, text_size, "fps: - -, avg: - -ms\n");
	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TEXT_WIDTH, WINDOW_TEXT_HEIGHT);
	frame_buffer = new RenderBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

	int scene_index = 0;

	scene_info = load_scene(scene_index);
	callbacks.button_callback = button_callback;
	callbacks.scroll_callback = scroll_callback;
	callbacks.key_callback = key_callback;

	record.window_width = WINDOW_WIDTH;
	record.window_height = WINDOW_HEIGHT;
	window_set_userdata(window, &record);
	input_set_callbacks(window, callbacks);

	while (!window_should_close(window)) {
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time;

		// 更新摄像机控制
		update_camera(window, scene_info.scene->camera, &record);

		if (input_key_pressed(window, KEY_W)) {
			scene_index = (scene_index - 1 + scene_count) % scene_count;
			scene_info = load_scene(scene_index);
		}
		else if (input_key_pressed(window, KEY_S)) {
			scene_index = (scene_index + 1 + scene_count) % scene_count;
			scene_info = load_scene(scene_index);
		}

		// 更新场景
		scene_info.scene->tick(delta_time);

		// 计算帧率和耗时
		num_frames += 1;
		if (curr_time - print_time >= 1) {
			int sum_millis = (int)((curr_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;

			show_num_frames = num_frames;
			show_avg_millis = avg_millis;
			num_frames = 0;
			print_time = curr_time;
		}
		prev_time = curr_time;

		// 把帧缓存绘制到UI窗口
		window_draw_buffer(window, frame_buffer);

		//更新显示文本信息
		refresh_screen_text_timer += delta_time;
		if (refresh_screen_text_timer > REFRESH_SCREEN_TEXT_TIME)
		{
			snprintf(screen_text, text_size, "");

			char line[50] = "";

			snprintf(line, 50, "fps: %3d, avg: %3d ms\n\n", show_num_frames, show_avg_millis);
			strcat(screen_text, line);

			snprintf(line, 50, "scene: %s\n", scene_info.name);
			strcat(screen_text, line);
			snprintf(line, 50, "press key [W] or [S] to switch scene\n\n");
			strcat(screen_text, line);

			strcat(screen_text, scene_info.scene->get_text());
			
			window_draw_text(window, screen_text);
			refresh_screen_text_timer -= REFRESH_SCREEN_TEXT_TIME;
		}

		// 重置摄像机输入参数
		record.orbit_delta = Vector2f(0, 0);
		record.pan_delta = Vector2f(0, 0);
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;

		// 清除颜色缓存和深度缓存
		frame_buffer->renderbuffer_clear_color(Color::Black);
		frame_buffer->renderbuffer_clear_depth(std::numeric_limits<float>::max());

		input_poll_events();
	}

	delete scene_info.scene;
	delete frame_buffer;
	window_destroy(window);
}