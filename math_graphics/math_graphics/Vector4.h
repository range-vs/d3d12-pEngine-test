#pragma once

#include "mathematics_typedef_vector.h"

namespace gmath
{
	template<class T>
	class Vector_4 : public _Vector<T>
	{
		T z;
		T w;

	public:
		// конструкторы
		Vector_4() :_Vector<T>() { this->z = this->w = 0; }
		Vector_4(const T& v) :_Vector<T>(v) { this->z = v; this->w = v; }
		Vector_4(const Vector_4& v) :_Vector<T>(v) { this->z = v.z;  this->w = v.w; };
		Vector_4(const T& x, const T& y, const T& z, const T& w) : _Vector<T>(x, y) { this->z = z; this->w = w; }
		Vector_4(const std::initializer_list<T>& t);

		// Преобразования типов
		explicit operator Vector_2<T>()const;
		explicit operator Vector_3<T>()const;

		// Перегруженные операторы
		// присваивание
		Vector_4& operator=(const std::initializer_list<T>& l); // присваивание списка инициализации
		Vector_4& operator=(const Vector_4<T>& v); // присвоение копии
		Vector_4& operator=(const T& v); // присвоение числа

		// умножение на скаляр 
		Vector_4& operator*=(const T& sc);
		Vector_4 operator*(const T& sc);
		template<class T1>
		friend Vector_4<T1>operator*(const T1& sc, Vector_4<T1>& v);

		// умножение скалярное 
		T operator*(const Vector_4<T>& v)const;

		// сравнение
		bool operator==(const Vector_4<T>& v)const;
		bool operator!=(const Vector_4<T>& v);
		bool operator<(const T& v);
		bool operator>(const T& v);
		bool operator<(const Vector_4<T>& v)const;
		bool operator>(const Vector_4<T>& v)const;
		bool operator<=(const T& v);
		bool operator>=(const T& v);

		// прибавление скаляра
		Vector_4& operator+=(const T& sc);
		Vector_4 operator+(const T& sc)const;
		template<class T1>
		friend Vector_4<T1>operator+(const T1& sc, const Vector_4<T1>& v);

		// вычитание скаляра 
		Vector_4& operator-=(const T& sc);
		Vector_4 operator-(const T& sc)const;

		// сложение векторов
		Vector_4& operator+=(const Vector_4<T>& v);
		Vector_4 operator+(const Vector_4<T>& v);

		// вычитание векторов
		Vector_4& operator-=(const Vector_4<T>& v);
		Vector_4 operator-(const Vector_4<T>& v)const;

		// инверсия вектора
		template<class T1>
		friend Vector_4<T1>operator~(const Vector_4<T1>& v);

		// доступ к координате
		T& operator[](const gmtypes::VectorCoordinate& i);
		T operator[](const gmtypes::VectorCoordinate& i)const;

		// умножение на матрицу
		Vector_4& operator*=(const Matrix_4<T>& m);
		Vector_4 operator*(const Matrix_4<T>& m);

		// инкремент/декремент
		Vector_4 operator++(int);
		Vector_4& operator++();
		Vector_4 operator--(int);
		Vector_4& operator--();

		// унарные операторы
		template<class T1>
		friend const Vector_4<T1>operator-(const Vector_4<T1>& v);

		// методы
		void normalize();
		void inverse();
		void identity();
		std::string c_str();
		std::wstring c_wstr();
		T length();
		std::vector<T> toArray();

		// основные
		static float Length(const Vector_4<T>& v);
		static Vector_4 Inverse(const Vector_4<T>& v);
		static Vector_4 Identity();
		static Vector_4 Normalize(const Vector_4<T>& v);

		// вектор по двум точкам
		static Vector_4 createVectorForDoublePoints(Vector_4<T>& v1, Vector_4<T>& v2);

		// скалярное произведение
		T dot(const Vector_4<T>& v);
		static T dot(Vector_4<T>& v1, Vector_4<T>& v2);
		static T dot(const Vector_4<T>& v1, const Vector_4<T>& v2);

		// векторное произведение
		Vector_4& cross(const Vector_4<T>& v);
		static Vector_4 cross(const Vector_4<T>& v1, const Vector_4<T>& v2);

		// точка между двумя
		Vector_4 pointOfLeft(const Vector_4<T> v, int n = 1);
		Vector_4 pointOfRight(const Vector_4<T> v, int n = 1);

