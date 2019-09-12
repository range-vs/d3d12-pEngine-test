#pragma once

#include "mathematics_typedef_vector.h"

//// matrix
//#include "Matrix3.h"

namespace gmath
{
	template<class T>
	class Vector_3 : public _Vector<T>
	{
		T z;

	public:
		// конструкторы
		Vector_3() :_Vector<T>() { this->z = 0; }
		Vector_3(const T& v) :_Vector<T>(v) { this->z = v; }
		Vector_3(const Vector_3& v) :_Vector<T>(v) { this->z = v.z; };
		Vector_3(const T& x, const T& y, const T& z) : _Vector<T>(x, y) { this->z = z; }
		Vector_3(const std::initializer_list<T>& t);

		// Преобразования типов
		explicit operator Vector_2<T>()const;
		explicit operator Vector_4<T>()const;

		// Перегруженные операторы
		// присваивание
		Vector_3& operator=(const std::initializer_list<T>& l); // присваивание списка инициализации
		Vector_3& operator=(const Vector_3<T>& v); // присвоение копии
		Vector_3& operator=(const T& v); // присвоение числа

		// умножение на скаляр 
		Vector_3& operator*=(const T& sc);
		Vector_3 operator*(const T& sc)const;
		Vector_3 operator*(const T& sc);
		template<class T1>
		friend Vector_3<T1>operator*(const T1& sc, const Vector_3<T1>& v);

		// умножение скалярное 
		T operator*(const Vector_3<T>& v)const;

		// сравнение
		bool operator==(const Vector_3<T>& v)const;
		bool operator!=(const Vector_3<T>& v);
		bool operator<(const T& v);
		bool operator>(const T& v);
		bool operator<=(const T& v);
		bool operator>=(const T& v);

		// прибавление скаляра
		Vector_3& operator+=(const T& sc);
		Vector_3 operator+(const T& sc)const;
		template<class T1>
		friend Vector_3<T1>operator+(const T1& sc, const Vector_3<T1>& v);

		// вычитание скаляра 
		Vector_3& operator-=(const T& sc);
		Vector_3 operator-(const T& sc)const;

		// сложение векторов
		Vector_3& operator+=(const Vector_3<T>& v);
		Vector_3 operator+(const Vector_3<T>& v)const;

		// вычитание векторов
		Vector_3& operator-=(const Vector_3<T>& v);
		Vector_3 operator-(const Vector_3<T>& v)const;

		// инверсия вектора
		template<class T1>
		friend Vector_3<T1>operator~(const Vector_3<T1>& v);

		// доступ к координате
		T& operator[](const gmtypes::VectorCoordinate& i);
		T operator[](const gmtypes::VectorCoordinate& i)const;

		// умножение на матрицу
		Vector_3& operator*=(const Matrix_4<T>& m);
		Vector_3 operator*(const Matrix_4<T>& m);

		// инкремент/декремент
		Vector_3 operator++(int);
		Vector_3& operator++();
		Vector_3 operator--(int);
		Vector_3& operator--();

		// унарные операторы
		template<class T1>
		friend const Vector_3<T1>operator-(const Vector_3<T1>& v);

		// методы
		// абстрактные методы
		float length();
		void normalize();
		void inverse();
		void identity();
		std::string c_str();
		std::wstring c_wstr();

		// основные
		static float Length(const Vector_3<T>& v);
		static Vector_3 Normalize(const Vector_3<T>& v);
		static Vector_3 Inverse(const Vector_3<T>& v);
		static Vector_3 Identity();

		// вектор по двум точкам
		static Vector_3 createVectorForDoublePoints(Vector_3<T>& v1, Vector_3<T>& v2);

		// скалярное произведение
		T dot(const Vector_3<T>& v);
		static T dot(const Vector_3<T>& v1, const Vector_3<T>& v2);

		// векторное произведение
		Vector_3& cross(const Vector_3<T>& v);
		static Vector_3 cross(const Vector_3<T>& v1, const Vector_3<T>& v2);

		// точка между двумя
		Vector_3 pointOfLeft(const Vector_3<T> v, int n = 1);
		Vector_3 pointOfRight(const Vector_3<T> v, int n = 1);

		// преобразование
		void vector3TransformCoord(const Matrix_4<T>& m);
		Vector_3 vector3TransformCoord(const Vector_3<T>& v, const Matrix_4<T>& m);
		void vector3TransformNormal(const Matrix_4<T>& m);
		Vector_3 vector3TransformNormal(const Vector_3<T>& v, const Matrix_4<T>& m);

