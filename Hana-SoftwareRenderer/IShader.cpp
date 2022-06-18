#include "IShader.h"
#include "our_gl.h"

//¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý GroundShader ¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý

GroundShader::GroundShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f GroundShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.intensity = std::max(0.f, proj<3>(ModelMatrix * embed<4>(a2v->obj_normal)) * light_dir.normalize());
	return v2f;
}

bool GroundShader::fragment(shader_struct_v2f* v2f, Color& color) {
	color = Color::White * v2f->intensity;
	return false;
}

//¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü GroundShader ¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü



//¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý ToonShader ¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý

ToonShader::ToonShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f ToonShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.intensity = std::max(0.f, proj<3>(ModelMatrix * embed<4>(a2v->obj_normal)) * light_dir.normalize());
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

//¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü ToonShader ¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü



//¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý TextureShader ¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý

TextureShader::TextureShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f TextureShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.uv = a2v->uv;
	return v2f;
}

bool TextureShader::fragment(shader_struct_v2f* v2f, Color& color) {
	color = tex_diffuse(draw_data->matrial->material_property->diffuse_map, v2f->uv);
	return false;
}

//¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü TextureShader ¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü



//¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý TextureWithLightShader ¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý

TextureWithLightShader::TextureWithLightShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f TextureWithLightShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.world_normal = a2v->obj_normal;
	v2f.uv = a2v->uv;
	return v2f;
}

bool TextureWithLightShader::fragment(shader_struct_v2f* v2f, Color& color) {
	float intensity = std::max(0.f, v2f->world_normal * light_dir.normalize());
	color = tex_diffuse(draw_data->matrial->material_property->diffuse_map, v2f->uv) * intensity;
	return false;
}

//¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü TextureWithLightShader ¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü



//¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý BlinnShader ¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý¡ý

BlinnShader::BlinnShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f BlinnShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = Projection * ModelView * embed<4>(a2v->obj_pos);
	v2f.world_pos = proj<3>(ModelMatrix * embed<4>(a2v->obj_pos));
	Matrix<1, 4, float> m_normal;
	m_normal[0] = embed<4>(a2v->obj_normal);
	v2f.world_normal = proj<3>((m_normal * ModelMatrix.invert())[0]);
	v2f.uv = a2v->uv;
	return v2f;
}

bool BlinnShader::fragment(shader_struct_v2f* v2f, Color& color) {
	MaterialProperty* mp = draw_data->matrial->material_property;

	Vector3f worldNormalDir = (v2f->world_normal).normalize();
	Color albedo = tex_diffuse(mp->diffuse_map, v2f->uv) * mp->color;
	Color ambient = AMBIENT * albedo;
	Color diffuse = LightColor * albedo * saturate(worldNormalDir * light_dir.normalize());
	Vector3f viewDir = (get_camera_pos(draw_data->camera) - v2f->world_pos).normalize();
	Vector3f halfDir = (viewDir + light_dir.normalize()).normalize();
	Color spcular = LightColor * mp->specular * std::pow(saturate(worldNormalDir * halfDir), mp->gloss);
	color = ambient + diffuse + spcular;
	return false;
}

//¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü BlinnShader ¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü¡ü