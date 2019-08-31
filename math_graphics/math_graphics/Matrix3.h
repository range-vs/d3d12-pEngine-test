#pragma once

#include "mathematics_typedefs_matrix.h"

namespace gmath
{
	template<class T>
	class Matrix_3 : public _Matrix<T, 3>
	{
	public:
		// конструкторы
		Matrix_3() :_Matrix<T, 3>() {}
		Matrix_3(const T& e) :_Matrix<T, 3>(e) {};
		Matrix_3(const Matrix_3<T>& m) :_Matrix<T, 3>(m) {};
		Matrix_3(const std::initializer_list<T>& l) :_Matrix<T, 3>(l) {};
		Matrix_3(const std::initializer_list<Vector_3<T>>& l, bool line = true);
		Matrix_3(T* arr, size_t s) :_Matrix<T, 3>(arr, s) {};

		// преобразование типов
		explicit operator Matrix_2<T>()const;
		explicit operator Matrix_4<T>()const;

		// перегруженные операторы
		// оператор присваивания
		Matrix_3& operator=(const Matrix_3<T>& m); // присвоение копии
		Matrix_3& operator=(const std::initializer_list<T>& l); // присвоение списка
		Matrix_3& operator=(const T& m); // присвоение числа

		// сложение матриц
		Matrix_3& operator+=(const Matrix_3<T>& m);
		Matrix_3 operator+(const Matrix_3<T>& m);

		// вычитание матриц
		Matrix_3& operator-=(const Matrix_3<T>& m);
		Matrix_3 operator-(const Matrix_3<T>& m);

		// добавление числа
		Matrix_3& operator+=(const T& v);
		Matrix_3 operator+(const T& v);
		template<class T1>
		friend Matrix_3<T1>operator+(const T1& v, Matrix_3<T1>& m);

		// вычитание числа
		Matrix_3& operator-=(const T& v);
		Matrix_3 operator-(const T& v);

		// умножение матриц
		Matrix_3& operator*=(const Matrix_3<T>& m);
		Matrix_3 operator*(const Matrix_3<T>& m);

		// умножение на число
		Matrix_3& operator*=(const T& m);
		Matrix_3 operator*(const T& m);
		template<class T1>
		friend Matrix_3<T1>operator*(const T1& v, const Matrix_3<T1>& m);

		// инверсия
		template<class T1>
		friend Matrix_3<T1>operator~(const Matrix_3<T1>& m);

		// доступ к элементу
		Vector_3<T> operator[](const int& i)const;
		const T& operator()(const int& i, const int& j)const;
		T& operator()(const int& i, const int& j);

		// инкремент/декремент
		Matrix_3 operator++(int);
		Matrix_3& operator++();
		Matrix_3 operator--(int);
		Matrix_3& operator--();


		// методы
		// абстрактные
		void identity();
		void transponse();
		void inverse();
		float determinant();
		void reverse();
		std::string c_str();
		std::wstring c_wstr();


		// статические аналоги абстрактных методов
		static Matrix_3 Identity();
		static Matrix_3 Transponse(const Matrix_3<T>& m);
		static Matrix_3 Inverse(const Matrix_3<T>& m);
		static Matrix_3 Reverse(const Matrix_3<T>& m);
		static float Determinant(const Matrix_3<T>& m);

		// матричные преобразования, применимо к 2D
		void createRotateMatrixX(const float& x); // вращение по х
		void createRotateMatrixY(const float& y); // вращение по у
		void createRotateMatrixZ(const float& z); // вращение по z
		void createRotateMatrixXYZ(const float& x, const float& y, const float& z); // вращение по двум осям
		void createTranslationMatrixX(const float& x); // перемещение по х
		void createTranslationMatrixY(const float& y); // перемещение по у
		void createTranslationMatrixXY(const float& x, const float& y); // перемещение по двум осям
		void createScaleMatrixX(const float& x); // масштаб по х
		void createScaleMatrixY(const float& y); // масштаб по у
		void createScaleMatrixXY(const float& x, const float& y); // масштаб по двум осям