		// возврат массива
		void toArray(T* arr);
	};

	// конструкторы

	template<class T>
	inline Vector_3<T>::Vector_3(const std::initializer_list<T>& t) : _Vector<T>(t)
	{
		if (t.size() >= 3)
			this->z = *(t.end() - 1);
	}

	// преобразование типов

	template<class T>
	inline Vector_3<T>::operator Vector_2<T>()const
	{
		return Vector_2<T>(this->x, this->y);
	}

	template<class T>
	inline Vector_3<T>::operator Vector_4<T>()const
	{
		return Vector_4<T>(this->x, this->y, this->z, 0);
	}

	// перегруженные операторы
	// оператор присваивания

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator=(const std::initializer_list<T>& l)
	{
		this->_initializate(l);
		if (l.size() >= 3)
			this->z = *(l.end() - 1);
		return *this;
	}

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator=(const Vector_3<T>& v)
	{
		if (this != &v)
		{
			this->_copy(v.x, v.y);
			this->z = v.z;
		}
		return *this;
	}

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator=(const T & v)
	{
		if (this->x != v || this->y != v || this->z != v)
			this->x = this->y = this->z = v;
		return *this;
	}

	// умножение на скаляр

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator*=(const T & sc)
	{
		this->x *= sc;
		this->y *= sc;
		this->z *= sc;
		return *this;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator*(const T & sc)const
	{
		Vector_3<T> v(*this);
		v.x *= sc;
		v.y *= sc;
		v.z *= sc;
		return v;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator*(const T & sc)
	{
		Vector_3<T> v(*this);
		v.x *= sc;
		v.y *= sc;
		v.z *= sc;
		return v;
	}

	template<class T1>
	inline Vector_3<T1>operator*(const T1 & sc, const Vector_3<T1>& v)
	{
		return v * sc;
	}

	// умножение скалярное

	template<class T>
	inline T Vector_3<T>::operator*(const Vector_3<T>& v)const
	{
		return (this->x * v.x + this->y * v.y + this->z * v.z);
	}

	// сравнение

	template<class T>
	inline bool Vector_3<T>::operator==(const Vector_3 <T>& v)const
	{
		return this->x == v.x && this->y == v.y && this->z == v.z;
	}

	template<class T>
	inline bool Vector_3<T>::operator!=(const Vector_3<T>& v)
	{
		return !(*this == v);
	}

	template<class T>
	inline bool Vector_3<T>::operator<(const T & v)
	{
		return this->x < v && this->y < v && this->z < v;
	}

	template<class T>
	inline bool Vector_3<T>::operator>(const T & v)
	{
		return this->x > v && this->y > v && this->z > v;
	}

	template<class T>
	inline bool Vector_3<T>::operator<=(const T & v)
	{
		return this->x <= v && this->y <= v && this->z <= v;
	}

	template<class T>
	inline bool Vector_3<T>::operator>=(const T & v)
	{
		return this->x >= v && this->y >= v && this->z >= v;
	}

	// Прибавление скаляра

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator+=(const T & sc)
	{
		this->x += sc;
		this->y += sc;
		this->z += sc;
		return *this;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator+(const T & sc)const
	{
		Vector_3<T> vec(*this);
		vec.x += sc;
		vec.y += sc;
		vec.z += sc;
		return vec;
	}

	template<class T1>
	inline Vector_3<T1>operator+(const T1 & sc, const Vector_3<T1>& v)
	{
		return v * sc;
	}

	// вычитание скаляра

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator-=(const T & sc)
	{
		this->x -= sc;
		this->y -= sc;
		this->z -= sc;
		return *this;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator-(const T & sc)const
	{
		Vector_3<T> vec(*this);
		vec.x -= sc;
		vec.y -= sc;
		vec.z -= sc;
		return vec;
	}

	// сложение векторов

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator+=(const Vector_3<T>& v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator+(const Vector_3<T>& v)const
	{
		Vector_3<T> vec(*this);
		vec.x += v.x;
		vec.y += v.y;
		vec.z += v.z;
		return vec;
	}

	// вычитание векторов

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator-=(const Vector_3<T>& v)
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.x;
		return *this;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator-(const Vector_3<T>& v)const
	{
		Vector_3<T> vec(*this);
		vec.x -= v.x;
		vec.y -= v.y;
		vec.z -= v.z;
		return vec;
	}

	// инверсия вектора

	template<class T1>
	inline Vector_3<T1>operator~(const Vector_3<T1>& v)
	{
		return Vector_3<T1>(-v.x, -v.y, -v.z);
	}

	// доступ к координате

	template<class T>
	inline T & Vector_3<T>::operator[](const gmtypes::VectorCoordinate & i)
	{
		switch (i)
		{
		case gmtypes::x:
			return this->x;

		case gmtypes::y:
			return this->y;

		case gmtypes::z:
			return this->z;

		default:
#ifdef UNICODE
			throw gmexception::VectorIndexException(L"Error index for Vector3!");
#else
			throw gmexception::VectorIndexException("Error index for Vector3!");
#endif
		}
	}

	template<class T>
	inline T Vector_3<T>::operator[](const gmtypes::VectorCoordinate & i)const
	{
		switch (i)
		{
		case gmtypes::x:
			return this->x;

		case gmtypes::y:
			return this->y;

		case gmtypes::z:
			return this->z;

		default:
#ifdef UNICODE
			throw gmexception::VectorIndexException(L"Error index for Vector3!");
#else
			throw gmexception::VectorIndexException("Error index for Vector3!");
#endif
		}
	}

	// умножение на матрицу
	template<class T>
	inline Vector_3<T> & gmath::Vector_3<T>::operator*=(const Matrix_4<T>& m)
	{
		this->vector3TransformCoord(m);
		return *this;
	}

	template<class T>
	inline Vector_3<T> gmath::Vector_3<T>::operator*(const Matrix_4<T>& m)
	{
		Vector_3<T> v(*this); v.vector3TransformCoord(m);
		return v;
	}

	// инкремент/декремент

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator++(int)
	{
		Vector_3<T> v(*this);
		this->x++;
		this->y++;
		this->z++;
		return v;
	}

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator++()
	{
		this->x++;
		this->y++;
		this->z++;
		return *this;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::operator--(int)
	{
		Vector_3<T> v(*this);
		this->x--;
		this->y--;
		this->z--;
		return v;
	}

	template<class T>
	inline Vector_3<T> & Vector_3<T>::operator--()
	{
		this->x--;
		this->y--;
		this->z--;
		return *this;
	}

	// унарные операторы
	template<class T1>
	const Vector_3<T1>operator-(const Vector_3<T1>& v)
	{
		Vector_3<T1>_v(v); _v.inverse();
		return _v;
	}

	// методы
	// абстрактные методы

	template<class T>
	inline float Vector_3<T>::length()
	{
		return sqrtf(powf(this->x, 2) + powf(this->y, 2) + powf(this->z, 2));
	}

	template<class T>
	inline void Vector_3<T>::normalize()
	{
		float invLength = 1 / this->length();
		this->x *= invLength;
		this->y *= invLength;
		this->z *= invLength;
	}

	template<class T>
	inline void Vector_3<T>::inverse()
	{
		*this = ~(*this);
	}

	template<class T>
	inline void Vector_3<T>::identity()
	{
		this->x = 1;
		this->y = 1;
		this->z = 1;
	}

	template<class T>
	inline std::string gmath::Vector_3<T>::c_str()
	{
		std::stringstream buf;
		buf << "[" << std::setprecision(std::to_string((int)this->x).length() + 3) << this->x << ", " << std::setprecision(std::to_string((int)this->y).length() + 3) << this->y <<
			", " << std::setprecision(std::to_string((int)this->z).length() + 3) << this->z << "]";
		return buf.str();
	}

	template<class T>
	inline std::wstring gmath::Vector_3<T>::c_wstr()
	{
		std::wstringstream buf;
		buf << L"[" << std::setprecision(std::to_wstring((int)this->x).length() + 3) << this->x << L", " << std::setprecision(std::to_wstring((int)this->y).length() + 3) << this->y <<
			L", " << std::setprecision(std::to_wstring((int)this->z).length() + 3) << this->z << L"]";
		return buf.str();
	}

	// основные
	template<class T>
	inline float gmath::Vector_3<T>::Length(const Vector_3<T>& v)
	{
		return v.length();
	}

	template<class T>
	inline Vector_3<T> gmath::Vector_3<T>::Inverse(const Vector_3<T>& v)
	{
		Vector_3<float> _v(v);
		_v.inverse();
		return _v;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::Identity()
	{
		Vector_3<float> v;
		v.identity();
		return v;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::Normalize(const Vector_3<T>& v)
	{
		Vector_3<T> vec(v);
		vec.normalize();
		return vec;
	}

	// вектор по двум точкам

	template<class T>
	inline Vector_3<T> Vector_3<T>::createVectorForDoublePoints(Vector_3<T>& v1, Vector_3<T>& v2)
	{
		return v2 - v1;
	}

	// скалярное произведение

	template<class T>
	inline T Vector_3<T>::dot(const Vector_3<T>& v)
	{
		return *this * v;
	}

	template<class T>
	inline T Vector_3<T>::dot(const Vector_3<T>& v1, const Vector_3<T>& v2)
	{
		return v1 * v2;
	}

	// векторное произведение
	template<class T>
	inline Vector_3<T> & gmath::Vector_3<T>::cross(const Vector_3<T>& v)
	{
		Vector_3<T> res;
		res.x = this->y * v.z - this->z * v.y;
		res.y = this->z * v.x - this->x * v.z;
		res.z = this->x * v.y - this->y * v.x;
		*this = res;
		return *this;
	}

	template<class T>
	inline Vector_3<T> gmath::Vector_3<T>::cross(const Vector_3<T>& v1, const Vector_3<T>& v2)
	{
		Vector_3<T> res(v1);
		res.cross(v2);
		return res;
	}

	// точка между двумя другими

	template<class T>
	inline Vector_3<T> Vector_3<T>::pointOfLeft(const Vector_3<T> v, int n)
	{
		if (n <= 0)
			n = 1;

		Vector_3<T> v1(*this);
		Vector_3<T> v2(v);
		Vector_3<T> result;
		for (int i(0); i < n; i++)
		{
			result.x = (v1.x + v2.x) / 2;
			result.y = (v1.y + v2.y) / 2;
			result.z = (v1.z + v2.z) / 2;
			v2 = result;
		}
		return result;
	}

	template<class T>
	inline Vector_3<T> Vector_3<T>::pointOfRight(const Vector_3<T> v, int n)
	{
		if (n <= 0)
			n = 1;

		Vector_3<T> v1(*this);
		Vector_3<T> v2(v);
		Vector_3<T> result;
		for (int i(0); i < n; i++)
		{
			result.x = (v1.x + v2.x) / 2;
			result.y = (v1.y + v2.y) / 2;
			result.z = (v1.z + v2.z) / 2;
			v1 = result;
		}
		return result;
	}

	// преобразование
	template<class T>
	inline void gmath::Vector_3<T>::vector3TransformCoord(const Matrix_4<T>& m)
	{
		float norm = m(0, 3) * this->x + m(1, 3) * this->y + this->z * m(2, 3) + m(3,3);
		Vector_3<T> tmp(*this);
		if (norm)
		{
			this->x = (m(0, 0) * tmp.x + m(1, 0) * tmp.y + tmp.z *m(2, 0) + m(3, 0)) / norm;
			this->y = (m(0, 1) * tmp.x + m(1, 1) * tmp.y + tmp.z *m(2, 1) + m(3, 1)) / norm;
			this->z = (m(0, 2) * tmp.x + m(1, 2) * tmp.y + tmp.z * m(2, 2) + m(3,2)) / norm;
		}
		else
			this->x = this->y = this->z = 0.f;
	}

	template<class T>
	inline Vector_3<T> gmath::Vector_3<T>::vector3TransformCoord(const Vector_3<T>& v, const Matrix_4<T>& m)
	{
		Vector_3<T> _v(v);
		_v.vector3TransformCoord(m);
		return _v;
	}

	template<class T>
	inline void gmath::Vector_3<T>::vector3TransformNormal(const Matrix_4<T>& m)
	{
		Vector_3<T> tmp(*this);
		this->x = m(0, 0) * tmp.x + m(1, 0) * tmp.y + tmp.z *m(2, 0);
		this->y = m(0, 1) * tmp.x + m(1, 1) * tmp.y + tmp.z *m(2, 1);
		this->z = m(0, 2) * tmp.x + m(1, 2) * tmp.y + tmp.z * m(2, 2);
	}

	template<class T>
	inline Vector_3<T> gmath::Vector_3<T>::vector3TransformNormal(const Vector_3<T>& v, const Matrix_4<T>& m)
	{
		Vector_3<T> _v(v);
		_v.vector3TransformNormal(m);
		return _v;
	}

	// возвращаем массив
	template<class T>
	inline void gmath::Vector_3<T>::toArray(T* arr)
	{
		arr[0] = this->x;
		arr[1] = this->y;
		arr[2] = this->z;
	}
}