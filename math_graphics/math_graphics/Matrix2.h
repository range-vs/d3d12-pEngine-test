#pragma once

#include "mathematics_typedefs_matrix.h"

namespace gmath
{
	template<class T>
	class Matrix_2 : public _Matrix<T, 2>
	{
	public:
		// конструкторы
		Matrix_2() :_Matrix<T, 2>() {}
		Matrix_2(const T& e) :_Matrix<T, 2>(e) {};
		Matrix_2(const Matrix_2<T>& m) :_Matrix<T, 2>(m) {};
		Matrix_2(const std::initializer_list<T>& l) :_Matrix<T, 2>(l) {};
		Matrix_2(const std::initializer_list<Vector_3<T>>& l, bool line = true);
		Matrix_2(T* arr, size_t s) :_Matrix<T, 2>(arr, s) {};

		// преобразование типов
		explicit operator Matrix_3<T>()const;
		explicit operator Matrix_4<T>()const;

		// перегруженные операторы
		// оператор присваивания
		Matrix_2& operator=(const Matrix_2<T>& m); // присвоение копии
		Matrix_2& operator=(const std::initializer_list<T>& l); // присвоение списка
		Matrix_2& operator=(const T& m); // присвоение числа

		// сложение матриц
		Matrix_2& operator+=(const Matrix_2<T>& m);
		Matrix_2 operator+(const Matrix_2<T>& m);

		// вычитание матриц
		Matrix_2& operator-=(const Matrix_2<T>& m);
		Matrix_2 operator-(const Matrix_2<T>& m);

		// добавление числа
		Matrix_2& operator+=(const T& v);
		Matrix_2 operator+(const T& v);
		template<class T1>
		friend Matrix_2<T1>operator+(const T1& v, Matrix_2<T1>& m);

		// вычитание числа
		Matrix_2& operator-=(const T& v);
		Matrix_2 operator-(const T& v);

		// умножение матриц
		Matrix_2& operator*=(const Matrix_2<T>& m);
		Matrix_2 operator*(const Matrix_2<T>& m);

		// умножение на число
		Matrix_2& operator*=(const T& m);
		Matrix_2 operator*(const T& m);
		template<class T1>
		friend Matrix_2<T1>operator*(const T1& v, const Matrix_2<T1>& m);

		// инверсия
		template<class T1>
		friend Matrix_2<T1>operator~(const Matrix_2<T1>& m);

		// доступ к элементу
		Vector_2<T> operator[](const int& i)const;
		const T& operator()(const int& i, const int& j)const;
		T& operator()(const int& i, const int& j);

		// инкремент/декремент
		Matrix_2 operator++(int);
		Matrix_2& operator++();
		Matrix_2 operator--(int);
		Matrix_2& operator--();
	  
		// методы
		void identity();
		void transponse();
		void inverse();
		float determinant();
		void reverse();
		std::string c_str();
		std::wstring c_wstr();
		std::vector<T> toArray();
		std::vector<std::vector<T>> toMatrixArray();


		// методы статические
		Matrix_2 Identity();
		Matrix_2 Transponse(const Matrix_2<T>& m);
		Matrix_2 Inverse(const Matrix_2<T>& m);
		Matrix_2 Reverse(const Matrix_2<T>& m);
		float Determinant(const Matrix_2<T>& m);
	};

	// конструкторы

	template<class T>
	Matrix_2<T>::Matrix_2(const std::initializer_list<Vector_3<T>>& l, bool line)
	{
		int i(0);
		for (auto it = l.begin(); it != l.end(); it++)
		{
			Vector_3<T> v = *it;
			if (i < this->size)
			{
				if (line)
				{
					this->matrix[i][0] = v[gmtypes::x];
					this->matrix[i][1] = v[gmtypes::y];
				}
				else
				{
					this->matrix[0][i] = v[gmtypes::x];
					this->matrix[1][i] = v[gmtypes::y];
				}
			}
			i++;
		}
	}

