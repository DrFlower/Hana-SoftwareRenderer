#include "iostream"
#include "platform.h"

static const char* const WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

int main()
{
	std::cout << "hello world" << std::endl;
	platform_initialize();
	window_t* window;
	framebuffer_t* framebuffer;
	float prev_time;
	float print_time;
	int num_frames;

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);

	for (int x = 0; x <= WINDOW_WIDTH; x++)
	{
		for (int y = 0; y < WINDOW_HEIGHT; y++)
		{
			if (x >= WINDOW_WIDTH / 4 && x <= WINDOW_WIDTH / 4 * 3 && y >= WINDOW_HEIGHT / 4 && y <= WINDOW_HEIGHT / 4 * 3)
			{
				int index = y * WINDOW_WIDTH + x;
				framebuffer->color_buffer[index * 4 + 0] = float_to_uchar(1);
				framebuffer->color_buffer[index * 4 + 1] = float_to_uchar(0);
				framebuffer->color_buffer[index * 4 + 2] = float_to_uchar(0);
				framebuffer->depth_buffer[index] = 0;
			}
		}
	}



	num_frames = 0;
	prev_time = platform_get_time();
	print_time = prev_time;
	while (!window_should_close(window)) {
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time;

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

		input_poll_events();
	}

	window_destroy(window);
}