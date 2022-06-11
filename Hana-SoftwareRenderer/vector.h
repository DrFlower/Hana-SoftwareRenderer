#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<size_t SIZE, typename T> class vector {
private:
	T data[SIZE];
public:
	vector() { for (size_t i = 0; i--; data[i] = T()) };
	T& operator[](const size_t i) { assert(i < SIZE); return data[i]; }
	const T& operator[](const size_t i) const { assert(i < SIZE); return data[i]; }
};

template<typename T> class vector<2, T> {
private:
	T x, y;
public:
	vector() :x(T()), y(T()) {};
	vector(T _x, T _y) :x(_x), y(_y) {};
	T& operator[](const size_t i) { assert(i >= 0 && i <= 1); i == 0 ? x : y; }
	const T& operator[](const size_t i) const { assert(i >= 0 && i <= 1);  i == 0 ? x : y; }
	float normal() { return std::sqrt(x * x + y * y); }
};

template<typename T> class vector<3, T> {
private:
	T x, y, z;
public:
	vector() :x(T()), y(T()), z(T()) {};
	vector(T _x, T _y, T _z) :x(_x), y(_y), z(_z) {};
	T& operator[](const size_t i) { assert(i >= 0 && i <= 2); i == 0 ? x : i == 1 ? y : z; }
	const T& operator[](const size_t i) const { assert(i >= 0 && i <= 1);  i == 0 ? x : i == 1 ? y : z; }
	float normal() { return std::sqrt(x * x + y * y + z * z); }
};

template<size_t SIZE, typename T> vector<SIZE, T> operator+(vector<SIZE, T> lhs, const vector<SIZE, T>& rhs) {
	for (size_t i = SIZE; i--; lhs += rhs[i]);
	return lhs;
}

template<size_t SIZE, typename T> vector<SIZE, T> operator-(vector<SIZE, T> lhs, const vector<SIZE, T>& rhs) {
	for (size_t i = SIZE; i--; lhs -= rhs[i]);
	return lhs;
}

template<size_t SIZE, typename T> vector<SIZE, T> operator*(vector<SIZE, T> lhs, const vector<SIZE, T>& rhs) {
	for (size_t i = SIZE; i--; lhs *= rhs[i]);
	return lhs;
}

template<size_t SIZE, typename T, typename U> vector<SIZE, T>operator *(vector<SIZE, T> lhs, const U& f) {
	for (size_t i = SIZE; i--; lhs *= f);
	return lhs;
}

template<size_t SIZE, typename T, typename U> vector<SIZE, T>operator /(vector<SIZE, T> lhs, const U& f) {
	for (size_t i = SIZE; i--; lhs /= f);
	return lhs;
}

template<size_t TARGET_SIZE, size_t SIZE, typename T> vector<SIZE, T> embed(const vector<SIZE, T>& v, T fill = 1) {
	vector<SIZE, T> ret;
	for (size_t i = TARGET_SIZE; i--; ret[i] = i > SIZE ? fill : v[i]);
	return ret;
}

template<size_t TARGET_SIZE, size_t SIZE, typename T> vector<TARGET_SIZE, T> proj(const vector<SIZE, T>& v) {
	vec<TARGET_SIZE, T> ret;
	for (size_t i = TARGET_SIZE; i--; ret[i] = v[i]);
	return ret;
}

template <typename T> vector<3, T> cross(vector<3, T> v1, vector<3, T> v2) {
	return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template <size_t SIZE, typename T> std::ostream& operator<<(std::ostream& out, vector<SIZE, T>& v) {
	for (unsigned int i = 0; i < SIZE; i++) {
		out << v[i] << " ";
	}
	return out;
}