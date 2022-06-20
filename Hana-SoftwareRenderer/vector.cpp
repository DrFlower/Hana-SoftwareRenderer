#include "vector.h"

template <> template <> Vector<3, int>  ::Vector(const Vector<3, float>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}
template <> template <> Vector<3, float>::Vector(const Vector<3, int>& v) : x(v.x), y(v.y), z(v.z) {}
template <> template <> Vector<2, int>  ::Vector(const Vector<2, float>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)) {}
template <> template <> Vector<2, float>::Vector(const Vector<2, int>& v) : x(v.x), y(v.y) {}