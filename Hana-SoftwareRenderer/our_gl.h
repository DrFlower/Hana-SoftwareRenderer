#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "math.h"
#include "model.h"
#include "graphics.h"

extern Matrix4x4 ModelMatrix;
extern Matrix4x4 ModelView;
extern Matrix4x4 Viewport;
extern Matrix4x4 Projection;
const float depth = 2000.f;
extern Vector3f light_dir;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); // coeff = -1/c
void lookat(Vector3f eye, Vector3f center, Vector3f up);

struct IShader {
	Matrix<4, 3, float> varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
	Matrix<4, 4, float> uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates

	virtual ~IShader();
	virtual Vector4f vertex(Model* model, int iface, int nthvert) = 0;
	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) = 0;
};

//void triangle(Matrix<4, 3, float>& clipc, Model* model, IShader& shader, framebuffer* framebuffer);

#endif //__OUR_GL_H__