		// преобразование
		void vector4TransformCoord(const Matrix_4<T>& m);
		Vector_4 vector4TransformCoord(const Vector_4<T>& v, const Matrix_4<T>& m);
		void vector4TransformNormal(const Matrix_4<T>& m);
		Vector_4 vector4TransformNormal(const Vector_4<T>& v, const Matrix_4<T>& m);

	};

	// конструкторы

	template<class T>
	inline Vector_4<T>::Vector_4(const std::initializer_list<T>& t) : _Vector<T>(t)
	{
		T* adr[] = { &this->z, &this->w };
		int i(0);
		for (auto it(t.begin() + 2); it != t.end(); it++, i++)
			*adr[i] = *it;
	}

	// преобразование типов

	template<class T>
	inline Vector_4<T>::operator Vector_2<T>()const
	{
		return Vector_2<T>(this->x, this->y);
	}

	template<class T>
	inline Vector_4<T>::operator Vector_3<T>()const
	{
		return Vector_3<T>(this->x, this->y, this->z);
	}

	// перегруженные операторы
	// оператор присваивания

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator=(const std::initializer_list<T>& l)
	{
		this->_initializate(l);
		T* adr[] = { &this->z, &this->w }; int i(0);
		for (auto it(l.begin() + 2); it != l.end(); it++, i++)
			*adr[i] = *it;
		return *this;
	}

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator=(const Vector_4<T>& v)
	{
		if (this != &v)
		{
			this->_copy(v.x, v.y);
			this->z = v.z;
			this->w = v.w;
		}
		return *this;
	}

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator=(const T & v)
	{
		if (this->x != v || this->y != v || this->z != v || this->w != v)
			this->x = this->y = this->z = this->w = v;
		return *this;
	}

