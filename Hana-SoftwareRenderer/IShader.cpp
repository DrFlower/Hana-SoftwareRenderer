#include "IShader.h"
#include "our_gl.h"

//������������������������������������������������ GroundShader ������������������������������������������������

GroundShader::GroundShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f GroundShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToClipPos(a2v->obj_pos);
	v2f.intensity = saturate(ObjectToWorldNormal(a2v->obj_normal) * WorldLightDir());
	return v2f;
}

bool GroundShader::fragment(shader_struct_v2f* v2f, Color& color) {
	color = Color::White * v2f->intensity;
	return false;
}

//������������������������������������������������ GroundShader ������������������������������������������������



//������������������������������������������������ ToonShader ������������������������������������������������

ToonShader::ToonShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f ToonShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToClipPos(a2v->obj_pos);
	v2f.intensity = saturate(ObjectToWorldNormal(a2v->obj_normal) * WorldLightDir());
	return v2f;
}

bool ToonShader::fragment(shader_struct_v2f* v2f, Color& color) {
	float intensity = v2f->intensity;
	if (intensity > .85) intensity = 1;
	else if (intensity > .60) intensity = .80;
	else if (intensity > .45) intensity = .60;
	else if (intensity > .30) intensity = .45;
	else if (intensity > .15) intensity = .30;
	color = Color(1, 155 / 255.f, 0) * intensity;
	return false;
}

//������������������������������������������������ ToonShader ������������������������������������������������



//������������������������������������������������ TextureShader ������������������������������������������������

TextureShader::TextureShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f TextureShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToClipPos(a2v->obj_pos);
	v2f.uv = a2v->uv;
	return v2f;
}

bool TextureShader::fragment(shader_struct_v2f* v2f, Color& color) {
	color = tex_diffuse(v2f->uv);
	return false;
}

//������������������������������������������������ TextureShader ������������������������������������������������



//������������������������������������������������ TextureWithLightShader ������������������������������������������������

TextureWithLightShader::TextureWithLightShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f TextureWithLightShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToClipPos(a2v->obj_pos);
	v2f.world_normal = ObjectToWorldNormal(a2v->obj_normal);
	v2f.uv = a2v->uv;
	return v2f;
}

bool TextureWithLightShader::fragment(shader_struct_v2f* v2f, Color& color) {
	float intensity = saturate(v2f->world_normal * WorldLightDir());
	color = tex_diffuse(v2f->uv) * intensity;
	return false;
}

//������������������������������������������������ TextureWithLightShader ������������������������������������������������



//������������������������������������������������ BlinnShader ������������������������������������������������

BlinnShader::BlinnShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f BlinnShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToClipPos(a2v->obj_pos);
	v2f.world_pos = ObjectToWorldPos(a2v->obj_pos);
	v2f.world_normal = ObjectToWorldNormal(a2v->obj_normal);
	v2f.uv = a2v->uv;
	return v2f;
}

bool BlinnShader::fragment(shader_struct_v2f* v2f, Color& color) {
	MaterialProperty* mp = draw_data->matrial->material_property;

	Vector3f worldNormalDir = (v2f->world_normal).normalize();
	Color albedo = tex_diffuse(v2f->uv) * mp->color;
	Color ambient = AMBIENT * albedo;
	Color diffuse = LightColor * albedo * saturate(worldNormalDir * WorldLightDir());
	Vector3f viewDir = WorldSpaceViewDir(v2f->world_pos).normalize();
	Vector3f halfDir = (viewDir + WorldLightDir()).normalize();
	Color spcular = LightColor * mp->specular * std::pow(saturate(worldNormalDir * halfDir), mp->gloss);
	color = ambient + diffuse + spcular;
	return false;
}

//������������������������������������������������ BlinnShader ������������������������������������������������



//������������������������������������������������ NormalMapShader ������������������������������������������������

NormalMapShader::NormalMapShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f NormalMapShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToClipPos(a2v->obj_pos);
	v2f.uv = a2v->uv;
	v2f.world_pos = ObjectToWorldPos(a2v->obj_pos);
	v2f.world_normal = ObjectToWorldNormal(a2v->obj_normal);
	return v2f;
}

bool NormalMapShader::fragment(shader_struct_v2f* v2f, Color& color) {
	MaterialProperty* mp = draw_data->matrial->material_property;

	Vector3f normal = v2f->world_normal;

	float x = normal.x;
	float y = normal.y;
	float z = normal.z;

	Vector3f t = Vector3f(x * y / std::sqrt(x * x + z * z), std::sqrt(x * x + z * z), z * y / std::sqrt(x * x + z * z));
	Vector3f b = cross(normal, t);

	Matrix3x3 TBN;
	TBN[0] = Vector3f(t.x, b.x, normal.x);
	TBN[1] = Vector3f(t.y, b.y, normal.y);
	TBN[2] = Vector3f(t.z, b.z, normal.z);

	Vector3f bump = tex_normal(v2f->uv);
	bump.x = bump.x * mp->bump_scale;
	bump.y = bump.y * mp->bump_scale;
	bump.z = sqrt(1.0 - saturate(Vector2f(bump.x, bump.y) * Vector2f(bump.x, bump.y)));

	normal = (TBN * bump).normalize();

	Vector3f worldNormalDir = normal;
	Color albedo = tex_diffuse(v2f->uv) * mp->color;
	Color ambient = AMBIENT * albedo;
	Color diffuse = LightColor * albedo * saturate(worldNormalDir * WorldLightDir());
	Vector3f viewDir = WorldSpaceViewDir(v2f->world_pos).normalize();
	Vector3f halfDir = (viewDir + WorldLightDir()).normalize();
	Color spcular = LightColor * mp->specular * std::pow(saturate(worldNormalDir * halfDir), mp->gloss);
	color = ambient + diffuse + spcular;
	return false;
}

//������������������������������������������������ NormalMapShader ������������������������������������������������



//������������������������������������������������ ShadowShader ������������������������������������������������

ShadowShader::ShadowShader(DrawData* dd) :IShader(dd) {};

shader_struct_v2f ShadowShader::vertex(shader_struct_a2v* a2v) {
	shader_struct_v2f v2f;
	v2f.clip_pos = ObjectToViewPos(a2v->obj_pos);
	return v2f;
}

bool ShadowShader::fragment(shader_struct_v2f* v2f, Color& color) {
	//float factor = v2f->clip_pos[2];
	//float factor = v2f->clip_pos[2] / 2;
	//float factor = v2f->clip_pos[2] / v2f->clip_pos[3];
	//float factor = v2f->clip_pos[2] / v2f->clip_pos[3] -1;
	//float factor = (v2f->clip_pos[2] - 1) / 2;
	//float factor = (4 - v2f->clip_pos[2]) / 2;
	float factor = 1 - v2f->clip_pos[2];

	//float factor =  v2f->clip_pos[2] / v2f->clip_pos[3];
	//factor = 1 - v2f->clip_pos[2];

	color = Color::White * factor;

	//std::cout << factor << std::endl;
	return false;
}

//������������������������������������������������ ShadowShader ������������������������������������������������