#ifndef __MODEL_H__
#define __MODEL_H__
#include <vector>
#include <string>
#include "tgaimage.h"
#include "api.h"

class Model {
private:
    std::vector<Vector3f> verts_;
    std::vector<std::vector<Vector3i> > faces_; // attention, this Vec3i means vertex/uv/normal
    std::vector<Vector3f> norms_;
    std::vector<Vector2f> uv_;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    void load_texture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(const char *filename);
    ~Model();
    int nverts();
    int nfaces();
    Vector3f normal(int iface, int nthvert);
    Vector3f normal(Vector2f uv);
    Vector3f vert(int i);
    Vector3f vert(int iface, int nthvert);
    Vector2f uv(int iface, int nthvert);
    TGAColor diffuse(Vector2f uv);
    float specular(Vector2f uv);
    std::vector<int> face(int idx);
    TGAImage* get_diffuse_map();
    TGAImage* get_normal_map();
    TGAImage* get_specular_map();
};
#endif //__MODEL_H__