	// преобразование типов
	template<class T>
	inline gmath::Matrix_2<T>::operator Matrix_3<T>()const
	{
		Matrix_3<T> m = { this->matrix[0][0], this->matrix[0][1], 0, this->matrix[1][0], this->matrix[1][1], 0,
			0, 0, 1 };
		return m;
	}

	template<class T>
	inline gmath::Matrix_2<T>::operator Matrix_4<T>() const
	{
		Matrix_4<T> m = { this->matrix[0][0], this->matrix[0][1], 0, 0, this->matrix[1][0], this->matrix[1][1], 0, 0,
			0, 0, 1, 0, 0, 0, 0, 1 };
		return m;
	}

	// перегруженные операторы
	// оператор присваивания

	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator=(const Matrix_2<T>& m)
	{
		if (this != &m)
		{
			for (int i(0); i < 2; i++)
				std::copy(m.matrix[i], m.matrix[i] + 2, this->matrix[i]);
		}
		return *this;
	}

	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator=(const std::initializer_list<T>& l)
	{
		auto iter = l.begin();
		for (auto& l1 : this->matrix)
		{
			for (auto & e : l1)
				if (iter != l.end())
					e = *(iter++);
		}
		return *this;
	}

	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator=(const T & m)
	{
		for (auto& l : this->matrix)
		{
			for (auto & e : l)
				e = m;
		}
		return *this;
	}

	// сложение матриц
	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator+=(const Matrix_2<T>& m)
	{
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				this->matrix[i][j] += m.matrix[i][j];
		return *this;
	};

	template<class T>
	inline Matrix_2<T> Matrix_2<T>::operator+(const Matrix_2<T>& m)
	{
		Matrix_2<T> mat(*this);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				mat.matrix[i][j] += m.matrix[i][j];
		return mat;
	}

	// вычитание матриц
	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator-=(const Matrix_2<T>& m)
	{
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				this->matrix[i][j] -= m.matrix[i][j];
		return *this;
	};

	template<class T>
	inline Matrix_2<T> Matrix_2<T>::operator-(const Matrix_2<T>& m)
	{
		Matrix_2<T> mat(*this);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				mat.matrix[i][j] -= m.matrix[i][j];
		return mat;
	}

	// добавление числа
	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator+=(const T& v)
	{
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				this->matrix[i][j] += v;
		return *this;
	};

	template<class T>
	inline Matrix_2<T> Matrix_2<T>::operator+(const T& v)
	{
		Matrix_2<T> mat(*this);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				mat.matrix[i][j] += v;
		return mat;
	}

	template<class T1>
	inline Matrix_2<T1>operator+(const T1 & v, Matrix_2<T1>& m)
	{
		Matrix_2<T1>mat(v);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				mat.matrix[i][j] += v;
		return mat;
	}

	// вычитание числа
	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator-=(const T& v)
	{
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				this->matrix[i][j] -= v;
		return *this;
	};

	template<class T>
	inline Matrix_2<T> Matrix_2<T>::operator-(const T& v)
	{
		Matrix_2<T> mat(*this);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				mat.matrix[i][j] -= v;
		return mat;
	}

	// умножение матриц
	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator*=(const Matrix_2<T>& m)
	{
		Matrix_2<T> old(*this);
		this->matrix[0][0] = old.matrix[0][0] * m.matrix[0][0] + old.matrix[0][1] * m.matrix[1][0];
		this->matrix[0][1] = old.matrix[0][0] * m.matrix[0][1] + old.matrix[0][1] * m.matrix[1][1];
		this->matrix[1][0] = old.matrix[1][0] * m.matrix[0][0] + old.matrix[1][1] * m.matrix[1][0];
		this->matrix[1][1] = old.matrix[1][0] * m.matrix[0][1] + old.matrix[1][1] * m.matrix[1][1];
		return *this;
	}

	template<class T>
	inline Matrix_2<T>  Matrix_2<T>::operator*(const Matrix_2<T>& m)
	{
		Matrix_2<T> mat(*this);
		return mat *= m;
	}

	// умножение на число

	template<class T>
	inline Matrix_2<T> & Matrix_2<T>::operator*=(const T & m)
	{
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				this->matrix[i][j] *= m;
		return *this;
	}