		// статические, аналоги матричных преобразований, применимо к 2D
		static Matrix_3 CreateRotateMatrixX(const float& x); // вращение по х
		static Matrix_3 CreateRotateMatrixY(const float& y); // вращение по у
		static Matrix_3 CreateRotateMatrixZ(const float& z); // вращение по z
		static Matrix_3 CreateRotateMatrixXYZ(const float& x, const float& y, const float& z); // вращение по двум осям
		static Matrix_3 CreateTranslationMatrixX(const float& x); // перемещение по х
		static Matrix_3 CreateTranslationMatrixY(const float& y); // перемещение по у
		static Matrix_3 CreateTranslationMatrixXY(const float& x, const float& y); // перемещение по двум осям
		static Matrix_3 CreateScaleMatrixX(const float& x); // масштаб по х
		static Matrix_3 CreateScaleMatrixY(const float& y); // масштаб по у
		static Matrix_3 CreateScaleMatrixXY(const float& x, const float& y); // масштаб по двум осям

	};

	// конструкторы

	template<class T>
	Matrix_3<T>::Matrix_3(const std::initializer_list<Vector_3<T>>& l, bool line)
	{
		int i(0);
		for (auto & v : l)
		{
			if (i < this->size)
			{
				if (line)
				{
					this->matrix[i][0] = v[gmtypes::x];
					this->matrix[i][1] = v[gmtypes::y];
					this->matrix[i][2] = v[gmtypes::z];
				}
				else
				{
					this->matrix[0][i] = v[gmtypes::x];
					this->matrix[1][i] = v[gmtypes::y];
					this->matrix[2][i] = v[gmtypes::z];
				}
			}
			i++;
		}
	}

	// преобразование типов
	template<class T>
	inline gmath::Matrix_3<T>::operator Matrix_2<T>()const
	{
		Matrix_2<T> m = { this->matrix[0][0], this->matrix[0][1], this->matrix[1][0], this->matrix[1][1]};
		return m;
	}

	template<class T>
	inline gmath::Matrix_3<T>::operator Matrix_4<T>() const
	{
		Matrix_4<T> m = { this->matrix[0][0], this->matrix[0][1], this->matrix[0][2], 0,
			this->matrix[1][0], this->matrix[1][1], this->matrix[1][2], 0,
			this->matrix[2][0], this->matrix[2][1], this->matrix[2][2], 0,
			0, 0, 0, 1 };
		return m;
	}

	// перегруженные операторы
	// оператор присваивания

	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator=(const Matrix_3<T>& m)
	{
		if (this != &m)
		{
			for (int i(0); i < 3; i++)
				std::copy(m.matrix[i], m.matrix[i] + 3, this->matrix[i]);
		}
		return *this;
	}

	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator=(const std::initializer_list<T>& l)
	{
		auto iter = l.begin();
		for (auto& l : this->matrix)
		{
			for (auto & e : l)
				if (iter != l.end())
					e = *(iter++);
		}
		return *this;
	}

	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator=(const T & m)
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
	inline Matrix_3<T> & Matrix_3<T>::operator+=(const Matrix_3<T>& m)
	{
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				this->matrix[i][j] += m.matrix[i][j];
		return *this;
	};

	template<class T>
	inline Matrix_3<T> Matrix_3<T>::operator+(const Matrix_3<T>& m)
	{
		Matrix_3<T> mat(*this);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				mat.matrix[i][j] += m.matrix[i][j];
		return mat;
	}

	// вычитание матриц
	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator-=(const Matrix_3<T>& m)
	{
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				this->matrix[i][j] -= m.matrix[i][j];
		return *this;
	};

	template<class T>
	inline Matrix_3<T> Matrix_3<T>::operator-(const Matrix_3<T>& m)
	{
		Matrix_3<T> mat(*this);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				mat.matrix[i][j] -= m.matrix[i][j];
		return mat;
	}

	// добавление числа
	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator+=(const T& v)
	{
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				this->matrix[i][j] += v;
		return *this;
	};

	template<class T>
	inline Matrix_3<T> Matrix_3<T>::operator+(const T& v)
	{
		Matrix_3<T> mat(*this);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				mat.matrix[i][j] += v;
		return mat;
	}

	template<class T1>
	Matrix_3<T1>operator+(const T1 & v, Matrix_3<T1>& m)
	{
		Matrix_3<T1>mat(v);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				mat.matrix[i][j] += v;
		return mat;
	}

	// вычитание числа
	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator-=(const T& v)
	{
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				this->matrix[i][j] -= v;
		return *this;
	};

	template<class T>
	inline Matrix_3<T> Matrix_3<T>::operator-(const T& v)
	{
		Matrix_3<T> mat(*this);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				mat.matrix[i][j] -= v;
		return mat;
	}

