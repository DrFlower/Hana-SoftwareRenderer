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

static const char* const WINDOW_TITLE = "Hana-SoftwareRenderer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

Model* model = NULL;


//Vector3f       eye(1, 1, 3);
//Vector3f    center(0, 0, 0);
//Vector3f        up(0, 1, 0);

struct GouraudShader : public IShader {
	Vector3f varying_intensity; // written by vertex shader, read by fragment shader

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		Vector4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = Projection * ModelView * gl_Vertex;     // transform it to screen coordinates
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

struct ToonShader : public IShader {
	Vector3f varying_intensity; // written by vertex shader, read by fragment shader

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		Vector4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = Projection * ModelView * gl_Vertex;     // transform it to screen coordinates
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

struct TextureShader : public IShader {
	Vector3f          varying_intensity; // written by vertex shader, read by fragment shader
	Matrix<2, 3, float> varying_uv;        // same as above

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity
		Vector3f v = model->vert(iface, nthvert);
		Vector4f gl_Vertex = embed<4>(v); // read the vertex from .obj file
		gl_Vertex = Projection * ModelView * gl_Vertex; // transform it to screen coordinates
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

struct NormalmappingShader :public IShader
{
	Matrix<2, 3, float> varying_uv;  // same as above
	Matrix<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()

	virtual Vector4f vertex(Model* model, int iface, int nthvert) override
	{
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		Vector4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = Projection * ModelView * gl_Vertex; // transform it to screen coordinates
		varying_tri.setCol(nthvert, gl_Vertex);
		uniform_MIT = (Projection * ModelView).invert_transpose();
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

struct TangentSpaceNormalmappingShader :public IShader
{
	Matrix<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	Matrix<4, 4, float> uniform_M;   //  Projection*ModelView
	Matrix<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	Matrix<3, 3, float> ndc_tri;     // triangle in normalized device coordinates

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		varying_nrm.setCol(nthvert, proj<3>((Projection * ModelView).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));
		Vector4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));
		varying_tri.setCol(nthvert, gl_Vertex);
		uniform_M = Projection * ModelView;
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

struct TestShader : public IShader
{
	Matrix<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	//Matrix<4, 4, float> uniform_M;   //  Projection*ModelView
	//Matrix<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	//Matrix<3, 3, float> ndc_tri;     // triangle in normalized device coordinates
	Matrix<3, 4, float> Ttow;
	virtual Vector4f vertex(Model* model, int iface, int nthvert) override
	{
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		//varying_nrm.setCol(nthvert, proj<3>((Projection * ModelView).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));
		Vector4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));
		varying_tri.setCol(nthvert, gl_Vertex);
		//uniform_M = Projection * ModelView;
		//ndc_tri.setCol(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));

		Vector3f worldPos = model->vert(iface, nthvert);
		Vector3f worldNormal = model->normal(iface, nthvert);


		return gl_Vertex;
	}
	virtual bool fragment(Model* model, Vector3f bar, TGAColor& color) override
	{
		//Vector3f bn = (varying_nrm * bar).normalize();
		//Vector2f uv = varying_uv * bar;

		//Matrix<3, 3, float> A;
		//A[0] = ndc_tri.getCol(1) - ndc_tri.getCol(0);
		//A[1] = ndc_tri.getCol(2) - ndc_tri.getCol(0);
		//A[2] = bn;

		//Matrix<3, 3, float> AI = A.invert();

		//Vector3f i = AI * Vector3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		//Vector3f j = AI * Vector3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		//Matrix<3, 3, float> B;
		//B.setCol(0, i.normalize());
		//B.setCol(1, j.normalize());
		//B.setCol(2, bn);

		//Vector3f n = (B * model->normal(uv)).normalize();
		//Vector3f l = proj<3>(Projection * ModelView * embed<4>(light_dir, 0.f)).normalize();
		//float diff = std::max(0.f, n * l);
		//color = model->diffuse(uv) * diff;

		Vector2f uv = varying_uv * bar;
		color = model->diffuse(uv);
		return false;
	}
};

struct SpecularShader : public IShader
{
	Matrix<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	Matrix<4, 4, float> uniform_M;   //  Projection*ModelView
	Matrix<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	Matrix<3, 3, float> ndc_tri;     // triangle in normalized device coordinates

