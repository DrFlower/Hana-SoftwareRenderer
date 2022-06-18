#include "Color.h"

Color::Color() :r(), g(), b(), a() {};
Color::Color(float R, float G, float B, float A) :r(R), g(G), b(B), a(A) {}
Color::Color(const TGAColor& tga_color) :r(tga_color.bgra[2] / 255.f), g(tga_color.bgra[1] / 255.f), b(tga_color.bgra[0] / 255.f), a(tga_color.bgra[3] / 255.f) {}

float Color::MAX_CHANNEL_VALUE = 1.f;
Color Color::White(MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE);
Color Color::Red(MAX_CHANNEL_VALUE, 0, 0);
Color Color::Green(0, MAX_CHANNEL_VALUE, 0);
Color Color::Blue(0, 0, MAX_CHANNEL_VALUE);
Color Color::Black(0, 0, 0);

float& Color::operator[](const size_t i) {
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

float Color::operator[](const size_t i) const {
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

Color Color::operator+(const Color& color) const {
	Color ret = *this;
	for (size_t i = 0; i < 4; i++)
	{
		ret[i] = std::min(std::max(0.f, ret[i] + color[i]), MAX_CHANNEL_VALUE);
	}
	return ret;
}

Color Color::operator*(float intensity) const {
	Color ret = *this;
	intensity = (intensity > 1.f ? 1.f : (intensity < 0.f ? 0.f : intensity));
	for (size_t i = 0; i < 4; i++)
	{
		ret[i] = std::min(std::max(0.f, ret[i] * intensity), MAX_CHANNEL_VALUE);
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