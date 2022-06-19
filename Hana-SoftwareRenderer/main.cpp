#include <vector>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"
#include "graphics.h"
#include "iostream"
#include "platform.h"
#include "math.h"
#include "camera.h"
#include "input.h"
#include "IShader.h"
#include "drawdata.h"

static const char* const WINDOW_TITLE = "Hana-SoftwareRenderer";
static const int WINDOW_WIDTH = 1000;
static const int WINDOW_HEIGHT = 600;

Model* model = NULL;


//Vector3f       eye(1, 1, 3);
//Vector3f    center(0, 0, 0);
//Vector3f        up(0, 1, 0);

struct GouraudShader_old : public IShader_old {
	Vector3f varying_intensity; // written by vertex shader, read by fragment shader

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		Vector4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = Projection * ViewMatrix * gl_Vertex;     // transform it to screen coordinates
		varying_tri.setCol(nthvert, gl_Vertex);
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity
		return gl_Vertex;
	}

	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) {
		float intensity = varying_intensity * bar;   // interpolate intensity for the current pixel
		color = TGAColor(255, 255, 255) * intensity; // well duh
		return false;                              // no, we do not discard this pixel
	}
};

struct ToonShader_old : public IShader_old {
	Vector3f varying_intensity; // written by vertex shader, read by fragment shader

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		Vector4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = Projection * ViewMatrix * gl_Vertex;     // transform it to screen coordinates
		varying_tri.setCol(nthvert, gl_Vertex);
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity
		return gl_Vertex;
	}

	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) {
		float intensity = varying_intensity * bar;
		if (intensity > .85) intensity = 1;
		else if (intensity > .60) intensity = .80;
		else if (intensity > .45) intensity = .60;
		else if (intensity > .30) intensity = .45;
		else if (intensity > .15) intensity = .30;
		color = TGAColor(255, 155, 0) * intensity;
		return false;
	}
};

struct TextureShader_old : public IShader_old {
	Vector3f          varying_intensity; // written by vertex shader, read by fragment shader
	Matrix<2, 3, float> varying_uv;        // same as above

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity
		Vector3f v = model->vert(iface, nthvert);
		Vector4f gl_Vertex = embed<4>(v); // read the vertex from .obj file
		gl_Vertex = Projection * ViewMatrix * gl_Vertex; // transform it to screen coordinates
		varying_tri.setCol(nthvert, gl_Vertex);
		return gl_Vertex;
	}

	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) {
		float intensity = varying_intensity * bar;   // interpolate intensity for the current pixel
		Vector2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
		color = model->diffuse(uv) * intensity;      // well duh
		return false;                              // no, we do not discard this pixel
	}
};

struct NormalmappingShader_old :public IShader_old
{
	Matrix<2, 3, float> varying_uv;  // same as above
	Matrix<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()

	virtual Vector4f vertex(Model* model, int iface, int nthvert) override
	{
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		Vector4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = Projection * ViewMatrix * gl_Vertex; // transform it to screen coordinates
		varying_tri.setCol(nthvert, gl_Vertex);
		uniform_MIT = (Projection * ViewMatrix).invert_transpose();
		return gl_Vertex;
	}
	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) override
	{
		Vector2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
		Vector3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		Vector3f l = proj<3>(embed<4>(light_dir)).normalize();
		float intensity = std::max(0.f, n * l);
		color = model->diffuse(uv) * intensity;      // well duh
		return false;                              // no, we do not discard this pixel
	}
};

struct TangentSpaceNormalmappingShader_old :public IShader_old
{
	Matrix<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	Matrix<4, 4, float> uniform_M;   //  Projection*ModelView
	Matrix<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	Matrix<3, 3, float> ndc_tri;     // triangle in normalized device coordinates

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		varying_nrm.setCol(nthvert, proj<3>((Projection * ViewMatrix).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));
		Vector4f gl_Vertex = Projection * ViewMatrix * embed<4>(model->vert(iface, nthvert));
		varying_tri.setCol(nthvert, gl_Vertex);
		uniform_M = Projection * ViewMatrix;
		ndc_tri.setCol(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) {
		Vector3f bn = (varying_nrm * bar).normalize();
		Vector2f uv = varying_uv * bar;

		Matrix<3, 3, float> A;
		A[0] = ndc_tri.getCol(1) - ndc_tri.getCol(0);
		A[1] = ndc_tri.getCol(2) - ndc_tri.getCol(0);
		A[2] = bn;

		Matrix<3, 3, float> AI = A.invert();

		Vector3f i = AI * Vector3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		Vector3f j = AI * Vector3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		Matrix<3, 3, float> B;
		B.setCol(0, i.normalize());
		B.setCol(1, j.normalize());
		B.setCol(2, bn);

		Vector3f n = (B * model->normal(uv)).normalize();
		Vector3f l = proj<3>(uniform_M * embed<4>(light_dir, 0.f)).normalize();
		float diff = std::max(0.f, n * l);
		color = model->diffuse(uv) * diff;

		return false;
	}
};

