#include "renderbuffer.h"


/* framebuffer management */

renderbuffer* renderbuffer_create(int width, int height) {
	int color_buffer_size = width * height * 4;
	int depth_buffer_size = sizeof(float) * width * height;
	Color default_color = { 0, 0, 0, 1 };
	float default_depth = 1;
	renderbuffer* fb = new renderbuffer();

	assert(width > 0 && height > 0);

	fb->width = width;
	fb->height = height;
	fb->color_buffer = (unsigned char*)malloc(color_buffer_size);
	fb->depth_buffer = (float*)malloc(depth_buffer_size);

	renderbuffer_clear_color(fb, default_color);
	renderbuffer_clear_depth(fb, default_depth);

	return fb;
}

void renderbuffer_release(renderbuffer* framebuffer) {
	free(framebuffer->color_buffer);
	free(framebuffer->depth_buffer);
	free(framebuffer);
}

void renderbuffer_clear_color(renderbuffer* framebuffer, Color color) {
	int num_pixels = framebuffer->width * framebuffer->height;
	int i;
	for (i = 0; i < num_pixels; i++) {
		framebuffer->color_buffer[i * 4 + 0] = color.r * 255;
		framebuffer->color_buffer[i * 4 + 1] = color.g * 255;
		framebuffer->color_buffer[i * 4 + 2] = color.b * 255;
		framebuffer->color_buffer[i * 4 + 3] = color.a * 255;
	}
}

void renderbuffer_clear_depth(renderbuffer* framebuffer, float depth) {
	int num_pixels = framebuffer->width * framebuffer->height;
	int i;
	for (i = 0; i < num_pixels; i++) {
		framebuffer->depth_buffer[i] = depth;
	}
}