	virtual Vector4f vertex(Model* model, int iface, int nthvert) {
		varying_uv.setCol(nthvert, model->uv(iface, nthvert));
		varying_nrm.setCol(nthvert, proj<3>((Projection * ModelView).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));
		Vector4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));
		varying_tri.setCol(nthvert, gl_Vertex);
		uniform_M = Projection * ModelView;
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
		//5Ϊ����������0.6Ϊ���淴��ϵ��
		for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + c[i] * (diff + .6 * spec), 255);
		return false;
	}
};

void RenderModel(std::string modelName, framebuffer* framebuffer, IShader& shader)
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

struct TextureShader2 : public IShader2
{
	TextureShader2(MaterialProperty* mp) :IShader2(mp) {};

	// ͨ�� IShader �̳�
	virtual shader_struct_v2f vertex(const shader_struct_a2v& a2v) override {
		shader_struct_v2f v2f;
		v2f.clip_pos = Projection * ModelView * embed<4>(a2v.obj_pos);
		//v2f.world_pos = proj<3>(ModelMatrix * embed<4>(a2v.obj_pos));
		//Matrix<1, 4, float> m_objPos;
		//m_objPos[0] = embed<4>(a2v.obj_pos);
		//v2f.world_normal = proj<3>((m_objPos * ModelMatrix.invert())[0]);
		//v2f.uv = a2v.uv;
		return v2f;
	}

	virtual bool fragment(const shader_struct_v2f& v2f, Color& color) override {
		//float intensity = std::max(0.f, v2f.world_normal * light_dir);
		//float intensity = 1;
		//color = tex_diffuse(material_property->diffuse_map, v2f.uv) * intensity;
		color = Color(255, 255, 255);
		return false;
	}
};

int main()
{
	platform_initialize();
	window_t* window;
	framebuffer* framebuffer;
	camera_t* camera;
	record_t record;
	callbacks_t callbacks;
	float aspect;
	float prev_time;
	float print_time;
	int num_frames;

	//lookat(eye, center, up);
	viewport(WINDOW_WIDTH / 8, WINDOW_HEIGHT / 8, WINDOW_WIDTH * 3 / 4, WINDOW_HEIGHT * 3 / 4);
	//projection(-1.f / (eye - center).normal());


	light_dir.normalize();

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
	aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	camera = camera_create(CAMERA_POSITION, CAMERA_TARGET, aspect);

	record = record_t();
	//memset(&record, 0, sizeof(record_t));

	memset(&callbacks, 0, sizeof(callbacks_t));
	callbacks.button_callback = button_callback;
	callbacks.scroll_callback = scroll_callback;

	GouraudShader gouraudShader;
	ToonShader toonShader;
	TextureShader textureShader;
	NormalmappingShader normalmapping;
	TangentSpaceNormalmappingShader tangentSpaceNormalmappingShader;
	SpecularShader specularShader;
	TestShader testShader;


	model = new Model("D:\\Development\\Github\\Hana-SoftwareRenderer\\Hana-SoftwareRenderer\\obj\\african_head.obj");

	MaterialProperty mp;
	mp.diffuse_map = model->get_diffuse_map();
	mp.normal_map = model->get_normal_map();
	mp.specular_map = model->get_specular_map();
	mp.color = Color(1, 1, 1, 1);
	mp.specular = Color(1, 1, 1, 1);
	mp.gloss = 1;
	mp.bump_scale = 1;

	TextureShader2 shader = TextureShader2(&mp);

	Matrial* material = new Matrial(&shader, &mp);
	AppData* appdata = new AppData(model, material);

	window_set_userdata(window, &record);
	input_set_callbacks(window, callbacks);

	num_frames = 0;
	prev_time = platform_get_time();
	print_time = prev_time;
	while (!window_should_close(window)) {
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time;

		update_camera(window, camera, &record);
		ModelView = camera_get_view_matrix(camera);

		Projection = Matrix4x4::identity();

		Matrix4x4 m = Matrix4x4::identity();
		m[0][0] = -1;
		m[1][1] = -1;
		m[2][2] = -1;
		m[3][3] = -1;

		Projection = camera_get_proj_matrix(camera) * m;

		//RenderModel("african_head", framebuffer, gouraudShader);

		graphics_draw_triangle(framebuffer, appdata);

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

		framebuffer_clear_color(framebuffer, vec4_t());
		framebuffer_clear_depth(framebuffer, -std::numeric_limits<float>::max());

		input_poll_events();
	}

	if (model)
	{
		delete model;
	}

	delete material;
	delete appdata;

	window_destroy(window);
	framebuffer_release(framebuffer);
	camera_release(camera);
}