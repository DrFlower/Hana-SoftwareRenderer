#include <cassert>

template<int size, typename T>
class vector
{
private:
	T data[size];
public:
	T& operator[](const int index) {
		assert(index < size);
		return data[index];
	}

	const T& operator[](const int index) const {
		assert(index < size);
		return data[index];
	}
};

template< typename T>
class vector<2, T>
{
private:
	T x, y;
public:
	vector() :x(T()), y(T()) {}
	vector(T x, T y) :x(x), y(y) {}


};