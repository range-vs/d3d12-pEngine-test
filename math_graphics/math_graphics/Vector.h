#pragma once

// c++
#include <initializer_list>
#include <math.h>

// math library
#include "MathExcep.h"


namespace gmath
{
	template<class T>
	class _Vector
	{
	protected:
		T x;
		T y;

		// скрытые методы
		void _initializate(const std::initializer_list<T>& l); // инициализация
		void _copy(const T& x, const T& y); // копия
	public:
		// конструкторы
		_Vector() :x(0), y(0) {}
		_Vector(const T& v) : x(v), y(v) {}
		_Vector(const _Vector& v);
		_Vector(const T& x, const T& y) :x(x), y(y) {}
		_Vector(const std::initializer_list<T>& t);
		// методы
		/*virtual float length() = 0;
		virtual void normalize() = 0;
		virtual void inverse() = 0;
		virtual void identity() = 0;
		virtual std::string c_str() = 0;
		virtual std::wstring c_wstr() = 0;*/
	};

	// скрытые методы

	template<class T>
	inline void _Vector<T>::_initializate(const std::initializer_list<T>& l)
	{
		T* ptr[2] = { &x, &y };
		int i(0);
		for (auto & v : l)
			if (i == 2)
				break;
			else
				*ptr[i++] = v;
	}

	template<class T>
	inline void _Vector<T>::_copy(const T & x, const T & y)
	{
		this->x = x;
		this->y = y;
	}

	// конструкторы

	template<class T>
	inline _Vector<T>::_Vector(const _Vector & v)
	{
		this->_copy(v.x, v.y);
	}

	template<class T>
	inline _Vector<T>::_Vector(const std::initializer_list<T>& t) :x(0), y(0)
	{
		this->_initializate(t);
	}

}