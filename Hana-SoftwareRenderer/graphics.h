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

//struct clip_plane {
//	Vector3f P;
//	Vector3f Normal;
//
//	clip_plane(Vector3f in_P, Vector3f in_Normal) {
//		P = in_P;
//		Normal = in_Normal;
//	}
//};
//
//float fov = 60;
//
//clip_plane W_PLANE{ Vector3f(0, 0, 0), Vector3f(0, 0, 0) };
//clip_plane X_RIGHT{ Vector3f(0, 0, 0), Vector3f(cos(fov / 2), 0, sin(fov / 2)) };
//clip_plane X_LEFT{ Vector3f(0, 0, 0), Vector3f(-cos(fov / 2), 0, sin(fov / 2)) };
//clip_plane Y_TOP{ Vector3f(0, 0, 0), Vector3f(0, cos(fov / 2), sin(fov / 2)) };
//clip_plane Y_BOTTOM{ Vector3f(0, 0, 0), Vector3f(0, -cos(fov / 2), sin(fov / 2)) };
//clip_plane Z_NEAR{ Vector3f(0, 0, 0), Vector3f(0, 0, 1) };
//clip_plane Z_FAR{ Vector3f(0, 0, 0), Vector3f(0, 0, -1) };

void graphics_draw_triangle(DrawData* app_data);
#endif
