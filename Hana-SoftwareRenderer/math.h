#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<size_t SIZE, typename T> class Vector {
private:
	T data[SIZE];
public:
	Vector() { for (size_t i = 0; i--; data[i] = T()); }
	T& operator[](const size_t i) { assert(i < SIZE); return data[i]; }
	const T& operator[](const size_t i) const { assert(i < SIZE); return data[i]; }
};

template<typename T> class Vector<2, T> {
private:
	T x, y;
public:
	Vector() :x(T()), y(T()) {};
	Vector(T _x, T _y) :x(_x), y(_y) {};
	T& operator[](const size_t i) { assert(i >= 0 && i <= 1); return i == 0 ? x : y; }
	const T& operator[](const size_t i) const { assert(i >= 0 && i <= 1); return i == 0 ? x : y; }
	float normal() { return std::sqrt(x * x + y * y); }
};

template<typename T> class Vector<3, T> {
private:
	T x, y, z;
public:
	Vector() :x(T()), y(T()), z(T()) {};
	Vector(T _x, T _y, T _z) :x(_x), y(_y), z(_z) {};
	T& operator[](const size_t i) { assert(i >= 0 && i <= 2); return i == 0 ? x : i == 1 ? y : z; }
	const T& operator[](const size_t i) const { assert(i >= 0 && i <= 2); return i == 0 ? x : i == 1 ? y : z; }
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

template<size_t SIZE, typename T> T operator*(Vector<SIZE, T> lhs, const Vector<SIZE, T>& rhs) {
	T ret = T();
	for (size_t i = SIZE; i--; ret += lhs[i] * rhs[i]);
	return ret;
}

template<size_t SIZE, typename T, typename U> Vector<SIZE, T>operator *(Vector<SIZE, T> lhs, const U& f) {
	for (size_t i = SIZE; i--; lhs[i] *= f);
	return lhs;
}

template<size_t SIZE, typename T, typename U> Vector<SIZE, T>operator /(Vector<SIZE, T> lhs, const U& f) {
	for (size_t i = SIZE; i--; lhs[i] /= f);
	return lhs;
}

template<size_t TARGET_SIZE, size_t SIZE, typename T> Vector<SIZE, T> embed(const Vector<SIZE, T>& v, T fill = 1) {
	Vector<SIZE, T> ret;
	for (size_t i = TARGET_SIZE; i--; ret[i] = i > SIZE ? fill : v[i]);
	return ret;
}

template<size_t TARGET_SIZE, size_t SIZE, typename T> Vector<TARGET_SIZE, T> proj(const Vector<SIZE, T>& v) {
	Vector<TARGET_SIZE, T> ret;
	for (size_t i = TARGET_SIZE; i--; ret[i] = v[i]);
	return ret;
}

template <typename T> Vector<3, T> cross(Vector<3, T> v1, Vector<3, T> v2) {
	return Vector<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template <size_t SIZE, typename T> std::ostream& operator<<(std::ostream& out, Vector<SIZE, T>& v) {
	for (unsigned int i = 0; i < SIZE; i++) {
		out << v[i] << " ";
	}
	return out;
}

template<size_t ROW_SIZE, size_t COL_SIZE, typename T> class Matrix;

///////////////////////////////////////����ʽ//////////////////////////////////////////

template<size_t SIZE, typename T> struct dt {
	static T det(const Matrix<SIZE, SIZE, T>& src) {
		T ret = 0;
		for (size_t i = SIZE; i--; ret += src[0][i] * src.cofactor(0, i));
		return ret;
	}
};

template<typename T> struct dt<1, T> {
	static T det(const Matrix<1, 1, T>& src) {
		return src[0][0];
	}
};

///////////////////////////////////////����ʽ//////////////////////////////////////////


template<size_t ROW_SIZE, size_t COL_SIZE, typename T> class Matrix
{
private:
	Vector<COL_SIZE, T> rows[ROW_SIZE];
public:
	Matrix() {};
	Vector<COL_SIZE, T>& operator[](const size_t idx) {
		assert(idx < ROW_SIZE);
		return rows[idx];
	}

	const Vector<COL_SIZE, T>& operator[](const size_t idx) const {
		assert(idx < ROW_SIZE);
		return rows[idx];
	}

	Vector<ROW_SIZE, T>& getCol(const size_t idx) {
		assert(idx < COL_SIZE);
		Vector<ROW_SIZE, T> ret;
		for (size_t i = ROW_SIZE; i--; ret[i] = rows[i][idx]);
		return ret;
	}

	void setCol(size_t idx, Vector<ROW_SIZE, T> v) {
		assert(idx < COL_SIZE);
		for (size_t i = ROW_SIZE; i--; rows[i][idx] = v[i]);
	}

	//��λ����
	static Matrix<ROW_SIZE, COL_SIZE, T> identity() {
		assert(ROW_SIZE == COL_SIZE);
		Matrix<ROW_SIZE, COL_SIZE, T> ret;
		for (size_t i = 0; i < ROW_SIZE; i++)
		{
			ret[i][i] = 1;
		}
		return ret;
	}

	//ת�þ���
	Matrix<ROW_SIZE, COL_SIZE, T> transpose() {
		Matrix<COL_SIZE, ROW_SIZE, T> ret;
		for (size_t i = COL_SIZE; i--; ret[i] = this->getCol(i));
		return ret;
	}

	//�Ӿ�������ʽ��ȥ����row�к͵�col�У�
	Matrix<ROW_SIZE - 1, COL_SIZE - 1, T> get_minor(size_t row, size_t col) const {
		Matrix<ROW_SIZE - 1, COL_SIZE - 1, T> ret;
		for (size_t i = ROW_SIZE - 1; i--; )
			for (size_t j = COL_SIZE - 1; j--; ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1]);
		return ret;
	}

	//����ʽ
	T det() const {
		return dt<COL_SIZE, T>::det(*this);
	}

	//����ʽ C = M * (-1)^i+j
	T cofactor(size_t row, size_t col) const {
		return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
	}

	//��������ڡ�3D��ѧ����  ͼ�κ���Ϸ���� ��2�桷�У�������������ָԭ���������ʽ�ľ����ת�ã��������adjugate��û��ת�ã�
	Matrix<ROW_SIZE, COL_SIZE, T> adjugate() const {
		Matrix<ROW_SIZE, COL_SIZE, T> ret;
		for (size_t i = ROW_SIZE; i--; )
			for (size_t j = COL_SIZE; j--; ret[i][j] = cofactor(i, j));
		return ret;
	}

	//��ת�þ���
	Matrix<ROW_SIZE, COL_SIZE, T> invert_transpose() {
		Matrix<ROW_SIZE, COL_SIZE, T> ret = adjugate();
		//�ڡ�3D��ѧ����  ͼ�κ���Ϸ���� ��2�桷�У������Ӧ���Ǿ����������������ʽ�����㣨����İ������û��ת�ã����Խ����������ת�ã��������tmpӦ����ԭ���������ʽ��Ҳ����T tmp = det();
		//���ﲻ֪������ʲô��ħ�����ð������δת�ã��ĵ�һ����ԭ����ĵ�һ�н��е�ˣ��ó����������ʽ��ֵһ�£��д�ѧϰ
		T tmp = ret[0] * rows[0];
		return ret / tmp;
	}

	//�����
	Matrix<ROW_SIZE, COL_SIZE, T> invert() {
		return invert_transpose().transpose();
	}
};

template<size_t ROW_SIZE, size_t COL_SIZE, typename T> Vector<ROW_SIZE, T> operator*(const Matrix<ROW_SIZE, COL_SIZE, T>& lhs, const Vector<COL_SIZE, T>& rhs) {
	Vector<ROW_SIZE, T> ret;
	for (size_t i = ROW_SIZE; i--; ret[i] = lhs[i] * rhs);
	return ret;
}

template<size_t R1, size_t C1, size_t C2, typename T>Matrix<R1, C2, T> operator*(const Matrix<R1, C1, T>& lhs, const Matrix<C1, C2, T>& rhs) {
	Matrix<R1, C2, T> result;
	for (size_t i = R1; i--; )
		for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.col(j));
	return result;
}

template<size_t ROW_SIZE, size_t COL_SIZE, typename T>Matrix<COL_SIZE, ROW_SIZE, T> operator/(Matrix<ROW_SIZE, COL_SIZE, T> lhs, const T& rhs) {
	for (size_t i = ROW_SIZE; i--; lhs[i] = lhs[i] / rhs);
	return lhs;
}

template <size_t ROW_SIZE, size_t COL_SIZE, class T> std::ostream& operator<<(std::ostream& out, Matrix<ROW_SIZE, COL_SIZE, T>& m) {
	for (size_t i = 0; i < ROW_SIZE; i++) out << m[i] << std::endl;
	return out;
}


typedef Vector<2, float> Vector2f;
typedef Vector<2, int>   Vector2i;
typedef Vector<3, float> Vector3f;
typedef Vector<3, int>   Vector3i;
typedef Vector<4, float> Vector4f;

typedef Matrix<4, 4, float> Matrix4x4;
typedef Matrix<3, 3, float> Matrix3x3;