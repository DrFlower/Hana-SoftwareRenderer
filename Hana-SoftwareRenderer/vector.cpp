//#include <cassert>
//
//template<typename T>
//class vector2
//{
//	T x, y;
//	vector2() :x(T()), y(T()) {};
//	vector2(T x, T y) :x(x), y(y) {};
//	T& operator[](const int index)
//	{
//		assert(index >= 0 && index < 2);
//		if (index == 0) { return x; }
//		else { return y; }
//	}
//
//	vector2 operator+(const vector2& other) const { return vector2(x + other.x, y + other.y); }
//	vector2 operator-(const vector2& other) const { return vector2(x - other.x, y - other.y); }
//	vector2 operator*(const vector2& other) const { return vector2(x * other.x, y * other.y); }
//	vector2 operator*(float f) const { return vector2(x * f, y * f); }
//};
//
//template<typename T>
//class vector3
//{
//	T x, y;
//	vector3() :x(T()), y(T()) {};
//	vector3(T x, T y) :x(x), y(y) {};
//	T& operator[](const int index)
//	{
//		assert(index >= 0 && index < 2);
//		if (index == 0) { return x; }
//		else { return y; }
//	}
//
//	vector3 operator+(const vector3& other) const { return vector2(x + other.x, y + other.y); }
//	vector3 operator-(const vector3& other) const { return vector2(x - other.x, y - other.y); }
//	vector3 operator*(const vector3& other) const { return vector2(x * other.x, y * other.y); }
//	vector3 operator*(float f) const { return vector2(x * f, y * f); }
//};