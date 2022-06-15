#include "Color.h"

Color::Color() :r(), g(), b(), a() {};
Color::Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) :r(R), g(G), b(B), a(A) {}
Color::Color(const TGAColor& tga_color) :r(tga_color.bgra[2]), g(tga_color.bgra[1]), b(tga_color.bgra[0]), a(tga_color.bgra[3]) {}

unsigned char& Color::operator[](const size_t i) {
	assert(i >= 0 && i <= 3);
	if (i == 0)
		return r;
	else if (i == 1)
		return g;
	else if (i == 2)
		return b;
	else
		return a;
}

unsigned char Color::operator[](const size_t i) const {
	assert(i >= 0 && i <= 3);
	if (i == 0)
		return r;
	else if (i == 1)
		return g;
	else if (i == 2)
		return b;
	else
		return a;
}

Color Color::operator*(float intensity) const {
	Color ret = *this;
	intensity = (intensity > 1.f ? 1.f : (intensity < 0.f ? 0.f : intensity));
	for (size_t i = 0; i < 4; i++)
	{
		ret[i] = ret[i] * intensity;
	}
	return ret;
}

Color Color::operator*(const Color& color) const {
	Color ret = *this;
	for (size_t i = 0; i < 4; i++)
	{
		ret[i] = ret[i] * color[i];
	}
	return ret;
}