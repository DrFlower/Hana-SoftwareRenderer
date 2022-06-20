#pragma once
#include <cassert>
#include <iostream>
#include "math.h"


template<size_t ROW_SIZE, size_t COL_SIZE, typename T> class Matrix;

///////////////////////////////////////行列式//////////////////////////////////////////

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

///////////////////////////////////////行列式//////////////////////////////////////////


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

	Vector<ROW_SIZE, T> getCol(const size_t idx) const {
		assert(idx < COL_SIZE);
		Vector<ROW_SIZE, T> ret;
		for (size_t i = ROW_SIZE; i--; ret[i] = rows[i][idx]);
		return ret;
	}

	void setCol(size_t idx, Vector<ROW_SIZE, T> v) {
		assert(idx < COL_SIZE);
		for (size_t i = ROW_SIZE; i--; rows[i][idx] = v[i]);
	}

	static Matrix<ROW_SIZE, COL_SIZE, T> identity() {
		Matrix<ROW_SIZE, COL_SIZE, T> ret;
		for (size_t i = ROW_SIZE; i--; )
			for (size_t j = COL_SIZE; j--; ret[i][j] = (i == j));
		return ret;
	}


	//转置矩阵
	Matrix<COL_SIZE, ROW_SIZE, T> transpose() {
		Matrix<COL_SIZE, ROW_SIZE, T> ret;
		for (size_t i = COL_SIZE; i--; ret[i] = this->getCol(i));
		return ret;
	}

	//子矩阵行列式（去除第row行和第col列）
	Matrix<ROW_SIZE - 1, COL_SIZE - 1, T> get_minor(size_t row, size_t col) const {
		Matrix<ROW_SIZE - 1, COL_SIZE - 1, T> ret;
		for (size_t i = ROW_SIZE - 1; i--; )
			for (size_t j = COL_SIZE - 1; j--; ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1]);
		return ret;
	}

	//行列式
	T det() const {
		return dt<COL_SIZE, T>::det(*this);
	}

	//余子式 C = M * (-1)^i+j
	T cofactor(size_t row, size_t col) const {
		return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
	}

	//伴随矩阵（在《3D数学基础  图形和游戏开发 第2版》中，经典伴随矩阵是指原矩阵的余子式的矩阵的转置，但这里的adjugate并没有转置）
	Matrix<ROW_SIZE, COL_SIZE, T> adjugate() const {
		Matrix<ROW_SIZE, COL_SIZE, T> ret;
		for (size_t i = ROW_SIZE; i--; )
			for (size_t j = COL_SIZE; j--; ret[i][j] = cofactor(i, j));
		return ret;
	}

	//逆转置矩阵
	Matrix<ROW_SIZE, COL_SIZE, T> invert_transpose() {
		Matrix<ROW_SIZE, COL_SIZE, T> ret = adjugate();
		//在《3D数学基础  图形和游戏开发 第2版》中，逆矩阵应该是经典伴随矩阵除以行列式来计算（这里的伴随矩阵没做转置，所以结果是逆矩阵的转置），这里的tmp应该是原矩阵的行列式，也就是T tmp = det();
		//这里不知道用了什么黑魔法，用伴随矩阵（未转置）的第一行与原矩阵的第一行进行点乘，得出结果与行列式数值一致，有待学习
		T tmp = ret[0] * rows[0];
		return ret / tmp;
	}

	//逆矩阵
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
		for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.getCol(j));
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

typedef Matrix<4, 4, float> Matrix4x4;
typedef Matrix<3, 3, float> Matrix3x3;