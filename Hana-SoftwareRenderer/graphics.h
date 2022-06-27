#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "mathapi.h"
#include "renderbuffer.h"
#include "model.h"
#include "IShader.h"

struct DrawData {
	Model* model;
	IShader* shader;
	RenderBuffer* renderbuffer;
};

void graphics_draw_triangle(DrawData* app_data);
#endif