	// умножение матриц
	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator*=(const Matrix_3<T>& m)
	{
		Matrix_3<T> old(*this);
		this->matrix[0][0] = old.matrix[0][0] * m.matrix[0][0] + old.matrix[0][1] * m.matrix[1][0] + old.matrix[0][2] * m.matrix[2][0];
		this->matrix[0][1] = old.matrix[0][0] * m.matrix[0][1] + old.matrix[0][1] * m.matrix[1][1] + old.matrix[0][2] * m.matrix[2][1];
		this->matrix[0][2] = old.matrix[0][0] * m.matrix[0][2] + old.matrix[0][1] * m.matrix[1][2] + old.matrix[0][2] * m.matrix[2][2];

		this->matrix[1][0] = old.matrix[1][0] * m.matrix[0][0] + old.matrix[1][1] * m.matrix[1][0] + old.matrix[1][2] * m.matrix[2][0];
		this->matrix[1][1] = old.matrix[1][0] * m.matrix[0][1] + old.matrix[1][1] * m.matrix[1][1] + old.matrix[1][2] * m.matrix[2][1];
		this->matrix[1][2] = old.matrix[1][0] * m.matrix[0][2] + old.matrix[1][1] * m.matrix[1][2] + old.matrix[1][2] * m.matrix[2][2];

		this->matrix[2][0] = old.matrix[2][0] * m.matrix[0][0] + old.matrix[2][1] * m.matrix[1][0] + old.matrix[2][2] * m.matrix[2][0];
		this->matrix[2][1] = old.matrix[2][0] * m.matrix[0][1] + old.matrix[2][1] * m.matrix[1][1] + old.matrix[2][2] * m.matrix[2][1];
		this->matrix[2][2] = old.matrix[2][0] * m.matrix[0][2] + old.matrix[2][1] * m.matrix[1][2] + old.matrix[2][2] * m.matrix[2][2];

		return *this;
	}

	template<class T>
	inline Matrix_3<T>  Matrix_3<T>::operator*(const Matrix_3<T>& m)
	{
		Matrix_3<T> mat(*this);
		return mat *= m;
	}

	// умножение на число

	template<class T>
	inline Matrix_3<T> & Matrix_3<T>::operator*=(const T & m)
	{
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				this->matrix[i][j] *= m;
		return *this;
	}

	template<class T>
	inline Matrix_3<T>  Matrix_3<T>::operator*(const T & m)
	{
		Matrix_3<T> mat(*this);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				mat.matrix[i][j] *= m;
		return mat;
	}

	template<class T1>
	Matrix_3<T1>operator*(const T1 & v, const Matrix_3<T1>& m)
	{
		Matrix_3<T1>mat(m);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				mat.matrix[i][j] *= v;
		return mat;
	}

	template<class T1>
	inline Matrix_3<T1> operator~(const Matrix_3<T1>& m)
	{
		Matrix_3<T1> res(m);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				res.matrix[i][j] = -res.matrix[i][j];
		return res;
	}

	// доступ к элементу
	template<class T>
	inline Vector_3<T> Matrix_3<T>::operator[](const int & i)const
	{
		if (i < 0 || i >= 3)
#ifdef _UNICODE
			throw gmexception::MatrixIndexLineException(L"Error line for Matrix3!");
#else
			throw gmexception::MatrixIndexLineException("Error line for Matrix3!");
#endif
		return Vector_3<T>(this->matrix[i][0], this->matrix[i][1], this->matrix[i][2]);
	}

	template<class T>
	inline const T & Matrix_3<T>::operator()(const int & i, const int &j) const
	{
		if (i < 0 || i >= 3)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index line for Matrix3!");
#else
			throw gmexception::MatrixIndexElementException("Error index line for Matrix3!");
#endif
		else if (j < 0 || j >= 3)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index column for Matrix3!");
#else
			throw gmexception::MatrixIndexElementException("Error index column for Matrix3!");
#endif
		else
			return this->matrix[i][j];
	}

	template<class T>
	inline T & Matrix_3<T>::operator()(const int & i, const int & j)
	{
		if (i < 0 || i >= 3)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index line for Matrix3!");
#else
			throw gmexception::MatrixIndexElementException("Error index line for Matrix3!");
#endif
		else if (j < 0 || j >= 3)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index column for Matrix3!");
#else
			throw gmexception::MatrixIndexElementException("Error index column for Matrix3!");
#endif
		return this->matrix[i][j];
	}

