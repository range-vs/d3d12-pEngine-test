#pragma once

#include "mathematics_typedef_vector.h"

namespace gmath
{
	template<class T>
	class Vector_2 : public _Vector<T>
	{
	public:
		// конструкторы
		Vector_2() :_Vector<T>() {}
		Vector_2(const T& v) :_Vector<T>(v) {}
		Vector_2(const Vector_2<T>& v) :_Vector<T>(v) {};
		Vector_2(const T& x, const T& y) : _Vector<T>(x, y) {}
		Vector_2(const std::initializer_list<T>& t) : _Vector<T>(t) {};

		// Преобразования типов
		explicit operator Vector_3<T>()const;
		explicit operator Vector_4<T>()const;

		// Перегруженные операторы
		// присваивание
		Vector_2 & operator=(const std::initializer_list<T>& l); // присваивание списка инициализации
		Vector_2& operator=(const Vector_2<T>& v); // присвоение копии
		Vector_2& operator=(const T& v); // присвоение числа

		// умножение на скаляр
		Vector_2& operator*=(const T& sc);
		Vector_2 operator*(const T& sc);
		template<class T1>
		friend Vector_2<T1>operator*(const T1& sc, Vector_2<T1>& v);

		// умножение скалярное
		T operator*(const Vector_2<T>& v);

		// сравнение
		bool operator==(const Vector_2<T>& v);
		bool operator!=(const Vector_2<T>& v);
		bool operator<(const T& v);
		bool operator>(const T& v);
		bool operator<=(const T& v);
		bool operator>=(const T& v);

		// прибавление скаляра
		Vector_2& operator+=(const T& sc);
		Vector_2 operator+(const T& sc);
		template<class T1>
		friend Vector_2<T1>operator+(const T1& sc, const Vector_2<T1>& v);

		// вычитание скаляра
		Vector_2& operator-=(const T& sc);
		Vector_2 operator-(const T& sc);

		// сложение векторов
		Vector_2& operator+=(const Vector_2<T>& v);
		Vector_2 operator+(const Vector_2<T>& v);

		// вычитание векторов
		Vector_2& operator-=(const Vector_2<T>& v);
		Vector_2 operator-(const Vector_2<T>& v);

		// инверсия вектора
		template<class T1>
		friend Vector_2<T1>operator~(const Vector_2<T1>& v);

		// доступ к координате
		T& operator[](const gmtypes::VectorCoordinate& i);
		T operator[](const gmtypes::VectorCoordinate& i)const;

		// умножение на матрицу
		Vector_2& operator*=(const Matrix_4<T>& m);
		Vector_2 operator*(const Matrix_4<T>& m);

		// инкремент/декремент
		Vector_2 operator++(int);
		Vector_2& operator++();
		Vector_2 operator--(int);
		Vector_2& operator--();

		// унарные операторы
		template<class T1>
		friend const Vector_2<T1>operator-(const Vector_2<T1>& v);

		// методы
		// абстрактные методы
		T length();
		void normalize();
		void inverse();
		void identity();
		std::string c_str();
		std::wstring c_wstr();

		// основные
		static T Length(const Vector_2<T>& v);
		static Vector_2 Normalize(const Vector_2<T>& v);
		static Vector_2 Inverse(const Vector_2<T>& v);
		static Vector_2 Identity();

		// вектор по двум точкам
		static Vector_2 createVectorForDoublePoints(Vector_2<T>& v1,  Vector_2<T>& v2);

		// скалярное произведение
		T dot(const Vector_2<T>& v);
		static T dot(Vector_2<T>& v1, Vector_2<T>& v2);

		// точка между двумя
		Vector_2 pointOfLeft(const Vector_2<T> v, int n = 1);
		Vector_2 pointOfRight(const Vector_2<T> v, int n = 1);

		// преобразование
		void vector2TransformCoord(const Matrix_4<T>& m);
		static Vector_2 vector2TransformCoord(const Vector_2<T>& v, const Matrix_4<T>& m);
		void vector2TransformNormal(const Matrix_4<T>& m);
		static Vector_2 vector2TransformNormal(const Vector_2<T>& v, const Matrix_4<T>& m);
	};

