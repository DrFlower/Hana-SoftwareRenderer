#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"
#include "model.h"

Matrix4x4 ModelView;
Matrix4x4 Viewport;
Matrix4x4 Projection;

IShader::~IShader() {}

void viewport(int x, int y, int w, int h) {
	Viewport = Matrix4x4::identity();
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = depth / 2.f;
	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
	Viewport[2][2] = depth / 2.f;
}

void projection(float coeff) {
	Projection = Matrix4x4::identity();
	Projection[3][2] = coeff;
}

void lookat(Vector3f eye, Vector3f center, Vector3f up) {
	Vector3f z = (eye - center).normalize();
	Vector3f x = cross(up, z).normalize();
	Vector3f y = cross(z, x).normalize();
	Matrix4x4 Minv = Matrix4x4::identity();
	Matrix4x4 Tr = Matrix4x4::identity();
	for (int i = 0; i < 3; i++) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
		Minv[2][i] = z[i];
		Tr[i][3] = -center[i];
	}
	ModelView = Minv * Tr;
}

Vector3f barycentric(Vector2f A, Vector2f B, Vector2f C, Vector2f P) {
	Vector3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vector3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vector3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vector3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Matrix<4, 3, float>& clipc, Model* model, IShader& shader, framebuffer_t* frameBuffer) {
	Matrix<3, 4, float> pts = (Viewport * clipc).transpose(); // transposed to ease access to each of the points
	Matrix<3, 2, float> pts2;
	for (int i = 0; i < 3; i++) pts2[i] = proj<2>(pts[i] / pts[i][3]);

	Vector2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vector2f clamp(frameBuffer->width - 1, frameBuffer->height - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts2[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
		}
	}
	Vector2i P;
	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vector3f bc_screen = barycentric(pts2[0], pts2[1], pts2[2], P);
			Vector3f bc_clip = Vector3f(bc_screen.x / pts[0][3], bc_screen.y / pts[1][3], bc_screen.z / pts[2][3]);
			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
			float frag_depth = clipc[2] * bc_clip;
			color = TGAColor(frag_depth * 255, frag_depth * 255, frag_depth * 255);

			/*bool in = false;

			Vector2f check1 = Vector2f(339.f / 1200.f * 800, 72.f / 1200.f * 800);
			Vector2f check2 = Vector2f(821.f / 1200.f * 800, 440.f / 1200.f * 800);
			if (P.x > check1.x && P.y > check1.y && P.x < check2.x && P.y < check2.y)
			{
				in = true;
			}

			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
			{
				if(in)
					color = TGAColor(255, 0, 0);
				else
				{
					continue;
				}
			}
			

			if (frameBuffer->get_depth(P.x, P.y) > frag_depth)
			{
				if(in)
					color = TGAColor(0, 255, 0);
				else
				{
					continue;
				}
			}

			bool discard = false;

			if (bc_screen.x > 0 && bc_screen.y > 0 && bc_screen.z>0 && frameBuffer->get_depth(P.x, P.y) <= frag_depth)
			{
				discard = shader.fragment(model, bc_clip, color);
			}*/


			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z<0 || frameBuffer->get_depth(P.x, P.y) > frag_depth) continue;
			bool discard = shader.fragment(model, bc_clip, color);

			if (!discard) {
				frameBuffer->set_depth(P.x, P.y, frag_depth);
				frameBuffer->set_color(P.x, P.y, color);
			}
		}
	}
}