	//  инкремент / декремент
	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::operator++(int)
	{
		Matrix_3<T> m(*this);
		for (auto& l : this->matrix)
			for (auto& e : l)
				e++;
		return m;
	}

	template<class T>
	inline Matrix_3<T> & gmath::Matrix_3<T>::operator++()
	{
		for (auto& l : this->matrix)
			for (auto& e : l)
				e++;
		return *this;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::operator--(int)
	{
		Matrix_3<T> m(*this);
		for (auto& l : this->matrix)
			for (auto& e : l)
				e--;
		return m;
	}

	template<class T>
	inline Matrix_3<T> & gmath::Matrix_3<T>::operator--()
	{
		for (auto& l : this->matrix)
			for (auto& e : l)
				e--;
		return *this;
	}

	// методы
	// абстрактные

	template<class T>
	inline void Matrix_3<T>::identity()
	{
		this->matrix[0][0] = this->matrix[1][1] = this->matrix[2][2] = 1;
		this->matrix[0][1] = this->matrix[0][2] = this->matrix[1][2] = this->matrix[1][3] = this->matrix[2][0] = this->matrix[2][1] = 0;
	}

	template<class T>
	inline void Matrix_3<T>::transponse()
	{
		Matrix_3<T> res(*this);
		for (int i(0); i < 3; i++)
			for (int j(0); j < 3; j++)
				res.matrix[i][j] = this->matrix[j][i];
		*this = res;
	}

	template<class T>
	inline void Matrix_3<T>::inverse()
	{
		// детерминант(определитель)
		float determ(this->determinant());
		if (determ == 0)
#ifdef _UNICODE
			throw gmexception::MatrixDeterminantException(L"Determinant matrix is by zero!");
#else
			throw gmexception::MatrixDeterminantException("Determinant matrix is by zero!");
#endif

		// матрица миноров
		Matrix_3<T> out;
		int _i(0);
		int _j(0);
		T array[4];
		T* ptr_array(array);
		int iterat(9);
		while (iterat-- > 0)
		{
			for (int i(0); i < 3; i++)
				for (int j(0); j < 3; j++)
					if (i != _i && j != _j)
						*ptr_array++ = this->matrix[i][j];
			Matrix_2<T> det(array, 4);
			int d = det.determinant();
			if (d == 0)
				out.matrix[_i][_j] = 0;
			else
				out.matrix[_i][_j] = d;
			_j == 2 ? 0, _j = 0, ++_i : ++_j;
			ptr_array = array;
		}

		// матрица алгебраических дополнений
		out(0, 1) = -out(0, 1);
		out(1, 0) = -out(1, 0);
		out(1, 2) = -out(1, 2);
		out(2, 1) = -out(2, 1);

		out.transponse(); // транспонирование матрицы алгебраических дополнений
		*this = (1.f / determ) * out; // находим обратную матрицу
	}

	template<class T>
	inline float Matrix_3<T>::determinant()
	{
		return (this->matrix[0][0] * this->matrix[1][1] * this->matrix[2][2] + this->matrix[1][2] * this->matrix[0][1] * this->matrix[2][0] +
			this->matrix[0][2] * this->matrix[1][0] * this->matrix[2][1] - this->matrix[0][2] * this->matrix[1][1] * this->matrix[2][0] -
			this->matrix[0][0] * this->matrix[1][2] * this->matrix[2][1] - this->matrix[0][1] * this->matrix[1][0] * this->matrix[2][2]);
	}

	template<class T>
	inline void Matrix_3<T>::reverse()
	{
		*this = ~(*this);
	}

	template<class T>
	inline std::string gmath::Matrix_3<T>::c_str()
	{
		float _max(0);
		for (auto& l : this->matrix)
		{
			T* tmp = std::max_element(l, l + 3, [](const T& t1, const T& t2)
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
				if (++i != 3)
					buf << "   ";
			}
			i = 0;
			buf << " ]" << std::endl;
		}
		return buf.str();
	}

	template<class T>
	inline std::wstring gmath::Matrix_3<T>::c_wstr()
	{
		float _max(0);
		for (auto& l : this->matrix)
		{
			T* tmp = std::max_element(l, l + 3, [](const T& t1, const T& t2)
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
				if (++i != 3)
					buf << L"   ";
			}
			i = 0;
			buf << L" ]" << std::endl;
		}
		return buf.str();
	}

	// аналоги абстрактных методов
	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::Identity()
	{
		Matrix_3<T> m;
		m.identity();
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::Transponse(const Matrix_3<T>& m)
	{
		Matrix_3<T> m1(m);
		m1.transponse();
		return m1;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::Inverse(const Matrix_3<T>& m)
	{
		Matrix_3<T> m1(m);
		m1.inverse();
		return m1;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::Reverse(const Matrix_3<T>& m)
	{
		Matrix_3<T> m1(m);
		m1.reverse();
		return m1;
	}

	template<class T>
	inline float gmath::Matrix_3<T>::Determinant(const Matrix_3<T>& m)
	{
		return m.determinant();
	}

	// матричные преобразования, применимо к 2D

	template<class T>
	inline void gmath::Matrix_3<T>::createRotateMatrixX(const float & x)
	{
		// по часовой стрелке
		this->identity();
		this->matrix[1][1] = cos(x);
		this->matrix[1][2] = -sin(x);
		this->matrix[2][1] = sin(x);
		this->matrix[2][2] = cos(x);
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createRotateMatrixY(const float & y)
	{
		// по часовой стрелке
		this->identity();
		this->matrix[0][0] = cos(y);
		this->matrix[0][2] = sin(y);
		this->matrix[2][0] = -sin(y);
		this->matrix[2][2] = cos(y);
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createRotateMatrixZ(const float & z)
	{
		// по часовой стрелке
		this->identity();
		this->matrix[0][0] = cos(z);
		this->matrix[0][1] = -sin(z);
		this->matrix[1][0] = sin(z);
		this->matrix[1][1] = cos(z);
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createRotateMatrixXYZ(const float & x, const float & y, const float & z)
	{
		Matrix_3<T> rotx; rotx.createRotateMatrixX();
		Matrix_3<T> roty; roty.createRotateMatrixY();
		Matrix_3<T> rotz; rotz.createRotateMatrixZ();
		*this = rotx * roty * rotz;
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createTranslationMatrixX(const float & x)
	{
		this->identity();
		this->matrix[2][0] = x;
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createTranslationMatrixY(const float & y)
	{
		this->identity();
		this->matrix[2][1] = y;
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createTranslationMatrixXY(const float & x, const float & y)
	{
		this->identity();
		this->matrix[2][0] = x;
		this->matrix[2][1] = y;
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createScaleMatrixX(const float & x)
	{
		this->identity();
		this->matrix[0][0] = x;
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createScaleMatrixY(const float & y)
	{
		this->identity();
		this->matrix[1][1] = y;
	}

	template<class T>
	inline void gmath::Matrix_3<T>::createScaleMatrixXY(const float & x, const float & y)
	{
		this->identity();
		this->matrix[0][0] = x;
		this->matrix[1][1] = y;
	}

	// статические, аналоги матричных преобразований, применимо к 2D
	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateRotateMatrixX(const float & x)
	{
		Matrix_3<T> m;
		m.createRotateMatrixX(x);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateRotateMatrixY(const float & y)
	{
		Matrix_3<T> m;
		m.createRotateMatrixY(y);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateRotateMatrixZ(const float & z)
	{
		Matrix_3<T> m;
		m.createRotateMatrixZ(z);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateRotateMatrixXYZ(const float & x, const float & y, const float & z)
	{
		Matrix_3<T> m;
		m.createRotateMatrixXYZ(x, y, z);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateTranslationMatrixX(const float & x)
	{
		Matrix_3<T> m;
		m.createTranslationMatrixX(x);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateTranslationMatrixY(const float & y)
	{
		Matrix_3<T> m;
		m.createTranslationMatrixY(y);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateTranslationMatrixXY(const float & x, const float & y)
	{
		Matrix_3<T> m;
		m.createTranslationMatrixXY(x, y);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateScaleMatrixX(const float & x)
	{
		Matrix_3<T> m;
		m.createScaleMatrixX(x);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateScaleMatrixY(const float & y)
	{
		Matrix_3<T> m;
		m.createScaleMatrixY(y);
		return m;
	}

	template<class T>
	inline Matrix_3<T> gmath::Matrix_3<T>::CreateScaleMatrixXY(const float & x, const float & y)
	{
		Matrix_3<T> m;
		m.createScaleMatrixXY(x, y);
		return m;
	}

}