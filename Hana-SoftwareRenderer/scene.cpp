#include "scene.h"

Scene::Scene(RenderBuffer* renderBuffer) {
	this->frameBuffer = renderBuffer;
	float aspect = (float)this->frameBuffer->width / (float)this->frameBuffer->height;
	camera = new Camera(CAMERA_POSITION, CAMERA_TARGET, aspect);

	light_dir.normalize();
}

void Scene::tick(float delta_time) {

}

void Scene::on_key_input(keycode_t key, int pressed) {

}

Scene::~Scene() {
	delete camera;
}

SingleModelScene::SingleModelScene(const char* modelName, RenderBuffer* renderBuffer) :Scene(renderBuffer) {
	gameobject = new GameObject_StaticModel(modelName);

	material = new Matrial();
	material->diffuse_map = gameobject->model->get_diffuse_map();
	material->normal_map = gameobject->model->get_normal_map();
	material->specular_map = gameobject->model->get_specular_map();
	material->color = Color::White;
	material->specular = Color::White;
	material->gloss = 50;
	material->bump_scale = 1;

	//IShader* _shaders[] = { new NormalMapShader(), new BlinnShader(), new TextureWithLightShader(), new TextureShader(), new ToonShader(), new GroundShader() };
	//shaders = _shaders;
	shaders[0] = new NormalMapShader();
	shaders[1] = new BlinnShader();
	shaders[2] = new TextureWithLightShader();
	shaders[3] = new TextureShader();
	shaders[4] = new ToonShader();
	shaders[5] = new GroundShader();

	enable_shadow = false;

	cur_shader_index = 0;

	draw_model = new DrawModel(gameobject, material, shaders[cur_shader_index]);
}

SingleModelScene::~SingleModelScene() {
	delete draw_model;
	delete gameobject;
	delete material;
}

void SingleModelScene::tick(float delta_time) {
	Scene::tick(delta_time);

	draw_model->draw(camera, frameBuffer, enable_shadow);
}

void SingleModelScene::on_key_input(keycode_t key, int pressed) {
	if (pressed)
	{
		switch (key)
		{
		case KEY_Q:
			delete draw_model;
			cur_shader_index = (cur_shader_index + 1 + 6) % 6;
			draw_model = new DrawModel(gameobject, material, shaders[cur_shader_index]);
			break;
		case KEY_E:
			enable_shadow = !enable_shadow;
			break;
		default:
			break;
		}
	}
}

MultiModelScene::MultiModelScene(RenderBuffer* renderBuffer) :Scene(renderBuffer) {

	GameObject_StaticModel* go_1 = new GameObject_StaticModel("african_head.obj", Vector3f(0, 0, -2), Vector3f::Zero, Vector3f::One);
	GameObject_StaticModel* go_2 = new GameObject_StaticModel("floor.obj", Vector3f(0, -10, -2), Vector3f(-90, 0, 0), Vector3f(0.5f, 0.5f, 0.5f));

	gameobject = new GameObject_StaticModel[2]{ *go_1 ,*go_2 };

	Matrial* m_1 = new Matrial();
	m_1->diffuse_map = go_1->model->get_diffuse_map();
	m_1->normal_map = go_1->model->get_normal_map();
	m_1->specular_map = go_1->model->get_specular_map();
	m_1->color = Color::White;
	m_1->specular = Color::White;
	m_1->gloss = 50;
	m_1->bump_scale = 1;

	Matrial* m_2 = new Matrial();
	m_2->diffuse_map = go_2->model->get_diffuse_map();
	m_2->normal_map = go_2->model->get_normal_map();
	m_2->specular_map = go_2->model->get_specular_map();
	m_2->color = Color::White;
	m_2->specular = Color::White;
	m_2->gloss = 50;
	m_2->bump_scale = 1;

	material = new Matrial[2]{ *m_1, *m_2 };

	//GroundShader ground_shader = GroundShader();
	//ToonShader toon_shader = ToonShader();
	//TextureShader texture_shader = TextureShader();
	//TextureWithLightShader text_with_light_shader = TextureWithLightShader();
	//BlinnShader blinn_shader = BlinnShader();

	BlinnShader* shader_1 = new BlinnShader();
	BlinnShader* shader_2 = new BlinnShader();
	shader = new BlinnShader[2]{ *shader_1 ,*shader_2 };


	enable_shadow = false;

	draw_model = new DrawModel[2]{ {go_1, m_1, shader_1},{go_2, m_2, shader_2} };
}

MultiModelScene::~MultiModelScene() {
	delete[] draw_model;
	delete[] gameobject;
	delete[] material;
	delete shader;
}

void MultiModelScene::AddModel(int index, const char* filename, Matrial* material, IShader* shader, Vector3f position, Vector3f rotation, Vector3f scale) {
	//gameobject[index] = *new GameObject_StaticModel("african_head.obj", position, rotation, scale);

}

void MultiModelScene::tick(float delta_time) {
	Scene::tick(delta_time);

	for (int i = 0; i < 2; i++)
	{
		draw_model[i].draw(camera, frameBuffer, enable_shadow);
	}


}