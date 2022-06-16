//#include "IShader.h"
//#include "our_gl.h"
//
//struct TextureShader2 : IShader2
//{
//	// Í¨¹ý IShader ¼Ì³Ð
//	virtual shader_struct_v2f vertex(const shader_struct_a2v& a2v) override {
//		shader_struct_v2f v2f;
//		v2f.clip_pos = Projection * ModelView * embed<4>(a2v.obj_pos);
//		v2f.world_pos = proj<3>(ModelMatrix * embed<4>(a2v.obj_pos));
//		Matrix<1, 4, float> m_objPos;
//		m_objPos[0] = embed<4>(a2v.obj_pos);
//		v2f.world_normal = proj<3>((m_objPos * ModelMatrix.invert())[0]);
//		v2f.uv = a2v.uv;
//		return v2f;
//	}
//
//	virtual bool fragment(const shader_struct_v2f& v2f, Color& color) override {
//		float intensity = std::max(0.f, v2f.world_normal * light_dir);
//		color = tex_diffuse(material_property.diffuse_map, v2f.uv) * intensity;
//		return false;
//	}
//};