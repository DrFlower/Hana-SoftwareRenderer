#pragma once

#include "color.h"

class renderbuffer {
public:
	int width, height;
	unsigned char* color_buffer;
	float* depth_buffer;

	void set_depth(int x, int y, float depth) {
		int index = y * width + x;
		depth_buffer[index] = depth;
	}

	float get_depth(int x, int y)
	{
		int index = y * width + x;
		return depth_buffer[index];
	}

	void set_color(int x, int y, Color color)
	{
		int index = (y * width + x) * 4;
		color_buffer[index + 0] = color.r * 255;
		color_buffer[index + 1] = color.g * 255;
		color_buffer[index + 2] = color.b * 255;
	}

	Color get_color(int x, int y)
	{
		int index = (y * width + x) * 4;
		return Color(color_buffer[index + 0] / 255.f, color_buffer[index + 1] / 255.f, color_buffer[index + 2] / 255.f);
	}

};

/* framebuffer management */
renderbuffer* renderbuffer_create(int width, int height);
void renderbuffer_release(renderbuffer* framebuffer);
void renderbuffer_clear_color(renderbuffer* framebuffer, Color color);
void renderbuffer_clear_depth(renderbuffer* framebuffer, float depth);
