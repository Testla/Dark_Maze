#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <utility>

template <typename T>
class Matrix {
private:
    T *_data;
    std::pair<int, int> _size;
public:
    explicit Matrix(std::pair<int, int> init_size);
	~Matrix();
    void assign(const T& value);
    T * operator [](const int row_index);
    T & at(std::pair<int, int> position);
    std::pair<int, int> size();
    void resize(std::pair<int, int> new_size);
};

template <typename T>
Matrix<T>::Matrix(std::pair<int, int> init_size) : _size(init_size) {
    _data = new T[_size.first * _size.second];
}

template <typename T>
Matrix<T>::~Matrix() {
	delete[]_data;
}

template <typename T>
void Matrix<T>::assign(const T& value) {
	for (int index = 0; index < _size.first * _size.second; ++index)
		_data[index] = value;
}

template <typename T>
T * Matrix<T>::operator [](const int row_index) {
    return _data + row_index * _size.second;
}

template <typename T>
T & Matrix<T>::at(std::pair<int, int> position) {
    return _data[position.first * _size.second + position.second];
}

template <typename T>
std::pair<int, int> Matrix<T>::size() {
    return _size;
}

template <typename T>
void Matrix<T>::resize(std::pair<int, int> new_size) {
    if (_size != new_size) {
        delete []_data;
        _size = new_size;
        _data = new T[_size.first * _size.second];
    }
}

#endif  /* __MATRIX_HPP__ */
