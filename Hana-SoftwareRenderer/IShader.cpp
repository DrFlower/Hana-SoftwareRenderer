#include "IShader.h"
#include "our_gl.h"

//������������������������������������������������ GroundShader ������������������������������������������������

GroundShader::GroundShader(MaterialProperty* mp) :IShader(mp) {};

shader_struct_v2f GroundShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.intensity = std::max(0.f, proj<3>(ModelMatrix * embed<4>(a2v->obj_normal)) * light_dir);
	return v2f;
}

bool GroundShader::fragment(shader_struct_v2f* v2f, Color& color) {
	color = Color::White * v2f->intensity;
	return false;
}

//������������������������������������������������ GroundShader ������������������������������������������������



//������������������������������������������������ ToonShader ������������������������������������������������

ToonShader::ToonShader(MaterialProperty* mp) :IShader(mp) {};

shader_struct_v2f ToonShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.intensity = std::max(0.f, proj<3>(ModelMatrix * embed<4>(a2v->obj_normal)) * light_dir);
	return v2f;
}

bool ToonShader::fragment(shader_struct_v2f* v2f, Color& color) {
	float intensity = v2f->intensity;
	if (intensity > .85) intensity = 1;
	else if (intensity > .60) intensity = .80;
	else if (intensity > .45) intensity = .60;
	else if (intensity > .30) intensity = .45;
	else if (intensity > .15) intensity = .30;
	color = TGAColor(255, 155, 0) * intensity;
	return false;
}

//������������������������������������������������ ToonShader ������������������������������������������������



//������������������������������������������������ TextureShader ������������������������������������������������

TextureShader::TextureShader(MaterialProperty* mp) :IShader(mp) {};

shader_struct_v2f TextureShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.uv = a2v->uv;
	return v2f;
}

bool TextureShader::fragment(shader_struct_v2f* v2f, Color& color) {
	color = tex_diffuse(material_property->diffuse_map, v2f->uv);
	return false;
}

//������������������������������������������������ TextureShader ������������������������������������������������

//������������������������������������������������ TextureWithLightShader ������������������������������������������������

TextureWithLightShader::TextureWithLightShader(MaterialProperty* mp) :IShader(mp) {};

shader_struct_v2f TextureWithLightShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.world_normal = a2v->obj_normal;
	v2f.uv = a2v->uv;
	return v2f;
}

bool TextureWithLightShader::fragment(shader_struct_v2f* v2f, Color& color) {
	float intensity = std::max(0.f, v2f->world_normal * light_dir);
	color = tex_diffuse(material_property->diffuse_map, v2f->uv) * intensity;
	return false;
}

//������������������������������������������������ TextureWithLightShader ������������������������������������������������

//������������������������������������������������ TextureWithLightShader ������������������������������������������������

//TextureWithLightShader::TextureWithLightShader(MaterialProperty* mp) :IShader(mp) {};
//
//shader_struct_v2f TextureWithLightShader::vertex(shader_struct_a2v* a2v) {
//	shader_struct_v2f v2f;
//	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
//	//v2f.world_pos = proj<3>(ModelMatrix * embed<4>(a2v.obj_pos));
//	//Matrix<1, 4, float> m_objPos;
//	//m_objPos[0] = embed<4>(a2v.obj_pos);
//	//v2f.world_normal = proj<3>((m_objPos * ModelMatrix.invert())[0]);
//	v2f.world_normal = a2v->obj_normal;
//	v2f.uv = a2v->uv;
//	return v2f;
//}
//
//bool TextureWithLightShader::fragment(shader_struct_v2f* v2f, Color& color) {
//	//float intensity = std::max(0.f, v2f.world_normal * light_dir);
//	//float intensity = std::max(0.f, tex_normal(material_property->normal_map, v2f.uv) * light_dir);
//	float intensity = std::max(0.f, v2f->world_normal * light_dir);
//	color = tex_diffuse(material_property->diffuse_map, v2f->uv) * intensity;
//	return false;
//}
//
////������������������������������������������������ TextureWithLightShader ������������������������������������������������