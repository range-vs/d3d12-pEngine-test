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
		// ������������
		Vector_3() :_Vector<T>() { this->z = 0; }
		Vector_3(const T& v) :_Vector<T>(v) { this->z = v; }
		Vector_3(const Vector_3& v) :_Vector<T>(v) { this->z = v.z; };
		Vector_3(const T& x, const T& y, const T& z) : _Vector<T>(x, y) { this->z = z; }
		Vector_3(const std::initializer_list<T>& t);

		// �������������� �����
		explicit operator Vector_2<T>()const;
		explicit operator Vector_4<T>()const;

		// ������������� ���������
		// ������������
		Vector_3& operator=(const std::initializer_list<T>& l); // ������������ ������ �������������
		Vector_3& operator=(const Vector_3<T>& v); // ���������� �����
		Vector_3& operator=(const T& v); // ���������� �����

		// ��������� �� ������ 
		Vector_3& operator*=(const T& sc);
		Vector_3 operator*(const T& sc)const;
		Vector_3 operator*(const T& sc);
		template<class T1>
		friend Vector_3<T1>operator*(const T1& sc, const Vector_3<T1>& v);

		// ��������� ��������� 
		T operator*(const Vector_3<T>& v)const;

		// ���������
		bool operator==(const Vector_3<T>& v)const;
		bool operator!=(const Vector_3<T>& v);
		bool operator<(const T& v);
		bool operator>(const T& v);
		bool operator<=(const T& v);
		bool operator>=(const T& v);

		// ����������� �������
		Vector_3& operator+=(const T& sc);
		Vector_3 operator+(const T& sc)const;
		template<class T1>
		friend Vector_3<T1>operator+(const T1& sc, const Vector_3<T1>& v);

		// ��������� ������� 
		Vector_3& operator-=(const T& sc);
		Vector_3 operator-(const T& sc)const;

		// �������� ��������
		Vector_3& operator+=(const Vector_3<T>& v);
		Vector_3 operator+(const Vector_3<T>& v)const;

		// ��������� ��������
		Vector_3& operator-=(const Vector_3<T>& v);
		Vector_3 operator-(const Vector_3<T>& v)const;

		// �������� �������
		template<class T1>
		friend Vector_3<T1>operator~(const Vector_3<T1>& v);

		// ������ � ����������
		T& operator[](const gmtypes::VectorCoordinate& i);
		T operator[](const gmtypes::VectorCoordinate& i)const;

		// ��������� �� �������
		Vector_3& operator*=(const Matrix_4<T>& m);
		Vector_3 operator*(const Matrix_4<T>& m);

		// ���������/���������
		Vector_3 operator++(int);
		Vector_3& operator++();
		Vector_3 operator--(int);
		Vector_3& operator--();

		// ������� ���������
		template<class T1>
		friend const Vector_3<T1>operator-(const Vector_3<T1>& v);

		// ������
		// ����������� ������
		float length();
		void normalize();
		void inverse();
		void identity();
		std::string c_str();
		std::wstring c_wstr();

		// ��������
		static float Length(const Vector_3<T>& v);
		static Vector_3 Normalize(const Vector_3<T>& v);
		static Vector_3 Inverse(const Vector_3<T>& v);
		static Vector_3 Identity();

		// ������ �� ���� ������
		static Vector_3 createVectorForDoublePoints(Vector_3<T>& v1, Vector_3<T>& v2);

		// ��������� ������������
		T dot(const Vector_3<T>& v);
		static T dot(const Vector_3<T>& v1, const Vector_3<T>& v2);

		// ��������� ������������
		Vector_3& cross(const Vector_3<T>& v);
		static Vector_3 cross(const Vector_3<T>& v1, const Vector_3<T>& v2);

		// ����� ����� �����
		Vector_3 pointOfLeft(const Vector_3<T> v, int n = 1);
		Vector_3 pointOfRight(const Vector_3<T> v, int n = 1);

		// ��������������
		void vector3TransformCoord(const Matrix_4<T>& m);
		Vector_3 vector3TransformCoord(const Vector_3<T>& v, const Matrix_4<T>& m);
		void vector3TransformNormal(const Matrix_4<T>& m);
		Vector_3 vector3TransformNormal(const Vector_3<T>& v, const Matrix_4<T>& m);

		// ������� �������
		void toArray(T* arr);
	};

	// ������������

	template<class T>
	inline Vector_3<T>::Vector_3(const std::initializer_list<T>& t) : _Vector<T>(t)
	{
		if (t.size() >= 3)
			this->z = *(t.end() - 1);
	}

	// �������������� �����

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

	// ������������� ���������
	// �������� ������������

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

	// ��������� �� ������

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

	// ��������� ���������

	template<class T>
	inline T Vector_3<T>::operator*(const Vector_3<T>& v)const
	{
		return (this->x * v.x + this->y * v.y + this->z * v.z);
	}

	// ���������

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

	// ����������� �������

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

	// ��������� �������

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

	// �������� ��������

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

	// ��������� ��������

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

	// �������� �������

	template<class T1>
	inline Vector_3<T1>operator~(const Vector_3<T1>& v)
	{
		return Vector_3<T1>(-v.x, -v.y, -v.z);
	}

	// ������ � ����������

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

	// ��������� �� �������
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

	// ���������/���������

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

	// ������� ���������
	template<class T1>
	const Vector_3<T1>operator-(const Vector_3<T1>& v)
	{
		Vector_3<T1>_v(v); _v.inverse();
		return _v;
	}

	// ������
	// ����������� ������

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

	// ��������
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

	// ������ �� ���� ������

	template<class T>
	inline Vector_3<T> Vector_3<T>::createVectorForDoublePoints(Vector_3<T>& v1, Vector_3<T>& v2)
	{
		return v2 - v1;
	}

	// ��������� ������������

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

	// ��������� ������������
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

	// ����� ����� ����� �������

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

	// ��������������
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

	// ���������� ������
	template<class T>
	inline void gmath::Vector_3<T>::toArray(T* arr)
	{
		arr[0] = this->x;
		arr[1] = this->y;
		arr[2] = this->z;
	}
}