	// умножение на скаляр

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator*=(const T & sc)
	{
		this->x *= sc;
		this->y *= sc;
		this->z *= sc;
		this->w *= sc;
		return *this;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::operator*(const T & sc)
	{
		Vector_4<T> v(*this);
		v.x *= sc;
		v.y *= sc;
		v.z *= sc;
		v.w *= sc;
		return v;
	}

	template<class T1>
	inline Vector_4<T1>operator*(const T1 & sc, Vector_4<T1>& v)
	{
		return v * sc;
	}

	// умножение скалярное

	template<class T>
	inline T Vector_4<T>::operator*(const Vector_4<T>& v)const
	{
		return (this->x * v.x + this->y * v.y + this->z * v.z + this->w * v.w);
	}

	// сравнение

	template<class T>
	inline bool Vector_4<T>::operator==(const Vector_4 <T>& v)const
	{
		return this->x == v.x && this->y == v.y && this->z == v.z && this->w == v.w;
	}

	template<class T>
	inline bool Vector_4<T>::operator!=(const Vector_4<T>& v)
	{
		return !(*this == v);
	}

	template<class T>
	inline bool Vector_4<T>::operator<(const T & v)
	{
		return this->x < v && this->y < v && this->z < v && this->w < v;
	}

	template<class T>
	inline bool Vector_4<T>::operator>(const T & v)
	{
		return this->x > v && this->y > v && this->z > v && this->w > v;
	}

	template<class T>
	inline bool Vector_4<T>::operator<(const Vector_4<T>& v)const
	{
		// w
		if (this->w < v.w)
		{
			return true;
		}
		else if (this->w > v.w)
		{
			return false;
		}
		// z trumps, then y, then x
		if (this->z < v.z)
		{
			return true;
		}
		else if (this->z > v.z)
		{
			return false;
		}
		// Otherwise z is equal
		if (this->y < v.y)
		{
			return true;
		}
		else if (this->y > v.y)
		{
			return false;
		}
		// Otherwise z and y are equal
		if (this->x < v.x)
		{
			return true;
		}
		/* Simple optimization Do not need this test
		   If this fails or succeeded the result is false.
		else if( this->x > v.x )
		{    return false;
		}*/
		// Otherwise z and y and x are all equal
		return false;
	}

	template<class T>
	inline bool Vector_4<T>::operator>(const Vector_4<T>& v)const
	{
		// w
		if (this->w > v.w)
		{
			return true;
		}
		else if (this->w < v.w)
		{
			return false;
		}
		// z trumps, then y, then x
		if (this->z > v.z)
		{
			return true;
		}
		else if (this->z < v.z)
		{
			return false;
		}
		// Otherwise z is equal
		if (this->y > v.y)
		{
			return true;
		}
		else if (this->y < v.y)
		{
			return false;
		}
		// Otherwise z and y are equal
		if (this->x > v.x)
		{
			return true;
		}
		/* Simple optimization Do not need this test
		   If this fails or succeeded the result is false.
		else if( this->x > v.x )
		{    return false;
		}*/
		// Otherwise z and y and x are all equal
		return false;
	}

	template<class T>
	inline bool Vector_4<T>::operator<=(const T & v)
	{
		return this->x <= v && this->y <= v && this->z <= v && this->w <= v;
	}

	template<class T>
	inline bool Vector_4<T>::operator>=(const T & v)
	{
		return this->x >= v && this->y >= v && this->z >= v && this->w >= v;
	}

	// Прибавление скаляра

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator+=(const T & sc)
	{
		this->x += sc;
		this->y += sc;
		this->z += sc;
		this->w += sc;
		return *this;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::operator+(const T & sc)const
	{
		Vector_4<T> vec(*this);
		vec.x += sc;
		vec.y += sc;
		vec.z += sc;
		vec.w += sc;
		return vec;
	}

	template<class T1>
	inline Vector_4<T1>operator+(const T1 & sc, const Vector_4<T1>& v)
	{
		return v + sc;
	}

	// вычитание скаляра

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator-=(const T & sc)
	{
		this->x -= sc;
		this->y -= sc;
		this->z -= sc;
		this->w -= sc;
		return *this;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::operator-(const T & sc)const
	{
		Vector_4<T> vec(*this);
		vec.x -= sc;
		vec.y -= sc;
		vec.z -= sc;
		vec.w -= sc;
		return vec;
	}

	// сложение векторов

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator+=(const Vector_4<T>& v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		this->w += v.w;
		return *this;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::operator+(const Vector_4<T>& v)
	{
		Vector_4<T> vec(*this);
		vec.x += v.x;
		vec.y += v.y;
		vec.z += v.z;
		vec.w += v.w;
		return vec;
	}

	// вычитание векторов

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator-=(const Vector_4<T>& v)
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.x;
		this->w -= v.w;
		return *this;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::operator-(const Vector_4<T>& v)const
	{
		Vector_4<T> vec(*this);
		vec.x -= v.x;
		vec.y -= v.y;
		vec.z -= v.z;
		vec.w -= v.w;
		return vec;
	}

	// инверсия вектора

	template<class T1>
	inline Vector_4<T1>operator~(const Vector_4<T1>& v)
	{
		return Vector_4<T1>(-v.x, -v.y, -v.z, -v.w);
	}

	// доступ к координате

	template<class T>
	inline T & Vector_4<T>::operator[](const gmtypes::VectorCoordinate & i)
	{
		switch (i)
		{
		case  gmtypes::x:
			return this->x;

		case  gmtypes::y:
			return this->y;

		case  gmtypes::z:
			return this->z;

		case  gmtypes::w:
			return this->w;

		default:
#ifdef UNICODE
			throw gmexception::VectorIndexException(L"Error index for Vector4!");
#else
			throw gmexception::VectorIndexException("Error index for Vector4!");
#endif
		}
	}

	template<class T>
	inline T Vector_4<T>::operator[](const gmtypes::VectorCoordinate & i)const
	{
		switch (i)
		{
		case  gmtypes::x:
			return this->x;

		case  gmtypes::y:
			return this->y;

		case  gmtypes::z:
			return this->z;

		case  gmtypes::w:
			return this->w;

		default:
#ifdef UNICODE
			throw gmexception::VectorIndexException(L"Error index for Vector4!");
#else
			throw gmexception::VectorIndexException("Error index for Vector4!");
#endif
		}
	}

	// умножение на матрицу
	template<class T>
	inline Vector_4<T> & gmath::Vector_4<T>::operator*=(const Matrix_4<T>& m)
	{
		this->vector4TransformCoord(m);
		return *this;
	}

	template<class T>
	inline Vector_4<T> gmath::Vector_4<T>::operator*(const Matrix_4<T>& m)
	{
		Vector_4<T> v(*this); v.vector4TransformCoord(m);
		return v;
	}

	// инкремент/декремент

	template<class T>
	inline Vector_4<T> Vector_4<T>::operator++(int)
	{
		Vector_4<T> v(*this);
		this->x++;
		this->y++;
		this->z++;
		this->w++;
		return v;
	}

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator++()
	{
		this->x++;
		this->y++;
		this->z++;
		this->w++;
		return *this;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::operator--(int)
	{
		Vector_4<T> v(*this);
		this->x--;
		this->y--;
		this->z--;
		this->w--;
		return v;
	}

	template<class T>
	inline Vector_4<T> & Vector_4<T>::operator--()
	{
		this->x--;
		this->y--;
		this->z--;
		this->w--;
		return *this;
	}

	// унарные операторы
	template<class T1>
	const Vector_4<T1>operator-(const Vector_4<T1>& v)
	{
		Vector_4<T1>_v(v);
		_v.inverse();
		return _v;
	}

	// методы
	// абстрактные методы

	template<class T>
	inline T Vector_4<T>::length()
	{
		return sqrtf(powf(this->x, 2) + powf(this->y, 2) + powf(this->z, 2) + powf(this->w, 2));
	}

	template<class T>
	inline void Vector_4<T>::normalize()
	{
		float invLength = 1 / this->length();
		this->x *= invLength;
		this->y *= invLength;
		this->z *= invLength;
		this->w *= invLength;
	}

	template<class T>
	inline void Vector_4<T>::inverse()
	{
		*this = ~(*this);
	}

	template<class T>
	inline void Vector_4<T>::identity()
	{
		this->x = 1;
		this->y = 1;
		this->z = 1;
		this->w = 1;
	}

	template<class T>
	inline std::string gmath::Vector_4<T>::c_str()
	{
		std::stringstream buf;
		buf << "[" << std::setprecision(std::to_string((int)this->x).length() + 3) << this->x << ", " << std::setprecision(std::to_string((int)this->y).length() + 3) << this->y <<
			", " << std::setprecision(std::to_string((int)this->z).length() + 3) << this->z << ", " << std::setprecision(std::to_string((int)this->w).length() + 3) << this->w << "]";
		return buf.str();
	}

	template<class T>
	inline std::wstring gmath::Vector_4<T>::c_wstr()
	{
		std::wstringstream buf;
		buf << L"[" << std::setprecision(std::to_wstring((int)this->x).length() + 3) << this->x << L", " << std::setprecision(std::to_wstring((int)this->y).length() + 3) << this->y <<
			L", " << std::setprecision(std::to_wstring((int)this->z).length() + 3) << this->z << L", " << std::setprecision(std::to_wstring((int)this->w).length() + 3) << this->w << L"]";
		return buf.str();
	}

	// основные
	template<class T>
	inline float gmath::Vector_4<T>::Length(const Vector_4<T>& v)
	{
		return v.length();
	}

	template<class T>
	inline Vector_4<T> gmath::Vector_4<T>::Inverse(const Vector_4<T>& v)
	{
		Vector_4<T> _v(v); _v.inverse();
		return _v;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::Identity()
	{
		Vector_4<float> v;
		v.identity();
		return v;
	}

	template<class T>
	inline Vector_4<T> Vector_4<T>::Normalize(const Vector_4<T>& v)
	{
		Vector_4<T> vec(v);
		vec.normalize();
		return vec;
	}

	// вектор по двум точкам

	template<class T>
	inline Vector_4<T> Vector_4<T>::createVectorForDoublePoints(Vector_4<T>& v1, Vector_4<T>& v2)
	{
		return v2 - v1;
	}

	// скалярное произведение

	template<class T>
	inline T Vector_4<T>::dot(const Vector_4<T>& v)
	{
		return *this * v;
	}

	template<class T>
	inline T Vector_4<T>::dot(Vector_4<T>& v1, Vector_4<T>& v2)
	{
		return v1 * v2;
	}

	template<class T>
	inline T Vector_4<T>::dot(const Vector_4<T>& v1, const Vector_4<T>& v2)
	{
		return v1 * v2;
	}

	// векторное произведение
	template<class T>
	inline Vector_4<T> & gmath::Vector_4<T>::cross(const Vector_4<T>& v)
	{
		Vector_4<T> res;
		res.x = this->y * v.z - this->z * v.y;
		res.y = this->z * v.x - this->x * v.z;
		res.z = this->x * v.y - this->y * v.x;
		res.w = 1;
		*this = res;
		return *this;
	}

	template<class T>
	inline Vector_4<T> gmath::Vector_4<T>::cross(const Vector_4<T>& v1, const Vector_4<T>& v2)
	{
		Vector_4<T> res(v1);
		res.cross(v2);
		return res;
	}

	// точка между двумя другими

	template<class T>
	inline Vector_4<T> Vector_4<T>::pointOfLeft(const Vector_4<T> v, int n)
	{
		if (n <= 0)
			n = 1;

		Vector_4<T> v1(*this);
		Vector_4<T> v2(v);
		Vector_4<T> result;
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
	inline Vector_4<T> Vector_4<T>::pointOfRight(const Vector_4<T> v, int n)
	{
		if (n <= 0)
			n = 1;

		Vector_4<T> v1(*this);
		Vector_4<T> v2(v);
		Vector_4<T> result;
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
	inline void gmath::Vector_4<T>::vector4TransformCoord(const Matrix_4<T>& m)
	{
		/*Vector_4<T> old(*this);
		this->x = old.x * m(0, 0) + old.y * m(0, 1) + old.z * m(0, 2) + m(3, 0);
		this->y = old.x * m(1, 0) + old.y * m(1, 1) + old.z * m(1, 2) + m(3, 1);
		this->z = old.x * m(2, 0) + old.y * m(2, 1) + old.z * m(2, 2) + m(3, 2);
		this->w = old.x * m(3, 0) + old.y * m(3, 1) + old.z * m(3, 2) + m(3, 3);
		this->w = 1.f / this->w;
		this->x *= w;
		this->y *= w;
		this->z *= w;*/

		float norm = m(0, 3) * this->x + m(1, 3) * this->y + this->z * m(2, 3) +  m(3, 3);
		Vector_4<T> tmp(*this);
		if (norm)
		{
			this->x = (m(0, 0) * tmp.x + m(1, 0) * tmp.y + tmp.z *m(2, 0) + m(3, 0)) / norm;
			this->y = (m(0, 1) * tmp.x + m(1, 1) * tmp.y + tmp.z *m(2, 1) + m(3, 1)) / norm;
			this->z = (m(0, 2) * tmp.x + m(1, 2) * tmp.y + tmp.z * m(2, 2) + m(3, 2)) / norm;
			this->w = (m(0, 3) * tmp.x + m(1, 3) * tmp.y + tmp.z * m(2, 3) + m(3, 3)) / norm;
		}
		else
			this->x = this->y = this->z = this->w = 0.f;
	}

	template<class T>
	inline Vector_4<T> gmath::Vector_4<T>::vector4TransformCoord(const Vector_4<T>& v, const Matrix_4<T>& m)
	{
		Vector_4<T> _v(v); _v.vector4TransformCoord(m);
		return _v;
	}

	template<class T>
	inline void gmath::Vector_4<T>::vector4TransformNormal(const Matrix_4<T>& m)
	{
		Vector_4<T> tmp(*this);
		this->x = m(0, 0) * tmp.x + m(1, 0) * tmp.y + tmp.z *m(2, 0) + tmp.w * m(3, 0);
		this->y = m(0, 1) * tmp.x + m(1, 1) * tmp.y + tmp.z *m(2, 1) + tmp.w * m(3, 1);
		this->z = m(0, 2) * tmp.x + m(1, 2) * tmp.y + tmp.z * m(2, 2) + tmp.w *m(3, 2);
		this->w = m(0, 3) * tmp.x + m(1, 3) * tmp.y + tmp.z * m(2, 3) + tmp.w *m(3, 3);
	}

	template<class T>
	inline Vector_4<T> gmath::Vector_4<T>::vector4TransformNormal(const Vector_4<T>& v, const Matrix_4<T>& m)
	{
		Vector_4<T> _v(v); _v.vector4TransformNormal(m);
		return _v;
	}

	// возвращаем массив
	template<class T>
	inline std::vector<T> gmath::Vector_4<T>::toArray()
	{
		return { this->x,  this->y, this->z, this->w };
	}

}