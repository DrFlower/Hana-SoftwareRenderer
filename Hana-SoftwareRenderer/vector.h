#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<size_t SIZE, typename T> class Vector {
private:
	T data[SIZE];
public:
	Vector() { for (size_t i = 0; i--; data[i] = T()) };
	T& operator[](const size_t i) { assert(i < SIZE); return data[i]; }
	const T& operator[](const size_t i) const { assert(i < SIZE); return data[i]; }
};

template<typename T> class Vector<2, T> {
private:
	T x, y;
public:
	Vector() :x(T()), y(T()) {};
	Vector(T _x, T _y) :x(_x), y(_y) {};
	T& operator[](const size_t i) { assert(i >= 0 && i <= 1); i == 0 ? x : y; }
	const T& operator[](const size_t i) const { assert(i >= 0 && i <= 1);  i == 0 ? x : y; }
	float normal() { return std::sqrt(x * x + y * y); }
};

template<typename T> class Vector<3, T> {
private:
	T x, y, z;
public:
	Vector() :x(T()), y(T()), z(T()) {};
	Vector(T _x, T _y, T _z) :x(_x), y(_y), z(_z) {};
	T& operator[](const size_t i) { assert(i >= 0 && i <= 2); i == 0 ? x : i == 1 ? y : z; }
	const T& operator[](const size_t i) const { assert(i >= 0 && i <= 1);  i == 0 ? x : i == 1 ? y : z; }
	float normal() { return std::sqrt(x * x + y * y + z * z); }
};

template<size_t SIZE, typename T> Vector<SIZE, T> operator+(Vector<SIZE, T> lhs, const Vector<SIZE, T>& rhs) {
	for (size_t i = SIZE; i--; lhs += rhs[i]);
	return lhs;
}

template<size_t SIZE, typename T> Vector<SIZE, T> operator-(Vector<SIZE, T> lhs, const Vector<SIZE, T>& rhs) {
	for (size_t i = SIZE; i--; lhs -= rhs[i]);
	return lhs;
}

template<size_t SIZE, typename T> Vector<SIZE, T> operator*(Vector<SIZE, T> lhs, const Vector<SIZE, T>& rhs) {
	for (size_t i = SIZE; i--; lhs *= rhs[i]);
	return lhs;
}

template<size_t SIZE, typename T, typename U> Vector<SIZE, T>operator *(Vector<SIZE, T> lhs, const U& f) {
	for (size_t i = SIZE; i--; lhs *= f);
	return lhs;
}

template<size_t SIZE, typename T, typename U> Vector<SIZE, T>operator /(Vector<SIZE, T> lhs, const U& f) {
	for (size_t i = SIZE; i--; lhs /= f);
	return lhs;
}

template<size_t TARGET_SIZE, size_t SIZE, typename T> Vector<SIZE, T> embed(const Vector<SIZE, T>& v, T fill = 1) {
	Vector<SIZE, T> ret;
	for (size_t i = TARGET_SIZE; i--; ret[i] = i > SIZE ? fill : v[i]);
	return ret;
}

template<size_t TARGET_SIZE, size_t SIZE, typename T> Vector<TARGET_SIZE, T> proj(const Vector<SIZE, T>& v) {
	vec<TARGET_SIZE, T> ret;
	for (size_t i = TARGET_SIZE; i--; ret[i] = v[i]);
	return ret;
}

template <typename T> Vector<3, T> cross(Vector<3, T> v1, Vector<3, T> v2) {
	return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template <size_t SIZE, typename T> std::ostream& operator<<(std::ostream& out, Vector<SIZE, T>& v) {
	for (unsigned int i = 0; i < SIZE; i++) {
		out << v[i] << " ";
	}
	return out;
}

typedef Vector<2, float> Vector2f;
typedef Vector<2, int>   Vector2i;
typedef Vector<3, float> Vector3f;
typedef Vector<3, int>   Vector3i;
typedef Vector<4, float> Vector4f;