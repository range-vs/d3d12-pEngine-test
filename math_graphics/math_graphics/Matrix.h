#pragma once

// c++
#include <algorithm>

// math library
#include "MathExcep.h"

namespace gmath
{
	template<class T, size_t arr>
	class _Matrix
	{
	protected:
		T matrix[arr][arr];

	public:
		// конструкторы
		_Matrix();
		_Matrix(const T& e);
		// конструктор инициализации вектором(-и) находится в наследниках
		_Matrix(const _Matrix<T, arr>& m);
		_Matrix(const std::initializer_list<T>& l);
		_Matrix(T* arr, size_t s);

		// виртуальные
		/*virtual void identity() = 0;
		virtual void transponse() = 0;
		virtual void reverse() = 0;
		virtual float determinant() = 0;
		virtual void inverse() = 0;
		virtual std::string c_str() = 0;
		virtual std::wstring c_wstr() = 0;*/
	};

	// конструкторы

	template<class T, size_t arr>
	inline _Matrix<T, arr>::_Matrix()
	{
		for (auto& l : matrix)
		{
			for (auto & e : l)
				e = 0;
		}
	}

	template<class T, size_t arr>
	inline _Matrix<T, arr>::_Matrix(const T & _e)
	{
		for (auto& l : matrix)
		{
			for (auto & e : l)
				e = _e;
		}
	}

	template<class T, size_t arr>
	inline _Matrix<T, arr>::_Matrix(const _Matrix<T, arr>& m)
	{
		for (int i(0); i < arr; i++)
			std::copy(m.matrix[i], m.matrix[i] + arr, this->matrix[i]);
	}

	template<class T, size_t arr>
	inline _Matrix<T, arr>::_Matrix(const std::initializer_list<T>& l)
	{
		auto iter = l.begin();
		for (auto& line : matrix)
		{
			for (auto & e : line)
				if (iter != l.end())
					e = *(iter++);
		}
	}

	template<class T, size_t arr>
	inline _Matrix<T, arr>::_Matrix(T * a, size_t s)
	{
		int c(0);
		for (int i(0); i < arr; i++)
			for (int j(0); j < arr; j++)
				if(c < s)
					matrix[i][j] = a[c++];
	}
}