#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "graphics.h"

//https://zhuanlan.zhihu.com/p/162190576 计算机图形学补充2：齐次空间裁剪(Homogeneous Space Clipping)


static float cal_project_distance(clip_plane c_plane, Vector3f vertex)
{
	return (vertex - c_plane.P) * c_plane.Normal;
}

static void liner_interpolate_varyings(shader_struct_v2f* from, shader_struct_v2f* to, shader_struct_v2f* ret, int sizeof_varyings, float t) {
	int num_floats = sizeof_varyings / sizeof(float);
	float* dst = (float*)ret;

	float* from_param = (float*)(from);
	float* to_param = (float*)(to);

	for (int i = 0; i < num_floats; i++) {
		dst[i] = from_param[i] + (to_param[i] - from_param[i]) * t;
	}
}


//clip_plane为裁剪平面的自定义结构体，vert_list存储了待裁剪凸多边形的所有顶点
//num_vert为顶点个数，in_list为需要保留下来的裁剪平面内侧顶点的列表
static int clip_with_plane(clip_plane c_plane, shader_struct_v2f* in_v2f_list, int num_vert, shader_struct_v2f* out_v2f_list)
{
	int i;
	int in_vert_num = 0;
	int previous_index, current_index;

	for (i = 0; i < num_vert; i++)
	{
		//从最后一个点开始，遍历所有边
		current_index = i;
		previous_index = (i - 1 + num_vert) % num_vert;

		shader_struct_v2f* pre_v2f = &in_v2f_list[previous_index]; //边的起始点
		shader_struct_v2f* cur_v2f = &in_v2f_list[current_index];  //边的终止点

		//Vector3f pre_vertex = proj<3>(in_v2f_list[previous_index].clip_pos); //边的起始点
		//Vector3f cur_vertex = proj<3>(in_v2f_list[current_index].clip_pos);  //边的终止点

		float d1 = cal_project_distance(c_plane, proj<3>(pre_v2f->clip_pos));
		float d2 = cal_project_distance(c_plane, proj<3>(cur_v2f->clip_pos));

		//如果该边与裁剪平面有交点，则计算交点并存入in_list
		if (d1 * d2 < 0)
		{
			//float t = get_intersect_ratio(pre_vertex, cur_vertex, c_plane); //求出t值
			//float t = d1 / (d1 - d2);
			float t = (pre_v2f->clip_pos[3] - proj<3>(pre_v2f->clip_pos) * c_plane.Axis) / (pre_v2f->clip_pos[3] - proj<3>(pre_v2f->clip_pos) * c_plane.Axis) - (cur_v2f->clip_pos[3] - cur_v2f->clip_pos[2]);
			//float t = (cur_v2f->clip_pos[3] - cur_v2f->clip_pos[2]) / (cur_v2f->clip_pos[3] - cur_v2f->clip_pos[2]) - (pre_v2f->clip_pos[3] - pre_v2f->clip_pos[2]);
			//vec3 I = vec3_lerp(pre_vertex, cur_vertex, t);
			liner_interpolate_varyings(pre_v2f, cur_v2f, &out_v2f_list[in_vert_num], sizeof(shader_struct_v2f), t);
			//out_v2f_list[in_vert_num] = I;
			in_vert_num++;
		}

		//如果终止点在内侧，直接存入in_list
		if (d2 < 0)
		{
			out_v2f_list[in_vert_num] = *cur_v2f;
			in_vert_num++;
		}
	}

	return in_vert_num;
}

