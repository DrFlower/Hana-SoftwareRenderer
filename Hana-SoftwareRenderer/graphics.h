#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "maths.h"
#include "model.h"
#include "tgaimage.h"
#include "color.h"
#include "IShader.h"

class framebuffer {
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

	void set_color(int x, int y, TGAColor color)
	{
		int index = y * width + x;
		color_buffer[index * 4 + 0] = color.bgra[2];
		color_buffer[index * 4 + 1] = color.bgra[1];
		color_buffer[index * 4 + 2] = color.bgra[0];
	}
};

/* framebuffer management */
framebuffer* framebuffer_create(int width, int height);
void framebuffer_release(framebuffer* framebuffer);
void framebuffer_clear_color(framebuffer* framebuffer, vec4_t color);
void framebuffer_clear_depth(framebuffer* framebuffer, float depth);



#endif
