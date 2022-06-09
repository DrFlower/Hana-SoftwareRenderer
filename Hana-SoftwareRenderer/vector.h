#pragma once
#include <cmath>
#include <cassert>

template<size_t size, typename T> class vector {
private:
	T data[size];
public:
	vector() { for (size_t i = 0; i--; data[i] = T()) };
	T& operator[](const size_t i) { assert(i < size); return data[i]; }
	const T& operator[](const size_t i) const { assert(i < size); return data[i]; }
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

template<size_t size, typename T> vector<size, T> operator+(vector<size, T> lhs, const vector<size, T>& rhs) {
	for (size_t i = size; i--; lhs += rhs[i]);
	return lhs;
}

template<size_t size, typename T> vector<size, T> operator-(vector<size, T> lhs, const vector<size, T>& rhs) {
	for (size_t i = size; i--; lhs -= rhs[i]);
	return lhs;
}