int draw_triangles(shader_struct_v2f* in_v2f_list, shader_struct_v2f* out_v2f_list)
{
	//int i;
	////vertex shader
	//for (i = 0; i < 3; i++)
	//{
	//	shader.vertex_shader(nface, i);
	//}

	//homogeneous clipping
	int num_vertex = 7476;

	shader_struct_v2f in_list1[3 * 10];
	shader_struct_v2f in_list2[3 * 10];
	shader_struct_v2f in_list3[3 * 10];
	shader_struct_v2f in_list4[3 * 10];
	shader_struct_v2f in_list5[3 * 10];
	shader_struct_v2f in_list6[3 * 10];

	float fov_y = TO_RADIANS(60);

	clip_plane W_PLANE{ Vector3f(0, 0, 0.00001), Vector3f(0, 0, -1), Vector3f(0, 0, 1) };
	clip_plane X_RIGHT{ Vector3f(0, 0, 0), Vector3f(cos(fov_y / 2), 0, -sin(fov_y / 2)), Vector3f(1, 0, 0) };
	clip_plane X_LEFT{ Vector3f(0, 0, 0), Vector3f(-cos(fov_y / 2), 0, -sin(fov_y / 2)), Vector3f(1, 0, 0) };
	clip_plane Y_TOP{ Vector3f(0, 0, 0), Vector3f(0, cos(fov_y / 2), -sin(fov_y / 2)), Vector3f(0, 1, 0) };
	clip_plane Y_BOTTOM{ Vector3f(0, 0, 0), Vector3f(0, -cos(fov_y / 2), -sin(fov_y / 2)), Vector3f(0, 1, 0) };
	clip_plane Z_NEAR{ Vector3f(0, 0, 0.1), Vector3f(0, 0, -1), Vector3f(0, 0, 1) };
	clip_plane Z_FAR{ Vector3f(0, 0, 10000), Vector3f(0, 0, 1) , Vector3f(0, 0, 1) };

	//num_vertex = clip_with_plane(W_PLANE, in_v2f_list, num_vertex, in_list1);
	//num_vertex = clip_with_plane(X_RIGHT, in_list1, num_vertex, in_list2);
	//num_vertex = clip_with_plane(X_LEFT, in_list2, num_vertex, in_list3);
	//num_vertex = clip_with_plane(Y_TOP, in_list3, num_vertex, in_list4);
	//num_vertex = clip_with_plane(Y_BOTTOM, in_list4, num_vertex, in_list5);
	//num_vertex = clip_with_plane(Z_NEAR, in_list5, num_vertex, in_list6);
	//num_vertex = clip_with_plane(Z_FAR, in_list6, num_vertex, out_v2f_list);

	num_vertex = clip_with_plane(Z_NEAR, in_v2f_list, num_vertex, out_v2f_list);

	////triangle assembly
	//for (int i = 0; i < num_vertex - 2; i++) {
	//	//构成三角面的3个顶点索引
	//	int index0 = 0;
	//	int index1 = i + 1;
	//	int index2 = i + 2;

	//	Vector4f clipcoord_attri[3];
	//	clipcoord_attri[0] = in_list7[index0];
	//	clipcoord_attri[1] = in_list7[index1];
	//	clipcoord_attri[2] = in_list7[index2];

	//	rasterize_triangle(clipcoord_attri, framebuffer, zbuffer, shader);
	//}

	return num_vertex;
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

//扫描线法
//void rasterize_triangle(DrawData* draw_data, shader_struct_v2f* v2f)
//{
//	RenderBuffer* render_buffer = draw_data->renderbuffer;
//	Vector3f ndc_coords[3];
//	for (int i = 0; i < 3; i++) ndc_coords[i] = proj<3>(v2f[i].clip_pos / v2f[i].clip_pos[3]);
//
//	// 背面剔除
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
//	//整个三角形的y轴长度
//	int total_height = t2.y - t0.y;
//
//	//沿y轴从最低点遍历到最高顶点的高度
//	for (int i = 0; i < total_height; i++)
//	{
//		//是否为第二部分(上半部分)，高于或等于中间顶点为上半部分，低于中间顶点为下半部分
//		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
//
//		//上半部分高度为最高顶点减去中间顶点的高度，下半部分为中间顶点减去最低顶点的高度
//		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
//
//		float alpha = (float)i / total_height;
//		//下半部分直接取i来计算，上半部分需要把i减去下半部分的高度
//		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
//
//		Vector2f A = t0 + (t2 - t0) * alpha;//求出当前y坐标对应的最高顶点与最低顶点连线上的点
//		Vector2f B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;//求出当前y坐标对应的中间顶点与最低顶点连线上的点
//		//沿x轴，从小到大画点，若A点的x比B点的x大，则交换两个点
//		if (A.x > B.x) std::swap(A, B);
//		for (int j = A.x; j <= B.x; j++) {
//			Vector2i P(j, t0.y + i);
//			if (P.x < 0 || P.y < 0 || P.x >= render_buffer->width || P.y >= render_buffer->height) continue;
//			Vector3f barycentric_weights = barycentric(screen_coords[0], screen_coords[1], screen_coords[2], P);
//			// 深度插值
//			float frag_depth = interpolate_depth(screen_depth, barycentric_weights);
//
//			// 深度测试
//			if (frag_depth > render_buffer->get_depth(P.x, P.y)) continue;
//
//			// 变量插值
//			shader_struct_v2f interpolate_v2f;
//			interpolate_varyings(v2f, &interpolate_v2f, sizeof(shader_struct_v2f), barycentric_weights, recip_w);
//
//			// fragment shader
//			Color color;
//			bool discard = draw_data->shader->fragment(&interpolate_v2f, color);
//
//			// 绘制像素
//			if (!discard) {
//				render_buffer->set_depth(P.x, P.y, frag_depth);
//				render_buffer->set_color(P.x, P.y, color);
//			}
//		}
//	}
//}

static void rasterize_triangle(DrawData* draw_data, shader_struct_v2f* v2f) {
	// 齐次除法 / 透视除法 (homogeneous division / perspective division)
	Vector3f ndc_coords[3];
	for (int i = 0; i < 3; i++) ndc_coords[i] = proj<3>(v2f[i].clip_pos / v2f[i].clip_pos[3]);

	// 背面剔除
	//if (is_back_facing(ndc_coords)) return;

	RenderBuffer* render_buffer = draw_data->render_buffer;

	// 屏幕映射
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

			// 深度插值
			float frag_depth = interpolate_depth(screen_depth, barycentric_weights);

			// 深度测试
			if (frag_depth > render_buffer->get_depth(P.x, P.y)) continue;

			// 变量插值
			shader_struct_v2f interpolate_v2f;
			interpolate_varyings(v2f, &interpolate_v2f, sizeof(shader_struct_v2f), barycentric_weights, recip_w);

			// fragment shader
			Color color;
			bool discard = draw_data->shader->fragment(&interpolate_v2f, color);

			// 绘制像素
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

//void graphics_draw_triangle(DrawData* draw_data) {
//	shader_struct_v2f v2fs[3];
//	for (int i = 0; i < draw_data->model->nfaces(); i++) {
//		for (int j = 0; j < 3; j++) {
//			shader_struct_a2v a2v;
//			a2v.obj_pos = draw_data->model->vert(i, j);
//			a2v.obj_normal = draw_data->model->normal(i, j);
//			a2v.uv = draw_data->model->uv(i, j);
//			v2fs[j] = draw_data->shader->vertex(&a2v);
//		}
//
//		shader_struct_v2f clip_v2fs[3 * 10];
//
//		int num_vertex = draw_triangles(v2fs, clip_v2fs);
//
//		//triangle assembly
//		shader_struct_v2f ret_v2fs[3];
//		for (int i = 0; i < num_vertex - 2; i++) {
//			//构成三角面的3个顶点索引
//			int index0 = 0;
//			int index1 = i + 1;
//			int index2 = i + 2;
//
//
//			ret_v2fs[0] = clip_v2fs[index0];
//			ret_v2fs[1] = clip_v2fs[index1];
//			ret_v2fs[2] = clip_v2fs[index2];
//			rasterize_triangle(draw_data, ret_v2fs);
//		}
//
//		//rasterize_triangle(draw_data, v2fs);
//	}
//}


//void graphics_draw_triangle(DrawData* draw_data) {
//	shader_struct_v2f* v2fs = new shader_struct_v2f[3 * 10000];
//	int index = 0;
//	for (int i = 0; i < draw_data->model->nfaces(); i++) {
//		for (int j = 0; j < 3; j++) {
//			shader_struct_a2v a2v;
//			a2v.obj_pos = draw_data->model->vert(i, j);
//			a2v.obj_normal = draw_data->model->normal(i, j);
//			a2v.uv = draw_data->model->uv(i, j);
//			v2fs[index] = draw_data->shader->vertex(&a2v);
//			index++;
//		}
//	}
//
//	shader_struct_v2f* clip_v2fs = new shader_struct_v2f[3 * 10000 * 10];
//
//	int num_vertex = draw_triangles(v2fs, clip_v2fs);
//
//	//triangle assembly
//	shader_struct_v2f ret_v2fs[3];
//	for (int i = 0; i < num_vertex - 2; i++) {
//		//构成三角面的3个顶点索引
//		int index0 = 0;
//		int index1 = i + 1;
//		int index2 = i + 2;
//
//
//		ret_v2fs[0] = clip_v2fs[index0];
//		ret_v2fs[1] = clip_v2fs[index1];
//		ret_v2fs[2] = clip_v2fs[index2];
//		rasterize_triangle(draw_data, ret_v2fs);
//	}
//}

//
//typedef enum {
//	POSITIVE_W,
//	POSITIVE_X,
//	NEGATIVE_X,
//	POSITIVE_Y,
//	NEGATIVE_Y,
//	POSITIVE_Z,
//	NEGATIVE_Z
//} plane_t;
//
//static int is_inside_plane(Vector4f coord, plane_t plane) {
//	switch (plane) {
//	case POSITIVE_W:
//		return coord.w >= EPSILON;
//	case POSITIVE_X:
//		return coord.x <= +coord.w;
//	case NEGATIVE_X:
//		return coord.x >= -coord.w;
//	case POSITIVE_Y:
//		return coord.y <= +coord.w;
//	case NEGATIVE_Y:
//		return coord.y >= -coord.w;
//	case POSITIVE_Z:
//		return coord.z <= +coord.w;
//	case NEGATIVE_Z:
//		return coord.z >= -coord.w;
//	default:
//		assert(0);
//		return 0;
//	}
//}
//
//static float get_intersect_ratio(Vector4f prev, Vector4f curr, plane_t plane) {
//	switch (plane) {
//	case POSITIVE_W:
//		return (prev.w - EPSILON) / (prev.w - curr.w);
//	case POSITIVE_X:
//		return (prev.w - prev.x) / ((prev.w - prev.x) - (curr.w - curr.x));
//	case NEGATIVE_X:
//		return (prev.w + prev.x) / ((prev.w + prev.x) - (curr.w + curr.x));
//	case POSITIVE_Y:
//		return (prev.w - prev.y) / ((prev.w - prev.y) - (curr.w - curr.y));
//	case NEGATIVE_Y:
//		return (prev.w + prev.y) / ((prev.w + prev.y) - (curr.w + curr.y));
//	case POSITIVE_Z:
//		return (prev.w - prev.z) / ((prev.w - prev.z) - (curr.w - curr.z));
//	case NEGATIVE_Z:
//		return (prev.w + prev.z) / ((prev.w + prev.z) - (curr.w + curr.z));
//	default:
//		assert(0);
//		return 0;
//	}
//}
//
//static int clip_against_plane(
//	plane_t plane, int in_num_vertices, int varying_num_floats,
//	shader_struct_v2f* in_v2fs,
//	shader_struct_v2f* out_v2fs) {
//	int out_num_vertices = 0;
//	int i, j;
//
//	assert(in_num_vertices >= 3 && in_num_vertices <= MAX_VARYINGS);
//	for (i = 0; i < in_num_vertices; i++) {
//		int prev_index = (i - 1 + in_num_vertices) % in_num_vertices;
//		int curr_index = i;
//		vec4_t prev_coord = in_coords[prev_index];
//		vec4_t curr_coord = in_coords[curr_index];
//		float* prev_varyings = (float*)in_varyings[prev_index];
//		float* curr_varyings = (float*)in_varyings[curr_index];
//		int prev_inside = is_inside_plane(prev_coord, plane);
//		int curr_inside = is_inside_plane(curr_coord, plane);
//
//		if (prev_inside != curr_inside) {
//			vec4_t* dest_coord = &out_coords[out_num_vertices];
//			float* dest_varyings = (float*)out_varyings[out_num_vertices];
//			float ratio = get_intersect_ratio(prev_coord, curr_coord, plane);
//
//			*dest_coord = vec4_lerp(prev_coord, curr_coord, ratio);
//			/*
//			 * since this computation is performed in clip space before
//			 * division by w, clipped varying values are perspective-correct
//			 */
//			for (j = 0; j < varying_num_floats; j++) {
//				dest_varyings[j] = float_lerp(prev_varyings[j],
//					curr_varyings[j],
//					ratio);
//			}
//			out_num_vertices += 1;
//		}
//
//		if (curr_inside) {
//			vec4_t* dest_coord = &out_coords[out_num_vertices];
//			float* dest_varyings = (float*)out_varyings[out_num_vertices];
//			int sizeof_varyings = varying_num_floats * sizeof(float);
//
//			*dest_coord = curr_coord;
//			memcpy(dest_varyings, curr_varyings, sizeof_varyings);
//			out_num_vertices += 1;
//		}
//	}
//	assert(out_num_vertices <= MAX_VARYINGS);
//	return out_num_vertices;
//}
//
//#define CLIP_IN2OUT(plane)                                                  \
//    do {                                                                    \
//        num_vertices = clip_against_plane(                                  \
//            plane, num_vertices, varying_num_floats,                        \
//            in_coords, in_varyings, out_coords, out_varyings);              \
//        if (num_vertices < 3) {                                             \
//            return 0;                                                       \
//        }                                                                   \
//    } while (0)
//
//#define CLIP_OUT2IN(plane)                                                  \
//    do {                                                                    \
//        num_vertices = clip_against_plane(                                  \
//            plane, num_vertices, varying_num_floats,                        \
//            out_coords, out_varyings, in_coords, in_varyings);              \
//        if (num_vertices < 3) {                                             \
//            return 0;                                                       \
//        }                                                                   \
//    } while (0)
//
//static int is_vertex_visible(Vector4f v) {
//	return fabs(v.x) <= v.w && fabs(v.y) <= v.w && fabs(v.z) <= v.w;
//}
//
//static int clip_triangle(
//	int sizeof_varyings,
//	shader_struct_v2f* in_v2fs,
//	shader_struct_v2f* out_v2fs) {
//	int v0_visible = is_vertex_visible(in_v2fs[0].clip_pos);
//	int v1_visible = is_vertex_visible(in_v2fs[1].clip_pos);
//	int v2_visible = is_vertex_visible(in_v2fs[2].clip_pos);
//	if (v0_visible && v1_visible && v2_visible) {
//		//out_coords[0] = in_coords[0];
//		//out_coords[1] = in_coords[1];
//		//out_coords[2] = in_coords[2];
//		//memcpy(out_varyings[0], in_varyings[0], sizeof_varyings);
//		//memcpy(out_varyings[1], in_varyings[1], sizeof_varyings);
//		//memcpy(out_varyings[2], in_varyings[2], sizeof_varyings);
//
//		void* _in_v2fs = &in_v2fs[0];
//		void* _out_v2fs = &out_v2fs[0];
//
//		memcpy(&out_v2fs[0], &in_v2fs[0], sizeof(shader_struct_v2f));
//		memcpy(&out_v2fs[1], &in_v2fs[1], sizeof(shader_struct_v2f));
//		memcpy(&out_v2fs[2], &in_v2fs[2], sizeof(shader_struct_v2f));
//		return 3;
//	}
//	else {
//		int varying_num_floats = sizeof_varyings / sizeof(float);
//		int num_vertices = 3;
//		CLIP_IN2OUT(POSITIVE_W);
//		CLIP_OUT2IN(POSITIVE_X);
//		CLIP_IN2OUT(NEGATIVE_X);
//		CLIP_OUT2IN(POSITIVE_Y);
//		CLIP_IN2OUT(NEGATIVE_Y);
//		CLIP_OUT2IN(POSITIVE_Z);
//		CLIP_IN2OUT(NEGATIVE_Z);
//		return num_vertices;
//	}
//}