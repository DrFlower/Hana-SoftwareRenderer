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

static bool is_back_facing(Vector3f* ndc_coords) {
	Vector3f a = ndc_coords[0];
	Vector3f b = ndc_coords[1];
	Vector3f c = ndc_coords[2];
	float signed_area = a.x * b.y - a.y * b.x +
		b.x * c.y - b.y * c.x +
		c.x * a.y - c.y * a.x;
	return signed_area <= 0;
}

static float interpolate_depth(Vector3f* screen_coords, Vector3f weights) {
	Vector3f screen_depth;
	for (size_t i = 0; i < 3; i++)
	{
		screen_depth[i] = screen_coords[i].z;
	}

	return screen_depth * weights;
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

static void rasterize_triangle(framebuffer* framebuffer, AppData* appdata, shader_struct_v2f* v2f) {

	// 齐次除法 / 透视除法 (homogeneous division / perspective division)
	Vector3f ndc_coords[3];
	for (int i = 0; i < 3; i++) ndc_coords[i] = proj<3>(v2f[i].clip_pos / v2f[i].clip_pos[3]);

	// 背面剔除
	if (is_back_facing(ndc_coords)) return;

	//
	Matrix<4, 3, float> m_clip_coords;
	for (int i = 0; i < 3; i++) m_clip_coords.setCol(i, v2f[i].clip_pos);
	Matrix<3, 4, float> m_screen_coords = (Viewport * m_clip_coords).transpose(); // transposed to ease access to each of the points
	Vector3f screen_coords[3];
	for (int i = 0; i < 3; i++) screen_coords[i] = proj<3>(m_screen_coords[i]);


	Matrix<3, 2, float> pts2;
	for (int i = 0; i < 3; i++) pts2[i] = proj<2>(m_screen_coords[i] / m_screen_coords[i][3]);
	//for (int i = 0; i < 3; i++) pts2[i] = proj<2>(screen_coords[i]);

	Vector2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vector2f clamp(framebuffer->width - 1, framebuffer->height - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts2[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
		}
	}

	Vector2i P;
	shader_struct_v2f interpolate_v2f;
	Matrix<4, 3, float> varying_clip_pos;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vector3f barycentric_weights = barycentric(pts2[0], pts2[1], pts2[2], P);
			if (barycentric_weights.x < 0 || barycentric_weights.y < 0 || barycentric_weights.z < 0) continue;


			Vector3f bc_clip = Vector3f(barycentric_weights.x / m_screen_coords[0][3], barycentric_weights.y / m_screen_coords[1][3], barycentric_weights.z / m_screen_coords[2][3]);
			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
			//float frag_depth = clip_coords[2] * bc_clip;

			float frag_depth = interpolate_depth(screen_coords, barycentric_weights);

			//Matrix<4, 3, float> varying_clip_pos, varying_world_pos, varying_clip_uv, varying_clip_normal;
			//for (int i = 0; i < 3; i++) { varying_clip_pos.setCol(i, v2f[i].clip_pos); }
			//for (int i = 0; i < 3; i++) { varying_world_pos.setCol(i, embed<4>(v2f[i].world_pos)); }
			//for (int i = 0; i < 3; i++) { varying_clip_uv.setCol(i, embed<4>(v2f[i].uv)); }
			//for (int i = 0; i < 3; i++) { varying_clip_normal.setCol(i, embed<4>(v2f[i].world_normal)); }


			//interpolate_v2f.clip_pos = varying_clip_pos * bc_clip;
			//interpolate_v2f.world_pos = proj<3>(varying_world_pos * bc_clip);
			//interpolate_v2f.uv = proj<2>(varying_clip_uv * bc_clip);
			//interpolate_v2f.world_normal = proj<3>(varying_clip_normal * bc_clip);

			
			for (int i = 0; i < 3; i++) { varying_clip_pos.setCol(i, v2f[i].clip_pos); }
			interpolate_v2f.clip_pos = varying_clip_pos * bc_clip;

			/*		shader_struct_v2f interpolate_v2f;
					interpolate_v2f.clip_pos = v2f->clip_pos;
					interpolate_v2f.world_pos = v2f->world_pos;
					interpolate_v2f.uv = v2f->uv;
					interpolate_v2f.world_normal = v2f->world_normal;*/

			if (framebuffer->get_depth(P.x, P.y) > frag_depth) continue;
			Color color;
			bool discard = appdata->matrial->shader->fragment(interpolate_v2f, color);

			if (!discard) {
				framebuffer->set_depth(P.x, P.y, frag_depth);
				framebuffer->set_color(P.x, P.y, color);
			}
		}
	}
}

void graphics_draw_triangle(framebuffer* framebuffer, AppData* appdata) {
	shader_struct_v2f v2fs[3];
	for (int i = 0; i < appdata->model->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			shader_struct_a2v a2v;
			a2v.obj_pos = appdata->model->vert(i, j);
			//a2v.obj_normal = appdata->model->normal(i, j);
			//a2v.uv = appdata->model->uv(i, j);
			v2fs[j] = appdata->matrial->shader->vertex(a2v);
		}

		rasterize_triangle(framebuffer, appdata, v2fs);
	}

	//int num_vertices;
	//int i;

	///* execute vertex shader */
	//for (i = 0; i < 3; i++) {
	//    vec4_t clip_coord = program->vertex_shader(program->shader_attribs[i],
	//        program->in_varyings[i],
	//        program->shader_uniforms);
	//    program->in_coords[i] = clip_coord;
	//}

	///* triangle clipping */
	//num_vertices = clip_triangle(program->sizeof_varyings,
	//    program->in_coords, program->in_varyings,
	//    program->out_coords, program->out_varyings);

	///* triangle assembly */
	//for (i = 0; i < num_vertices - 2; i++) {
	//    int index0 = 0;
	//    int index1 = i + 1;
	//    int index2 = i + 2;
	//    vec4_t clip_coords[3];
	//    void* varyings[3];
	//    int is_culled;

	//    clip_coords[0] = program->out_coords[index0];
	//    clip_coords[1] = program->out_coords[index1];
	//    clip_coords[2] = program->out_coords[index2];
	//    varyings[0] = program->out_varyings[index0];
	//    varyings[1] = program->out_varyings[index1];
	//    varyings[2] = program->out_varyings[index2];

	//    is_culled = rasterize_triangle(framebuffer, program,
	//        clip_coords, varyings);
	//    if (is_culled) {
	//        break;
	//    }
	//}
}