struct SpecularShader_old : public IShader_old
{
	Matrix<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	Matrix<4, 4, float> uniform_M;   //  Projection*ModelView
	Matrix<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	Matrix<3, 3, float> ndc_tri;     // triangle in normalized device coordinates

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		varying_nrm.setCol(nthvert, proj<3>((Projection * ViewMatrix).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));
		Vector4f gl_Vertex = Projection * ViewMatrix * embed<4>(model->vert(iface, nthvert));
		varying_tri.setCol(nthvert, gl_Vertex);
		uniform_M = Projection * ViewMatrix;
		ndc_tri.setCol(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) {
		Vector3f bn = (varying_nrm * bar).normalize();
		Vector2f uv = varying_uv * bar;

		Matrix<3, 3, float> A;
		A[0] = ndc_tri.getCol(1) - ndc_tri.getCol(0);
		A[1] = ndc_tri.getCol(2) - ndc_tri.getCol(0);
		A[2] = bn;

		Matrix<3, 3, float> AI = A.invert();

		Vector3f i = AI * Vector3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		Vector3f j = AI * Vector3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		Matrix<3, 3, float> B;
		B.setCol(0, i.normalize());
		B.setCol(1, j.normalize());
		B.setCol(2, bn);

		Vector3f n = (B * model->normal(uv)).normalize();
		Vector3f l = proj<3>(uniform_M * embed<4>(light_dir, 0.f)).normalize();
		Vector3f r = (n * (n * l * 2.f) - l).normalize();   // reflected light
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		float diff = std::max(0.f, n * l);
		TGAColor c = model->diffuse(uv) * diff;
		color = c;
		//5为环境分量，0.6为镜面反射系数
		for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + c[i] * (diff + .6 * spec), 255);
		return false;
	}
};

void RenderModel(std::string modelName, renderbuffer* framebuffer, IShader_old& shader)
{
	if (!model)
	{
		//std::string prefix = "obj";
		//model = new Model(prefix.append(modelName).append(".obj").c_str());
		model = new Model("D:\\Development\\Github\\Hana-SoftwareRenderer\\Hana-SoftwareRenderer\\obj\\african_head.obj");
	}


	for (int i = 0; i < model->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			shader.vertex(model, i, j);
		}
		triangle(shader.varying_tri, model, shader, framebuffer);
	}
}

Matrix4x4 matrix4_lookat(Vector3f eye, Vector3f target, Vector3f up) {
	Vector3f z_axis = (eye - target).normalize();
	Vector3f x_axis = cross(up, z_axis).normalize();
	Vector3f y_axis = cross(z_axis, x_axis);
	Matrix4x4 m = Matrix4x4::identity();

	m[0][0] = x_axis.x;
	m[0][1] = x_axis.y;
	m[0][2] = x_axis.z;

	m[1][0] = y_axis.x;
	m[1][1] = y_axis.y;
	m[1][2] = y_axis.z;

	m[2][0] = z_axis.x;
	m[2][1] = z_axis.y;
	m[2][2] = z_axis.z;

	m[0][3] = -(x_axis * eye);
	m[1][3] = -(y_axis * eye);
	m[2][3] = -(z_axis * eye);

	return m;
}

Matrix4x4 camera_get_light_view_matrix(Vector3f position, Vector3f target, Vector3f UP) {
	vec3_t up = vec3_new(UP.x, UP.y, UP.z);
	Matrix4x4 m = matrix4_lookat(position, target, UP);
	return m;
}

Matrix4x4 Matrix4_orthographic(float aspect, float size, float near, float far) {
	float z_range = far - near;
	Matrix4x4 m = Matrix4x4::identity();
	assert(aspect > 0 && size > 0 && z_range > 0);
	m[0][0] = 1 / aspect * size;
	m[1][1] = 1 / size;
	m[2][2] = -2 / z_range;
	m[2][3] = -(near + far) / z_range;
	return m;
}

static Matrix4x4 get_light_proj_matrix(float aspect, float size,
	float z_near, float z_far) {
	return Matrix4_orthographic(aspect, size, z_near, z_far);
}

