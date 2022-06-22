#include "renderbuffer.h"

RenderBuffer::RenderBuffer(int width, int height) {
	int color_buffer_size = width * height * 4;
	int depth_buffer_size = sizeof(float) * width * height;
	Color default_color = { 0, 0, 0, 1 };
	float default_depth = 1;

	assert(width > 0 && height > 0);

	this->width = width;
	this->height = height;
	this->color_buffer = (unsigned char*)malloc(color_buffer_size);
	this->depth_buffer = (float*)malloc(depth_buffer_size);

	this->renderbuffer_clear_color(default_color);
	this->renderbuffer_clear_depth(default_depth);
}

RenderBuffer::~RenderBuffer()
{
	free(color_buffer);
	free(depth_buffer);
}


void RenderBuffer::set_depth(int x, int y, float depth) {
	int index = y * width + x;
	depth_buffer[index] = depth;
}

float RenderBuffer::get_depth(int x, int y)
{
	int index = y * width + x;
	return depth_buffer[index];
}

void RenderBuffer::set_color(int x, int y, Color color)
{
	int index = (y * width + x) * 4;
	color_buffer[index + 0] = color.r * 255;
	color_buffer[index + 1] = color.g * 255;
	color_buffer[index + 2] = color.b * 255;
}

Color RenderBuffer::get_color(int x, int y)
{
	int index = (y * width + x) * 4;
	return Color(color_buffer[index + 0] / 255.f, color_buffer[index + 1] / 255.f, color_buffer[index + 2] / 255.f);
}


void RenderBuffer::renderbuffer_release() {
	free(color_buffer);
	free(depth_buffer);
	free(this);
}

void RenderBuffer::renderbuffer_clear_color(Color color) {
	int num_pixels = this->width * this->height;
	int i;
	for (i = 0; i < num_pixels; i++) {
		this->color_buffer[i * 4 + 0] = color.r * 255;
		this->color_buffer[i * 4 + 1] = color.g * 255;
		this->color_buffer[i * 4 + 2] = color.b * 255;
		this->color_buffer[i * 4 + 3] = color.a * 255;
	}
}

void RenderBuffer::renderbuffer_clear_depth(float depth) {
	int num_pixels = this->width * this->height;
	int i;
	for (i = 0; i < num_pixels; i++) {
		this->depth_buffer[i] = depth;
	}
}