	template<class T>
	inline Matrix_2<T>  Matrix_2<T>::operator*(const T & m)
	{
		Matrix_2<T> mat(*this);
		return mat *= m;
	}

	template<class T1>
	inline Matrix_2<T1>operator*(const T1 & v, const Matrix_2<T1>& m)
	{
		Matrix_2<T1>mat(m);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				mat.matrix[i][j] *= v;
		return mat;
	}

	// инверсия

	template<class T1>
	Matrix_2<T1>operator~(const Matrix_2<T1>& m)
	{
		Matrix_2<T1>res(m);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				res.matrix[i][j] = -res.matrix[i][j];
		return res;
	}

	// доступ к элементу
	template<class T>
	inline Vector_2<T> Matrix_2<T>::operator[](const int & i)const
	{
		if (i < 0 || i >= 2)
#ifdef _UNICODE
			throw gmexception::MatrixIndexLineException(L"Error line for Matrix2!");
#else
			throw gmexception::MatrixIndexLineException("Error line for Matrix2!");
#endif
		return Vector_2<T>(this->matrix[i][0], this->matrix[i][1]);
	}

	template<class T>
	inline const T & Matrix_2<T>::operator()(const int & i, const int &j) const
	{
		if (i < 0 || i >= 2)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index line for Matrix2!");
#else
			throw gmexception::MatrixIndexElementException("Error index line for Matrix2!");
#endif
		else if (j < 0 || j >= 2)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index column for Matrix2!");
#else
			throw gmexception::MatrixIndexElementException("Error index column for Matrix2!");
#endif
		else
			return this->matrix[i][j];
	}

	template<class T>
	inline T & Matrix_2<T>::operator()(const int & i, const int & j)
	{
		if (i < 0 || i >= 2)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index line for Matrix2!");
#else
			throw gmexception::MatrixIndexElementException("Error index line for Matrix2!");
#endif
		else if (j < 0 || j >= 2)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index column for Matrix2!");
#else
			throw gmexception::MatrixIndexElementException("Error index column for Matrix2!");
#endif
		return this->matrix[i][j];
	}

	//  инкремент / декремент
	template<class T>
	inline Matrix_2<T> gmath::Matrix_2<T>::operator++(int)
	{
		Matrix_2<T> m(*this);
		for (auto& l : this->matrix)
			for (auto& e : l)
				e++;
		return m;
	}

	template<class T>
	inline Matrix_2<T> & gmath::Matrix_2<T>::operator++()
	{
		for (auto& l : this->matrix)
			for (auto& e : l)
				e++;
		return *this;
	}

	template<class T>
	inline Matrix_2<T> gmath::Matrix_2<T>::operator--(int)
	{
		Matrix_2<T> m(*this);
		for (auto& l : this->matrix)
			for (auto& e : l)
				e--;
		return m;
	}

	template<class T>
	inline Matrix_2<T> & gmath::Matrix_2<T>::operator--()
	{
		for (auto& l : this->matrix)
			for (auto& e : l)
				e--;
		return *this;
	}

	// методы
	// абстрактные

	template<class T>
	inline void Matrix_2<T>::identity()
	{
		this->matrix[0][0] = this->matrix[1][1] = 1;
		this->matrix[0][1] = this->matrix[1][0] = 0;
	}

	template<class T>
	inline void Matrix_2<T>::transponse()
	{
		Matrix_2<T> res(*this);
		for (int i(0); i < 2; i++)
			for (int j(0); j < 2; j++)
				res.matrix[i][j] = this->matrix[j][i];
		*this = res;
	}

	template<class T>
	inline void Matrix_2<T>::inverse()
	{
		float d(this->determinant());
		if (d == 0)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Determinant matrix is by zero!");
#else
			throw gmexception::MatrixIndexElementException("Determinant matrix is by zero!");
#endif

		Matrix_2<T> minor = { this->matrix[1][1], this->matrix[1][0], this->matrix[0][1], this->matrix[0][0] }; // матрица миноров
		minor(0, 1) = -minor(0, 1); // матрица алгебраических дополнений
		minor(1, 0) = -minor(1, 0);
		minor.transponse(); // транспонирование матрицы алгебраических дополнений
		*this = (1.f / d) * minor; // находим обратную матрицу
	}

