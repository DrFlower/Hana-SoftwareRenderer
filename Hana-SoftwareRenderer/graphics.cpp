#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "graphics.h"

//https://zhuanlan.zhihu.com/p/162190576 �����ͼ��ѧ����2����οռ�ü�(Homogeneous Space Clipping)

//clip_planeΪ�ü�ƽ����Զ���ṹ�壬vert_list�洢�˴��ü�͹����ε����ж���
//num_vertΪ���������in_listΪ��Ҫ���������Ĳü�ƽ���ڲඥ����б�
//static int clip_with_plane(clip_plane c_plane, Vector3f* vert_list, int num_vert, Vector3f* in_list)
//{
//	int i;
//	int in_vert_num = 0;
//	int previous_index, current_index;
//
//	for (i = 0; i < num_vert; i++)
//	{
//		//�����һ���㿪ʼ���������б�
//		current_index = i;
//		previous_index = (i - 1 + num_vert) % num_vert;
//		Vector3f pre_vertex = vert_list[previous_index]; //�ߵ���ʼ��
//		Vector3f cur_vertex = vert_list[current_index];  //�ߵ���ֹ��
//
//		float d1 = cal_project_distance(c_plane, pre_vertex);
//		float d2 = cal_project_distance(c_plane, cur_vertex);
//
//		//����ñ���ü�ƽ���н��㣬����㽻�㲢����in_list
//		if (d1 * d2 < 0)
//		{
//			float t = get_intersect_ratio(pre_vertex, cur_vertex, c_plane); //���tֵ
//			vec3 I = vec3_lerp(pre_vertex, cur_vertex, t);
//			in_list[in_vert_num] = I;
//			in_vert_num++;
//		}
//
//		//�����ֹ�����ڲֱ࣬�Ӵ���in_list
//		if (d2 < 0)
//		{
//			in_list[in_vert_num] = cur_vertex;
//			in_vert_num++;
//		}
//	}
//
//	return in_vert_num;
//}
//
//void draw_triangles(Vector3f* in_vert_list, Vector3f* out_vert_list)
//{
//	//int i;
//	////vertex shader
//	//for (i = 0; i < 3; i++)
//	//{
//	//	shader.vertex_shader(nface, i);
//	//}
//
//	//homogeneous clipping
//	int num_vertex = 3;
//
//	num_vertex = clip_with_plane(W_PLANE, vert_list, num_vertex, in_list1);
//	num_vertex = clip_with_plane(X_RIGHT, in_list1, num_vertex, in_list2);
//	num_vertex = clip_with_plane(X_LEFT, in_list2, num_vertex, in_list3);
//	num_vertex = clip_with_plane(Y_TOP, in_list3, num_vertex, in_list4);
//	num_vertex = clip_with_plane(Y_BOTTOM, in_list4, num_vertex, in_list5);
//	num_vertex = clip_with_plane(Z_NEAR, in_list5, num_vertex, in_list6);
//	num_vertex = clip_with_plane(Z_FAR, in_list6, num_vertex, in_list7);
//
//	//triangle assembly
//	for (int i = 0; i < num_vertex - 2; i++) {
//		//�����������3����������
//		int index0 = 0;
//		int index1 = i + 1;
//		int index2 = i + 2;
//
//		Vector4f clipcoord_attri[3];
//		clipcoord_attri[0] = in_list7[index0];
//		clipcoord_attri[1] = in_list7[index1];
//		clipcoord_attri[2] = in_list7[index2];
//
//		rasterize_triangle(clipcoord_attri, framebuffer, zbuffer, shader);
//	}
//}

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

//ɨ���߷�
//void rasterize_triangle(DrawData* draw_data, shader_struct_v2f* v2f)
//{
//	RenderBuffer* render_buffer = draw_data->renderbuffer;
//	Vector3f ndc_coords[3];
//	for (int i = 0; i < 3; i++) ndc_coords[i] = proj<3>(v2f[i].clip_pos / v2f[i].clip_pos[3]);
//
//	// �����޳�
//	if (is_back_facing(ndc_coords)) return;
//
//	Vector2f screen_coords[3];
//	float screen_depth[3];
//	for (int i = 0; i < 3; i++) {
//		Vector3f win_coord = viewport_transform(render_buffer->width, render_buffer->height, ndc_coords[i]);
//		screen_coords[i] = Vector2f(win_coord.x, win_coord.y);
//		screen_depth[i] = win_coord.z;
//	}
//
//	float recip_w[3];
//	/* reciprocals of w */
//	for (int i = 0; i < 3; i++) {
//		recip_w[i] = 1 / v2f[i].clip_pos[3];
//	}
//
//	Vector2f t0 = screen_coords[0];
//	Vector2f t1 = screen_coords[1];
//	Vector2f t2 = screen_coords[2];
//
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
//		Vector2f A = t0 + (t2 - t0) * alpha;//�����ǰy�����Ӧ����߶�������Ͷ��������ϵĵ�
//		Vector2f B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;//�����ǰy�����Ӧ���м䶥������Ͷ��������ϵĵ�
//		//��x�ᣬ��С���󻭵㣬��A���x��B���x���򽻻�������
//		if (A.x > B.x) std::swap(A, B);
//		for (int j = A.x; j <= B.x; j++) {
//			Vector2i P(j, t0.y + i);
//			if (P.x < 0 || P.y < 0 || P.x >= render_buffer->width || P.y >= render_buffer->height) continue;
//			Vector3f barycentric_weights = barycentric(screen_coords[0], screen_coords[1], screen_coords[2], P);
//			// ��Ȳ�ֵ
//			float frag_depth = interpolate_depth(screen_depth, barycentric_weights);
//
//			// ��Ȳ���
//			if (frag_depth > render_buffer->get_depth(P.x, P.y)) continue;
//
//			// ������ֵ
//			shader_struct_v2f interpolate_v2f;
//			interpolate_varyings(v2f, &interpolate_v2f, sizeof(shader_struct_v2f), barycentric_weights, recip_w);
//
//			// fragment shader
//			Color color;
//			bool discard = draw_data->shader->fragment(&interpolate_v2f, color);
//
//			// ��������
//			if (!discard) {
//				render_buffer->set_depth(P.x, P.y, frag_depth);
//				render_buffer->set_color(P.x, P.y, color);
//			}
//		}
//	}
//}

static void rasterize_triangle(DrawData* draw_data, shader_struct_v2f* v2f) {
	// ��γ��� / ͸�ӳ��� (homogeneous division / perspective division)
	Vector3f ndc_coords[3];
	for (int i = 0; i < 3; i++) ndc_coords[i] = proj<3>(v2f[i].clip_pos / v2f[i].clip_pos[3]);

	// �����޳�
	if (is_back_facing(ndc_coords)) return;

	RenderBuffer* render_buffer = draw_data->render_buffer;

	// ��Ļӳ��
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