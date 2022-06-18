#pragma once
#include <cassert>
#include "tgaimage.h"

class Color
{
public:
	unsigned char r, g, b, a;
	Color();
	Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A = 255);
	Color(const TGAColor& tga_color);

	static Color White;

	unsigned char& operator[](const size_t i);

	unsigned char operator[](const size_t i) const;

	Color operator*(float intensity) const;

	Color operator*(const Color& color) const;
};