	template<class T>
	inline float Matrix_2<T>::determinant()
	{
		return (this->matrix[0][0] * this->matrix[1][1]) - (this->matrix[0][1] * this->matrix[1][0]);
	}

	template<class T>
	inline void Matrix_2<T>::reverse()
	{
		*this = ~(*this);
	}

	template<class T>
	inline std::string gmath::Matrix_2<T>::c_str()
	{
		float _max(0);
		for (auto& l : this->matrix)
		{
			T* tmp = std::max_element(l, l + 2, [](const T& t1, const T& t2)
			{
				return std::to_string((int)t1).length() < std::to_string((int)t2).length();
			});
			if (std::to_string((int)*tmp).length() > std::to_string((int)_max).length())
				_max = *tmp;
		}
		_max = std::to_string((int)_max).length() + 4;
		int i(0);

		std::stringstream buf;
		for (auto& line : this->matrix)
		{
			buf << "[ ";
			for (auto& e : line)
			{
				buf << std::right << std::setw(_max) << std::setprecision(3) << std::setiosflags(std::ios::fixed | std::ios::showpoint) << e;
				if (++i != 2)
					buf << "   ";
			}
			i = 0;
			buf << " ]" << std::endl;
		}
		return buf.str();
	}

	template<class T>
	inline std::wstring gmath::Matrix_2<T>::c_wstr()
	{
		float _max(0);
		for (auto& l : this->matrix)
		{
			T* tmp = std::max_element(l, l + 2, [](const T& t1, const T& t2)
			{
				return std::to_string((int)t1).length() < std::to_string((int)t2).length();
			});
			if (std::to_string((int)*tmp).length() > std::to_string((int)_max).length())
				_max = *tmp;
		}
		_max = std::to_string((int)_max).length() + 4;
		int i(0);

		std::wstringstream buf;
		for (auto& line : this->matrix)
		{
			buf << L"[ ";
			for (auto& e : line)
			{
				buf << std::right << std::setw(_max) << std::setprecision(3) << std::setiosflags(std::ios::fixed | std::ios::showpoint) << e;
				if (++i != 2)
					buf << L"   ";
			}
			i = 0;
			buf << L" ]" << std::endl;
		}
		return buf.str();
	}

	template<class T>
	inline std::vector<T> gmath::Matrix_2<T>::toArray()
	{
		return std::vector<T>
		{
			this->matrix[0][0], this->matrix[0][1], 
			this->matrix[1][0], this->matrix[1][1] 
		};
	}

	template<class T>
	inline std::vector<std::vector<T>> gmath::Matrix_2<T>::toMatrixArray()
	{
		return std::vector<std::vector<T>>
		{
			{ this->matrix[0][0], this->matrix[0][1]},
			{ this->matrix[1][0], this->matrix[1][1] }
		};
	}

	// методы статические

	template<class T>
	inline Matrix_2<T> gmath::Matrix_2<T>::Identity()
	{
		Matrix_2<T> m;
		m.identity();
		return m;
	}

	template<class T>
	inline Matrix_2<T> gmath::Matrix_2<T>::Transponse(const Matrix_2<T>& m)
	{
		Matrix_2<T> m1(m);
		m1.transponse();
		return m1;
	}

	template<class T>
	inline Matrix_2<T> gmath::Matrix_2<T>::Inverse(const Matrix_2<T>& m)
	{
		Matrix_2<T> m1(m);
		m1.inverse();
		return m1;
	}

	template<class T>
	inline Matrix_2<T> gmath::Matrix_2<T>::Reverse(const Matrix_2<T>& m)
	{
		Matrix_2<T> m1(m);
		m1.reverse();
		return m1;
	}

	template<class T>
	inline float gmath::Matrix_2<T>::Determinant(const Matrix_2<T>& m)
	{
		return m.determinant();
	}

} 