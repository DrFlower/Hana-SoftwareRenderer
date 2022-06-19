#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "macro.h"
#include "maths.h"
#include "IShader.h"
#include "our_gl.h"

/* framebuffer management */

framebuffer* framebuffer_create(int width, int height) {
	int color_buffer_size = width * height * 4;
	int depth_buffer_size = sizeof(float) * width * height;
	vec4_t default_color = { 0, 0, 0, 1 };
	float default_depth = 1;
	framebuffer* fb = new framebuffer();

	assert(width > 0 && height > 0);

	fb->width = width;
	fb->height = height;
	fb->color_buffer = (unsigned char*)malloc(color_buffer_size);
	fb->depth_buffer = (float*)malloc(depth_buffer_size);

	framebuffer_clear_color(fb, default_color);
	framebuffer_clear_depth(fb, default_depth);

	return fb;
}

void framebuffer_release(framebuffer* framebuffer) {
	free(framebuffer->color_buffer);
	free(framebuffer->depth_buffer);
	free(framebuffer);
}

void framebuffer_clear_color(framebuffer* framebuffer, vec4_t color) {
	int num_pixels = framebuffer->width * framebuffer->height;
	int i;
	for (i = 0; i < num_pixels; i++) {
		framebuffer->color_buffer[i * 4 + 0] = float_to_uchar(color.x);
		framebuffer->color_buffer[i * 4 + 1] = float_to_uchar(color.y);
		framebuffer->color_buffer[i * 4 + 2] = float_to_uchar(color.z);
		framebuffer->color_buffer[i * 4 + 3] = float_to_uchar(color.w);
	}
}

void framebuffer_clear_depth(framebuffer* framebuffer, float depth) {
	int num_pixels = framebuffer->width * framebuffer->height;
	int i;
	for (i = 0; i < num_pixels; i++) {
		framebuffer->depth_buffer[i] = depth;
	}
}

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
static void interpolate_varyings(
	void* src_varyings[3], void* dst_varyings,
	int sizeof_varyings, vec3_t weights, float recip_w[3]) {
	int num_floats = sizeof_varyings / sizeof(float);
	float* src0 = (float*)src_varyings[0];
	float* src1 = (float*)src_varyings[1];
	float* src2 = (float*)src_varyings[2];
	float* dst = (float*)dst_varyings;
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

static void rasterize_triangle(framebuffer* framebuffer, DrawData* appdata, shader_struct_v2f* v2f) {

	// ��γ��� / ͸�ӳ��� (homogeneous division / perspective division)
	Vector3f ndc_coords[3];
	for (int i = 0; i < 3; i++) ndc_coords[i] = proj<3>(v2f[i].clip_pos / v2f[i].clip_pos[3]);

	// �����޳�
	if (is_back_facing(ndc_coords)) return;

	Vector2f screen_coords[3];
	float screen_depth[3];
	for (int i = 0; i < 3; i++) {
		Vector3f win_coord = viewport_transform(framebuffer->width, framebuffer->height, ndc_coords[i]);
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
	Vector2f clamp(framebuffer->width - 1, framebuffer->height - 1);
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
			if (frag_depth > framebuffer->get_depth(P.x, P.y)) continue;

			// ������ֵ
			shader_struct_v2f interpolate_v2f;
			interpolate_varyings(v2f, &interpolate_v2f, sizeof(shader_struct_v2f), barycentric_weights, recip_w);

			// fragment shader
			Color color;
			bool discard = appdata->matrial->shader->fragment(&interpolate_v2f, color);

			// ��������
			if (!discard) {
				framebuffer->set_depth(P.x, P.y, frag_depth);
				framebuffer->set_color(P.x, P.y, color);
			}
		}
	}
}

void graphics_draw_triangle(framebuffer* framebuffer, DrawData* appdata) {
	shader_struct_v2f v2fs[3];
	for (int i = 0; i < appdata->model->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			shader_struct_a2v a2v;
			a2v.obj_pos = appdata->model->vert(i, j);
			a2v.obj_normal = appdata->model->normal(i, j);
			a2v.uv = appdata->model->uv(i, j);
			v2fs[j] = appdata->matrial->shader->vertex(&a2v);
		}

		rasterize_triangle(framebuffer, appdata, v2fs);
	}
}