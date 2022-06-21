#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "macro.h"

/*
 * for viewport transformation, see subsection 2.12.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
static Vector3f viewport_transform(int width, int height, Vector3f ndc_coord) {
	float x = (ndc_coord.x + 1) * 0.5f * (float)width;   /* [-1, 1] -> [0, w] */
	float y = (ndc_coord.y + 1) * 0.5f * (float)height;  /* [-1, 1] -> [0, h] */
	float z = (ndc_coord.z + 1) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return Vector3f(x, y, z);
}

/*
 * for facing determination, see subsection 3.5.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 *
 * this is the same as (but more efficient than)
 *     vec3_t ab = vec3_sub(b, a);
 *     vec3_t ac = vec3_sub(c, a);
 *     return vec3_cross(ab, ac).z <= 0;
 */
static bool is_back_facing(Vector3f* ndc_coords) {
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
static float interpolate_depth(float* screen_depths, Vector3f weights) {
	Vector3f screen_depth;
	for (size_t i = 0; i < 3; i++)
	{
		screen_depth[i] = screen_depths[i];
	}

	return screen_depth * weights;
}

/*
 * for perspective correct interpolation, see
 * https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 *
 * equation 15 in reference 1 (page 2) is a simplified 2d version of
 * equation 3.5 in reference 2 (page 58) which uses barycentric coordinates
 */

static void interpolate_varyings(shader_struct_v2f* v2f, shader_struct_v2f* ret, int sizeof_varyings, Vector3f weights, float recip_w[3]) {
	int num_floats = sizeof_varyings / sizeof(float);
	float* src0 = (float*)(&v2f[0]);
	float* src1 = (float*)(&v2f[1]);
	float* src2 = (float*)(&v2f[2]);
	float* dst = (float*)ret;
	float weight0 = recip_w[0] * weights.x;
	float weight1 = recip_w[1] * weights.y;
	float weight2 = recip_w[2] * weights.z;
	float normalizer = 1 / (weight0 + weight1 + weight2);
	int i;
	for (i = 0; i < num_floats; i++) {
		float sum = src0[i] * weight0 + src1[i] * weight1 + src2[i] * weight2;
		dst[i] = sum * normalizer;
	}
}

static Vector3f barycentric(Vector2f A, Vector2f B, Vector2f C, Vector2f P) {
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

static void rasterize_triangle(DrawData* draw_data, shader_struct_v2f* v2f) {

	renderbuffer* render_buffer = draw_data->renderbuffer;

	// ��γ��� / ͸�ӳ��� (homogeneous division / perspective division)
	Vector3f ndc_coords[3];
	for (int i = 0; i < 3; i++) ndc_coords[i] = proj<3>(v2f[i].clip_pos / v2f[i].clip_pos[3]);

	// �����޳�
	if (is_back_facing(ndc_coords)) return;

	Vector2f screen_coords[3];
	float screen_depth[3];
	for (int i = 0; i < 3; i++) {
		Vector3f win_coord = viewport_transform(render_buffer->width, render_buffer->height, ndc_coords[i]);
		screen_coords[i] = Vector2f(win_coord.x, win_coord.y);
		screen_depth[i] = win_coord.z;
	}

	float recip_w[3];
	/* reciprocals of w */
	for (int i = 0; i < 3; i++) {
		recip_w[i] = 1 / v2f[i].clip_pos[3];
	}

	Vector2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vector2f clamp(render_buffer->width - 1, render_buffer->height - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], screen_coords[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], screen_coords[i][j]));
		}
	}

	Vector2i P;

	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vector3f barycentric_weights = barycentric(screen_coords[0], screen_coords[1], screen_coords[2], P);
			if (barycentric_weights.x < 0 || barycentric_weights.y < 0 || barycentric_weights.z < 0) continue;

			// ��Ȳ�ֵ
			float frag_depth = interpolate_depth(screen_depth, barycentric_weights);

			// ��Ȳ���
			if (frag_depth > render_buffer->get_depth(P.x, P.y)) continue;

			// ������ֵ
			shader_struct_v2f interpolate_v2f;
			interpolate_varyings(v2f, &interpolate_v2f, sizeof(shader_struct_v2f), barycentric_weights, recip_w);

			// fragment shader
			Color color;
			bool discard = draw_data->shader->fragment(&interpolate_v2f, color);

			// ��������
			if (!discard) {
				render_buffer->set_depth(P.x, P.y, frag_depth);
				render_buffer->set_color(P.x, P.y, color);
			}
		}
	}
}

void graphics_draw_triangle(DrawData* draw_data) {
	shader_struct_v2f v2fs[3];
	for (int i = 0; i < draw_data->model->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			shader_struct_a2v a2v;
			a2v.obj_pos = draw_data->model->vert(i, j);
			a2v.obj_normal = draw_data->model->normal(i, j);
			a2v.uv = draw_data->model->uv(i, j);
			v2fs[j] = draw_data->shader->vertex(&a2v);
		}

		rasterize_triangle(draw_data, v2fs);
	}
}