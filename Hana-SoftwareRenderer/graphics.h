#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "mathapi.h"
#include "renderbuffer.h"
#include "model.h"
#include "IShader.h"

struct DrawData {
	Model* model;
	IShader* shader;
	RenderBuffer* render_buffer;
};

struct clip_plane {
	Vector3f P;
	Vector3f Normal;
	Vector3f Axis;

	clip_plane(Vector3f in_P, Vector3f in_Normal, Vector3f in_Axis) {
		P = in_P;
		Normal = in_Normal;
		Axis = in_Axis;
	}
};



void graphics_draw_triangle(DrawData* app_data);
#endif