int main()
{
	platform_initialize();
	window_t* window;
	renderbuffer* framebuffer = nullptr;
	renderbuffer* shdaow_map = nullptr;
	camera_t* camera;
	record_t record;
	callbacks_t callbacks;
	float aspect;
	float prev_time;
	float print_time;
	int num_frames;

	//lookat(eye, center, up);
	//viewport(WINDOW_WIDTH / 8, WINDOW_HEIGHT / 8, WINDOW_WIDTH * 3 / 4, WINDOW_HEIGHT * 3 / 4);
	//projection(-1.f / (eye - center).normal());


	light_dir.normalize();

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = renderbuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
	aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	camera = camera_create(CAMERA_POSITION, CAMERA_TARGET, aspect);

	record = record_t();

	memset(&callbacks, 0, sizeof(callbacks_t));
	callbacks.button_callback = button_callback;
	callbacks.scroll_callback = scroll_callback;

	GouraudShader_old gouraudShader;
	ToonShader_old toonShader;
	TextureShader_old textureShader;
	NormalmappingShader_old normalmapping;
	TangentSpaceNormalmappingShader_old tangentSpaceNormalmappingShader;
	SpecularShader_old specularShader;


	model = new Model("D:\\Development\\Github\\Hana-SoftwareRenderer\\Hana-SoftwareRenderer\\obj\\diablo3_pose.obj");

	Matrial material;
	material.diffuse_map = model->get_diffuse_map();
	material.normal_map = model->get_normal_map();
	material.specular_map = model->get_specular_map();
	material.color = Color::White;
	material.specular = Color::White;
	material.gloss = 50;
	material.bump_scale = 1;

	ShaderData* shader_data = new ShaderData();
	shader_data->matrial = &material;

	GroundShader ground_shader = GroundShader();
	ToonShader toon_shader = ToonShader();
	TextureShader texture_shader = TextureShader();
	TextureWithLightShader text_with_light_shader = TextureWithLightShader();
	BlinnShader blinn_shader = BlinnShader();
	NormalMapShader normalmap_shader = NormalMapShader();
	ShadowShader shadow_shader = ShadowShader();

	bool enable_shadow = true;

	DrawData* shadow_draw_data = NULL;
	DrawData* draw_data = new DrawData();


	draw_data->model = model;
	draw_data->shader = &normalmap_shader;
	draw_data->shader->shader_data = shader_data;
	draw_data->renderbuffer = framebuffer;

	window_set_userdata(window, &record);
	input_set_callbacks(window, callbacks);

	num_frames = 0;
	prev_time = platform_get_time();
	print_time = prev_time;
	while (!window_should_close(window)) {
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time;

		update_camera(window, camera, &record);
		ViewMatrix = camera_get_view_matrix(camera);
		Projection = camera_get_proj_matrix(camera);

		shader_data->view_Pos = camera_get_position(camera);
		shader_data->light_dir = light_dir.normalize();
		shader_data->model_matrix = ModelMatrix;
		shader_data->model_matrix_I = ModelMatrix.invert();
		shader_data->view_matrix = ViewMatrix;
		shader_data->projection_matrix = Projection;
		shader_data->light_vp_matrix = get_light_proj_matrix(aspect, 1, 0, 5) * camera_get_light_view_matrix(Vector3f(1, 1, 1), Vector3f(0, 0, 0), { 0, 1, 0 });
		shader_data->camera_vp_matrix = Projection * ViewMatrix;

		if (enable_shadow)
		{
			if (!shadow_draw_data)
			{
				shdaow_map = renderbuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
				shadow_draw_data = new DrawData();
				shadow_draw_data->model = model;
				shadow_draw_data->shader = &shadow_shader;
				shadow_draw_data->shader->shader_data = shader_data;
				shadow_draw_data->renderbuffer = shdaow_map;
			}

			graphics_draw_triangle(shadow_draw_data);
			shader_data->shadow_map = shdaow_map;
		}

		//RenderModel("african_head", framebuffer, tangentSpaceNormalmappingShader);

		graphics_draw_triangle(draw_data);

		window_draw_buffer(window, framebuffer);
		num_frames += 1;
		if (curr_time - print_time >= 1) {
			int sum_millis = (int)((curr_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;
			printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
			num_frames = 0;
			print_time = curr_time;
		}
		prev_time = curr_time;

		record.orbit_delta = Vector2f(0, 0);
		record.pan_delta = Vector2f(0, 0);
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;

		renderbuffer_clear_color(framebuffer, Color::Black);
		renderbuffer_clear_depth(framebuffer, std::numeric_limits<float>::max());

		if (enable_shadow)
		{
			renderbuffer_clear_color(shdaow_map, Color::Black);
			renderbuffer_clear_depth(shdaow_map, std::numeric_limits<float>::max());
		}

		input_poll_events();
	}

	if (model)
	{
		delete model;
	}

	if (shadow_draw_data)
	{
		delete shadow_draw_data;
	}

	delete shader_data;

	window_destroy(window);
	renderbuffer_release(framebuffer);
	if (shdaow_map) renderbuffer_release(shdaow_map);
	camera_release(camera);
}