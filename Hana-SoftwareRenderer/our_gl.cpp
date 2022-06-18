#include <cmath>
#include <limits>
#include <cstdlib>
#include "our_gl.h"
#include "model.h"

Matrix4x4 ModelMatrix = Matrix4x4::identity();
Matrix4x4 ModelView;
Matrix4x4 Viewport;
Matrix4x4 Projection;

Vector3f light_dir = Vector3f(1, 1, 1);
//extern Color AMBIENT = Color(50, 50, 50);
extern Color AMBIENT = Color::White * 0.3f;
extern Color LightColor = Color::White * 0.8f;

IShader_old::~IShader_old() {}

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

bool is_back_facing(Vector3f* ndc_coords) {
	Vector3f a = ndc_coords[0];
	Vector3f b = ndc_coords[1];
	Vector3f c = ndc_coords[2];
	float signed_area = a.x * b.y - a.y * b.x +
		b.x * c.y - b.y * c.x +
		c.x * a.y - c.y * a.x;
	return signed_area <= 0;
}

/*
 * for depth interpolation, see subsection 3.5.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
 //float interpolate_depth(float screen_depths[3], vec3_t weights) {
 //	float depth0 = screen_depths[0] * weights.x;
 //	float depth1 = screen_depths[1] * weights.y;
 //	float depth2 = screen_depths[2] * weights.z;
 //	return depth0 + depth1 + depth2;
 //}

//float interpolate_depth(Vector3f* screen_coords, Vector3f weights) {
//	float t = 0;
//	for (size_t i = 0; i < 3; i++)
//	{
//		t += weights[i] / screen_coords[i].z;
//	}
//	return (1 / t);
//}

float interpolate_depth(Vector3f* screen_coords, Vector3f weights) {
	Vector3f screen_depth;
	for (size_t i = 0; i < 3; i++)
	{
		screen_depth[i] = screen_coords[i].z;
	}

	return screen_depth * weights;
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

void triangle(Matrix<4, 3, float>& clip_coords, Model* model, IShader_old& shader, framebuffer* frameBuffer) {
	Matrix<3, 4, float> pts = (Viewport * clip_coords).transpose(); // transposed to ease access to each of the points

	Vector3f screen_coords[3];
	for (int i = 0; i < 3; i++) screen_coords[i] = proj<3>(pts[i]);

	Vector3f pts1[3];
	for (int i = 0; i < 3; i++) pts1[i] = proj<3>(pts[i] / pts[i][3]);
	if (is_back_facing(pts1)) return;

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
			Vector3f barycentric_weights = barycentric(pts2[0], pts2[1], pts2[2], P);
			if (barycentric_weights.x < 0 || barycentric_weights.y < 0 || barycentric_weights.z < 0) continue;


			Vector3f bc_clip = Vector3f(barycentric_weights.x / pts[0][3], barycentric_weights.y / pts[1][3], barycentric_weights.z / pts[2][3]);
			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
			//float frag_depth = clip_coords[2] * bc_clip;

			float frag_depth = interpolate_depth(screen_coords, barycentric_weights);

			if (frameBuffer->get_depth(P.x, P.y) > frag_depth) continue;
			bool discard = shader.fragment(model, bc_clip, color);

			if (!discard) {
				frameBuffer->set_depth(P.x, P.y, frag_depth);
				frameBuffer->set_color(P.x, P.y, color);
			}
		}
	}
}

//����һ���л��°벿���������߼���չ�����Ի�������������
//��������������ڲ����ص��߼�
//�������Ѿ�ʵ����2D�����εĻ���
//void triangle(Matrix<4, 3, float>& clipc, Model* model, IShader& shader, framebuffer_t* frameBuffer)
//{
//	Matrix<3, 4, float> pts = (Viewport * clipc).transpose(); // transposed to ease access to each of the points
//
//	Vector3f pts1[3];
//	for (int i = 0; i < 3; i++) pts1[i] = proj<3>(pts[i] / pts[i][3]);
//	if (is_back_facing(pts1)) return;
//
//	Matrix<3, 2, float> pts2;
//	for (int i = 0; i < 3; i++) pts2[i] = proj<2>(pts[i] / pts[i][3]);
//
//	Vector2i t0 = pts2[0];
//	Vector2i t1 = pts2[1];
//	Vector2i t2 = pts2[2];
//	if (t0.y > t1.y) std::swap(t0, t1);
//	if (t0.y > t2.y) std::swap(t0, t2);
//	if (t1.y > t2.y) std::swap(t1, t2);
//
//	//���������ε�y�᳤��
//	int total_height = t2.y - t0.y;
//
//	//��y�����͵��������߶���ĸ߶�
//	for (int i = 0; i < total_height; i++)
//	{
//		//�Ƿ�Ϊ�ڶ�����(�ϰ벿��)�����ڻ�����м䶥��Ϊ�ϰ벿�֣������м䶥��Ϊ�°벿��
//		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
//
//		//�ϰ벿�ָ߶�Ϊ��߶����ȥ�м䶥��ĸ߶ȣ��°벿��Ϊ�м䶥���ȥ��Ͷ���ĸ߶�
//		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
//
//		float alpha = (float)i / total_height;
//		//�°벿��ֱ��ȡi�����㣬�ϰ벿����Ҫ��i��ȥ�°벿�ֵĸ߶�
//		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
//
//		Vector2i A = t0 + (t2 - t0) * alpha;//�����ǰy�����Ӧ����߶�������Ͷ��������ϵĵ�
//		Vector2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;//�����ǰy�����Ӧ���м䶥������Ͷ��������ϵĵ�
//		//��x�ᣬ��С���󻭵㣬��A���x��B���x���򽻻�������
//		if (A.x > B.x) std::swap(A, B);
//
//		for (int j = A.x; j <= B.x; j++) {
//			Vector2i P = Vector2i(j, t0.y + i);
//			Vector3f bc_screen = barycentric(pts2[0], pts2[1], pts2[2], P);
//			Vector3f bc_clip = Vector3f(bc_screen.x / pts[0][3], bc_screen.y / pts[1][3], bc_screen.z / pts[2][3]);
//			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
//			float frag_depth = clipc[2] * bc_clip;
//			TGAColor color;
//
//			if (frameBuffer->get_depth(P.x, P.y) > frag_depth) continue;
//			bool discard = shader.fragment(model, bc_clip, color);
//			if (!discard) {
//				frameBuffer->set_depth(P.x, P.y, frag_depth);
//				frameBuffer->set_color(P.x, P.y, color);
//			}
//		}
//	}
//}
//

//void triangle(Matrix<4, 3, float>& clip_coords, Model* model, IShader2& shader, framebuffer_t* frameBuffer) {
//	Matrix<3, 4, float> pts = (Viewport * clip_coords).transpose(); // transposed to ease access to each of the points
//
//	Vector3f screen_coords[3];
//	for (int i = 0; i < 3; i++) screen_coords[i] = proj<3>(pts[i]);
//
//	Vector3f pts1[3];
//	for (int i = 0; i < 3; i++) pts1[i] = proj<3>(pts[i] / pts[i][3]);
//	if (is_back_facing(pts1)) return;
//
//	Matrix<3, 2, float> pts2;
//	for (int i = 0; i < 3; i++) pts2[i] = proj<2>(pts[i] / pts[i][3]);
//
//	Vector2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
//	Vector2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
//	Vector2f clamp(frameBuffer->width - 1, frameBuffer->height - 1);
//	for (int i = 0; i < 3; i++) {
//		for (int j = 0; j < 2; j++) {
//			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts2[i][j]));
//			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
//		}
//	}
//
//	Vector2i P;
//	TGAColor color;
//	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
//		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
//			Vector3f barycentric_weights = barycentric(pts2[0], pts2[1], pts2[2], P);
//			if (barycentric_weights.x < 0 || barycentric_weights.y < 0 || barycentric_weights.z < 0) continue;
//
//
//			Vector3f bc_clip = Vector3f(barycentric_weights.x / pts[0][3], barycentric_weights.y / pts[1][3], barycentric_weights.z / pts[2][3]);
//			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
//			//float frag_depth = clip_coords[2] * bc_clip;
//
//			float frag_depth = interpolate_depth(screen_coords, barycentric_weights);
//
//			if (frameBuffer->get_depth(P.x, P.y) > frag_depth) continue;
//			bool discard = shader.fragment(model, bc_clip, color);
//
//			if (!discard) {
//				frameBuffer->set_depth(P.x, P.y, frag_depth);
//				frameBuffer->set_color(P.x, P.y, color);
//			}
//		}
//	}
//}