	// преобразование типов
	template<class T>
	inline Vector_2<T>::operator Vector_3<T>() const
	{
		return Vector_3<T>(this->x, this->y, 0);
	}

	 template<class T>
	inline Vector_2<T>::operator Vector_4<T>() const
	{
		return Vector_4<T>(this->x, this->y, 0, 0);
	}

	// перегруженные операторы
	// оператор присвоения

	template<class T>
	inline Vector_2<T> & Vector_2<T>::operator=(const std::initializer_list<T>& l)
	{
		this->_initializate(l);
		return *this;
	}

	 template<class T>
	inline Vector_2<T>& Vector_2<T>::operator=(const Vector_2<T>& v)
	{
		if (this != &v)
			this->_copy(v.x, v.y);
		return *this;
	}

	 template<class T>
	inline Vector_2<T> & Vector_2<T>::operator=(const T & v)
	{
		if (this->x != v || this->y != v)
			this->x = this->y = v;
		return *this;
	}

	// умножение на скаляр

	 template<class T>
	inline Vector_2<T> & Vector_2<T>::operator*=(const T & sc)
	{
		this->x *= sc;
		this->y *= sc;
		return *this;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::operator*(const T & sc)
	{
		Vector_2<T> v(*this);
		v.x *= sc;
		v.y *= sc;
		return v;
	}

	template<class T1>
	inline Vector_2<T1>operator*(const T1 & sc, Vector_2<T1>& v)
	{
		Vector_2<T1>vec(v);
		vec.x *= sc;
		vec.y *= sc;
		return vec;
	}

	// умножение скалярное

	 template<class T>
	inline T Vector_2<T>::operator*(const Vector_2<T>& v)
	{
		return this->x * v.x + this->y + v.y;
	}

	// сравнение

	 template<class T>
	inline bool Vector_2<T>::operator==(const Vector_2<T>& v)
	{
		return this->x == v.x && this->y == v.y;
	}

	 template<class T>
	inline bool Vector_2<T>::operator!=(const Vector_2<T>& v)
	{
		return !(*this == v);
	}

	 template<class T>
	inline bool Vector_2<T>::operator<(const T & v)
	{
		return this->x < v && this->y < v;
	}

	 template<class T>
	inline bool Vector_2<T>::operator>(const T & v)
	{
		return this->x > v && this->y > v;
	}

	 template<class T>
	inline bool Vector_2<T>::operator<=(const T & v)
	{
		return this->x <= v && this->y <= v;
	}

	 template<class T>
	inline bool Vector_2<T>::operator>=(const T & v)
	{
		return this->x >= v && this->y >= v;
	}

	// прибавление скаляра
	 template<class T>
	inline Vector_2<T>& Vector_2<T>::operator+=(const T & sc)
	{
		this->x += sc;
		this->y += sc;
		return *this;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::operator+(const T & sc)
	{
		Vector_2<T> v(*this);
		v.x += sc;
		v.y += sc;
		return v;
	}

	template<class T1>
	inline Vector_2<T1>operator+(const T1 & sc, const Vector_2<T1>& v)
	{
		Vector_2<T1>vec(v);
		vec.x += sc;
		vec.y += sc;
		return vec;
	}

	// вычитание скаляра
	 template<class T>
	inline Vector_2<T>& Vector_2<T>::operator-=(const T & sc)
	{
		this->x -= sc;
		this->y -= sc;
		return *this;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::operator-(const T & sc)
	{
		Vector_2<T> v(*this);
		v.x -= sc;
		v.y -= sc;
		return v;
	}


	// сложение векторов

	 template<class T>
	inline Vector_2<T> & Vector_2<T>::operator+=(const Vector_2<T>& v)
	{
		this->x += v.x;
		this->y += v.y;
		return *this;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::operator+(const Vector_2<T>& v)
	{
		Vector_2<T> vec(*this);
		vec.x += v.x;
		vec.y += v.y;
		return vec;
	}

	// вычитание векторов

	 template<class T>
	inline Vector_2<T> & Vector_2<T>::operator-=(const Vector_2<T>& v)
	{
		this->x -= v.x;
		this->y -= v.y;
		return *this;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::operator-(const Vector_2<T>& v)
	{
		Vector_2<T> vec(*this);
		vec.x -= v.x;
		vec.y -= v.y;
		return vec;
	}

	// инверсия вектора

	template<class T1>
	inline Vector_2<T1>operator~(const Vector_2<T1>& v)
	{
		Vector_2<T1>vec(v);
		vec.x = -vec.x;
		vec.y = -vec.y;
		return vec;
	}

	// доступ к координате

	 template<class T>
	inline T & Vector_2<T>::operator[](const gmtypes::VectorCoordinate & i)
	{
		switch (i)
		{
		case gmtypes::x:
			return this->x;

		case gmtypes::y:
			return this->y;

		default:
#ifdef _UNICODE
			throw gmexception::VectorIndexException(L"Error index for Vector2!");
#else
			throw gmexception::VectorIndexException("Error index for Vector2!");
#endif
		}
	}

	 template<class T>
	inline T Vector_2<T>::operator[](const gmtypes::VectorCoordinate & i)const
	{
		switch (i)
		{
		case gmtypes::x:
			return this->x;

		case gmtypes::y:
			return this->y;

		default:
#ifdef _UNICODE
			throw gmexception::VectorIndexException(L"Error index for Vector2!");
#else
			throw gmexception::VectorIndexException("Error index for Vector2!");
#endif
		}
	}

	// умножение на матрицу
	 template<class T>
	inline Vector_2<T> & gmath::Vector_2<T>::operator*=(const Matrix_4<T>& m)
	{
		this->vector2TransformCoord(m);
		return *this;
	}

	 template<class T>
	inline Vector_2<T> gmath::Vector_2<T>::operator*(const Matrix_4<T>& m)
	{
		Vector_2<T> v(*this); v.vector2TransformCoord(m);
		return v;
	}

	// инкремент / декремент

	 template<class T>
	inline Vector_2<T> Vector_2<T>::operator++(int)
	{
		Vector_2<T> v(*this);
		this->x++;
		this->y++;
		return v;
	}

	 template<class T>
	inline Vector_2<T> & Vector_2<T>::operator++()
	{
		this->x++;
		this->y++;
		return *this;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::operator--(int)
	{
		Vector_2<T> v(*this);
		this->x--;
		this->y--;
		return v;
	}

	 template<class T>
	inline Vector_2<T> & Vector_2<T>::operator--()
	{
		this->x--;
		this->y--;
		return *this;
	}

	// унарные операторы
	template<class T1>
	const Vector_2<T1>operator-(const Vector_2<T1>& v)
	{
		Vector_2<T1>_v(v); _v.inverse();
		return _v;
	}

	// методы
	// абстрактные методы

	 template<class T>
	inline T Vector_2<T>::length()
	{
		return sqrtf(powf(this->x, 2) + powf(this->y, 2));
	}

	 template<class T>
	inline void Vector_2<T>::normalize()
	{
		float invLength = 1 / this->length();
		this->x *= invLength;
		this->y *= invLength;
	}

	 template<class T>
	inline void Vector_2<T>::inverse()
	{
		*this = ~(*this);
	}

	 template<class T>
	inline void Vector_2<T>::identity()
	{
		this->x = 1;
		this->y = 1;
	}

	 template<class T>
	inline std::string gmath::Vector_2<T>::c_str()
	{
		std::stringstream buf;
		buf << "[" << std::setprecision(std::to_string((int)this->x).length() + 3) << this->x << ", " << std::setprecision(std::to_string((int)this->y).length() + 3) << this->y << "]";
		return buf.str();
	}

	 template<class T>
	inline std::wstring gmath::Vector_2<T>::c_wstr()
	{
		std::wstringstream buf;
		buf << L"[" << std::setprecision(std::to_wstring((int)this->x).length() + 3) << this->x << L", " << std::setprecision(std::to_wstring((int)this->y).length() + 3) << this->y << L"]";
		return buf.str();
	}

	// основные
	 template<class T>
	inline T gmath::Vector_2<T>::Length(const Vector_2<T>& v)
	{
		return v.length();
	}

	 template<class T>
	inline Vector_2<T> gmath::Vector_2<T>::Inverse(const Vector_2<T>& v)
	{
		Vector_2<T> _v(v); _v.inverse();
		return _v;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::Identity()
	{
		Vector_2<float> v;
		v.identity();
		return v;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::Normalize(const Vector_2<T>& v)
	{
		Vector_2<T> vec(v);
		vec.normalize();
		return vec;
	}

	// вектор по двум точкам

	 template<class T>
	inline Vector_2<T> Vector_2<T>::createVectorForDoublePoints(Vector_2<T>& v1, Vector_2<T>& v2)
	{
		return v2 - v1;
	}

	// скалярное произведение

	 template<class T>
	inline T Vector_2<T>::dot(const Vector_2<T>& v)
	{
		return *this * v;
	}

	 template<class T>
	inline T Vector_2<T>::dot(Vector_2<T>& v1, Vector_2<T>& v2)
	{
		return v1 * v2;
	}

	// точка между двумя другими

	 template<class T>
	inline Vector_2<T> Vector_2<T>::pointOfLeft(const Vector_2<T> v, int n)
	{
		if (n <= 0)
			n = 1;

		Vector_2<T> v1(*this);
		Vector_2<T> v2(v);
		Vector_2<T> result;
		for (int i(0); i < n; i++)
		{
			result.x = (v1.x + v2.x) / 2;
			result.y = (v1.y + v2.y) / 2;
			v2 = result;
		}
		return result;
	}

	 template<class T>
	inline Vector_2<T> Vector_2<T>::pointOfRight(const Vector_2<T> v, int n)
	{
		if (n <= 0)
			n = 1;

		Vector_2<T> v1(*this);
		Vector_2<T> v2(v);
		Vector_2<T> result;
		for (int i(0); i < n; i++)
		{
			result.x = (v1.x + v2.x) / 2;
			result.y = (v1.y + v2.y) / 2;
			v1 = result;
		}
		return result;
	}

	// преобразование
	 template<class T>
	inline void gmath::Vector_2<T>::vector2TransformCoord(const Matrix_4<T>& m)
	{
		float norm = m(0, 3) * this->x + m(1, 3) * this->y + m(3, 3);
		Vector_2<T> tmp(*this);
		if (norm)
		{
			this->x = (m(0, 0) * tmp.x + m(1, 0) * tmp.y + m(3, 0)) / norm;
			this->y = (m(0, 1) * tmp.x + m(1, 1) * tmp.y + m(3, 1)) / norm;
		}
		else
			this->x = this->y = 0.f;
	}

	 template<class T>
	inline Vector_2<T> gmath::Vector_2<T>::vector2TransformCoord(const Vector_2<T>& v, const Matrix_4<T>& m)
	{
		Vector_2<T> tmp(v);
		tmp.vector2TransformCoord(m);
		return tmp;
	}

	template<class T>
	inline void gmath::Vector_2<T>::vector2TransformNormal(const Matrix_4<T>& m)
	{
		Vector_2<T> tmp(this);
		this->x = (m(0, 0) * tmp.x + m(1, 0) * tmp.y);
		this->y = (m(0, 1) * tmp.x + m(1, 1) * tmp.y);
	}

	template<class T>
	inline Vector_2<T> gmath::Vector_2<T>::vector2TransformNormal(const Vector_2<T>& v, const Matrix_4<T>& m)
	{
		Vector_2<T> tmp(v);
		tmp.vector2TransformNormal(m);
		return tmp;
	}
}