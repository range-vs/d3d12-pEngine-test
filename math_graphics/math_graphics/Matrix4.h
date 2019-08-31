#pragma once

#include "mathematics_typedefs_matrix.h"

namespace gmath
{
	template<class T>
	class Matrix_4 : public _Matrix<T, 4>
	{
	public:
		// конструкторы
		Matrix_4() :_Matrix<T, 4>() {}
		Matrix_4(const T& e) :_Matrix<T, 4>(e) {};
		Matrix_4(const Matrix_4<T>& m) :_Matrix<T, 4>(m) {};
		Matrix_4(const std::initializer_list<T>& l) :_Matrix<T, 4>(l) {};
		Matrix_4(const std::initializer_list<Vector_4<T>>& l, bool line = true);
		Matrix_4(T* arr, size_t s) :_Matrix<T, 4>(arr, s) {};

		// преобразование типов
		explicit operator Matrix_2<T>()const;
		explicit operator Matrix_3<T>()const;

		// перегруженные операторы
		// оператор присваивания
		Matrix_4& operator=(const Matrix_4<T>& m); // присвоение копии
		Matrix_4& operator=(const std::initializer_list<T>& l); // присвоение списка
		Matrix_4& operator=(const T& m); // присвоение числа

		// сложение матриц
		Matrix_4& operator+=(const Matrix_4<T>& m);
		Matrix_4 operator+(const Matrix_4<T>& m);

		// вычитание матриц
		Matrix_4& operator-=(const Matrix_4<T>& m);
		Matrix_4 operator-(const Matrix_4<T>& m);

		// добавление числа
		Matrix_4& operator+=(const T& v);
		Matrix_4 operator+(const T& v);
		template<class T1>
		friend Matrix_4<T1>operator+(const T1& v, Matrix_4<T1>& m);

		// вычитание числа
		Matrix_4& operator-=(const T& v);
		Matrix_4 operator-(const T& v);

		// умножение матриц
		Matrix_4& operator*=(const Matrix_4<T>& m);
		Matrix_4 operator*(const Matrix_4<T>& m)const;

		// умножение на число
		Matrix_4& operator*=(const T& m);
		Matrix_4 operator*(const T& m);
		template<class T1>
		friend Matrix_4<T1>operator*(const T1& v, const Matrix_4<T1>& m);

		// инверсия
		template<class T1>
		friend Matrix_4<T1>operator~(const Matrix_4<T1>& m);

		// доступ к элементу
		Vector_4<T> operator[](const int& i)const;
		const T& operator()(const int& i, const int& j)const;
		T& operator()(const int& i, const int& j);

		// инкремент/декремент
		Matrix_4 operator++(int);
		Matrix_4& operator++();
		Matrix_4 operator--(int);
		Matrix_4& operator--();

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
		static Matrix_4 Identity();
		static Matrix_4 Transponse(const Matrix_4<T>& m);
		static Matrix_4 Inverse(const Matrix_4<T>& m);
		static Matrix_4 Reverse(const Matrix_4<T>& m);
		static float Determinant(const Matrix_4<T>& m);

		// матричные преобразования, применимо к 3D
		void createRotateMatrixX(const float& x); // вращение по х
		void createRotateMatrixY(const float& y); // вращение по у
		void createRotateMatrixZ(const float& z); // вращение по z
		void createRotateMatrixXYZ(const float& x, const float& y, const float& z); // вращение по трем осям
		void createRotateMatrix(const float& ang); // вращение по всем осям
		void createTranslationMatrixX(const float& x); // перемещение по х
		void createTranslationMatrixY(const float& y); // перемещение по у
		void createTranslationMatrixZ(const float& y); // перемещение по z
		void createTranslationMatrixXYZ(const float& x, const float& y, const float& z); // перемещение по трем осям
		void createTranslationMatrix(const float& ang); // перемещение по всем осям
		void createScaleMatrixX(const float& x); // масштаб по х
		void createScaleMatrixY(const float& y); // масштаб по у
		void createScaleMatrixZ(const float& z); // масштаб по z
		void createScaleMatrixXYZ(const float& x, const float& y, const float& z); // масштаб по трем осям
		void createScaleMatrix(const float& ang); // масштаб по всем осям
		// матрицы вида, левая
		void createLookAtLHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp);
		void createLookAtLHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp);
		// правая
		void createLookAtRHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp);
		void createLookAtRHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp);
		// матрицы перспективной проекции, левая
		void createPerspectiveFovLHMatrix(const float& fovy, const float& aspect, const float& zn, const float& zf);
		// правая
		void createPerspectiveFovRHMatrix(const float& fovy, const float& aspect, const float& zn, const float& zf);
		// матрица порта просмотра
		void createViewportMatrix(float x, float y, float w, float h, float minz, float maxz);


		// статические, аналоги матричных преобразований, применимо к 3D
		static Matrix_4 CreateRotateMatrixX(const float& x); // вращение по х
		static Matrix_4 CreateRotateMatrixY(const float& y); // вращение по у
		static Matrix_4 CreateRotateMatrixZ(const float& z); // вращение по z
		static Matrix_4 CreateRotateMatrixXYZ(const float& x, const float& y, const float& z); // вращение по двум осям
		static Matrix_4 CreateRotateMatrix(const float&  ang); // вращение по всем осям
		static Matrix_4 CreateTranslationMatrixX(const float& x); // перемещение по х
		static Matrix_4 CreateTranslationMatrixY(const float& y); // перемещение по у
		static Matrix_4 CreateTranslationMatrixZ(const float& y); // перемещение по z
		static Matrix_4 CreateTranslationMatrixXYZ(const float& x, const float& y, const float& z); // перемещение по трем осям
		static Matrix_4 CreateTranslationMatrix(const float& ang); // перемещение по всем осям
		static Matrix_4 CreateScaleMatrixX(const float& x); // масштаб по х
		static Matrix_4 CreateScaleMatrixY(const float& y); // масштаб по у
		static Matrix_4 CreateScaleMatrixZ(const float& z); // масштаб по z
		static Matrix_4 CreateScaleMatrixXYZ(const float& x, const float& y, const float& z); // масштаб по трем осям
		static Matrix_4 CreateScaleMatrix(const float& ang); // масштаб по всем осям
		// матрицы вида, левая
		static Matrix_4 CreateLookAtLHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp);
		static Matrix_4 CreateLookAtLHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp);
		// правая
		static Matrix_4 CreateLookAtRHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp);
		static Matrix_4 CreateLookAtRHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp);
		// матрицы перспективной проекции, левая
		static Matrix_4 CreatePerspectiveFovLHMatrix(const float& fovy, const float& aspect, const float& zn, const float& zf);
		// правая
		static Matrix_4 CreatePerspectiveFovRHMatrix(const float& fovy, const float& aspect, const float& zn, const float& zf);
		// матрица порта просмотра
		static Matrix_4 CreateViewportMatrix(float x, float y, float w, float h, float minz, float maxz);

	};

	// конструкторы

	template<class T>
	Matrix_4<T>::Matrix_4(const std::initializer_list<Vector_4<T>>& l, bool line)
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
					this->matrix[i][3] = v[gmtypes::w];
				}
				else
				{
					this->matrix[0][i] = v[gmtypes::x];
					this->matrix[1][i] = v[gmtypes::y];
					this->matrix[2][i] = v[gmtypes::z];
					this->matrix[3][i] = v[gmtypes::w];
				}
			}
			i++;
		}
	}

	// преобразование типов
	template<class T>
	inline gmath::Matrix_4<T>::operator Matrix_2<T>()const
	{
		Matrix_2<T> m = { this->matrix[0][0], this->matrix[0][1], this->matrix[1][0], this->matrix[1][1] };
		return m;
	}

	template<class T>
	inline gmath::Matrix_4<T>::operator Matrix_3<T>() const
	{
		Matrix_3<T> m = { this->matrix[0][0], this->matrix[0][1], this->matrix[0][2],
			this->matrix[1][0], this->matrix[1][1], this->matrix[1][2],
			this->matrix[2][0], this->matrix[2][1], this->matrix[2][2]};
		return m;
	}

	// перегруженные операторы
	// оператор присваивания

	template<class T>
	inline Matrix_4<T> & Matrix_4<T>::operator=(const Matrix_4<T>& m)
	{
		if (this != &m)
		{
			for (int i(0); i < 4; i++)
				std::copy(m.matrix[i], m.matrix[i] + 4, this->matrix[i]);
		}
		return *this;
	}

	template<class T>
	inline Matrix_4<T> & Matrix_4<T>::operator=(const std::initializer_list<T>& l)
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
	inline Matrix_4<T> & Matrix_4<T>::operator=(const T & m)
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
	inline Matrix_4<T> & Matrix_4<T>::operator+=(const Matrix_4<T>& m)
	{
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				this->matrix[i][j] += m.matrix[i][j];
		return *this;
	};

	template<class T>
	inline Matrix_4<T> Matrix_4<T>::operator+(const Matrix_4<T>& m)
	{
		Matrix_4<T> mat(*this);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				mat.matrix[i][j] += m.matrix[i][j];
		return mat;
	}

	// вычитание матриц
	template<class T>
	inline Matrix_4<T> & Matrix_4<T>::operator-=(const Matrix_4<T>& m)
	{
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				this->matrix[i][j] -= m.matrix[i][j];
		return *this;
	};

	template<class T>
	inline Matrix_4<T> Matrix_4<T>::operator-(const Matrix_4<T>& m)
	{
		Matrix_4<T> mat(*this);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				mat.matrix[i][j] -= m.matrix[i][j];
		return mat;
	}

	// добавление числа
	template<class T>
	inline Matrix_4<T> & Matrix_4<T>::operator+=(const T& v)
	{
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				this->matrix[i][j] += v;
		return *this;
	};

	template<class T>
	inline Matrix_4<T> Matrix_4<T>::operator+(const T& v)
	{
		Matrix_4<T> mat(*this);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				mat.matrix[i][j] += v;
		return mat;
	}

	template<class T1>
	inline Matrix_4<T1>operator+(const T1 & v, Matrix_4<T1>& m)
	{
		Matrix_4<T1>mat(v);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				mat.matrix[i][j] += v;
		return mat;
	}

	// вычитание числа
	template<class T>
	inline Matrix_4<T> & Matrix_4<T>::operator-=(const T& v)
	{
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				this->matrix[i][j] -= v;
		return *this;
	};

	template<class T>
	inline Matrix_4<T> Matrix_4<T>::operator-(const T& v)
	{
		Matrix_4<T> mat(*this);
		for (int i(0); i < 4; i++)
			for (int j(0); j <4; j++)
				mat.matrix[i][j] -= v;
		return mat;
	}

	// умножение матриц
	template<class T>
	inline Matrix_4<T> & Matrix_4<T>::operator*=(const Matrix_4<T>& m)
	{
		Matrix_4<T> old(*this);
		this->matrix[0][0] = old.matrix[0][0] * m.matrix[0][0] + old.matrix[0][1] * m.matrix[1][0] + old.matrix[0][2] * m.matrix[2][0] + old.matrix[0][3] * m.matrix[3][0];
		this->matrix[0][1] = old.matrix[0][0] * m.matrix[0][1] + old.matrix[0][1] * m.matrix[1][1] + old.matrix[0][2] * m.matrix[2][1] + old.matrix[0][3] * m.matrix[3][1];
		this->matrix[0][2] = old.matrix[0][0] * m.matrix[0][2] + old.matrix[0][1] * m.matrix[1][2] + old.matrix[0][2] * m.matrix[2][2] + old.matrix[0][3] * m.matrix[3][2];
		this->matrix[0][3] = old.matrix[0][0] * m.matrix[0][3] + old.matrix[0][1] * m.matrix[1][3] + old.matrix[0][2] * m.matrix[2][3] + old.matrix[0][3] * m.matrix[3][3];

		this->matrix[1][0] = old.matrix[1][0] * m.matrix[0][0] + old.matrix[1][1] * m.matrix[1][0] + old.matrix[1][2] * m.matrix[2][0] + old.matrix[1][3] * m.matrix[3][0];
		this->matrix[1][1] = old.matrix[1][0] * m.matrix[0][1] + old.matrix[1][1] * m.matrix[1][1] + old.matrix[1][2] * m.matrix[2][1] + old.matrix[1][3] * m.matrix[3][1];
		this->matrix[1][2] = old.matrix[1][0] * m.matrix[0][2] + old.matrix[1][1] * m.matrix[1][2] + old.matrix[1][2] * m.matrix[2][2] + old.matrix[1][3] * m.matrix[3][2];
		this->matrix[1][3] = old.matrix[1][0] * m.matrix[0][3] + old.matrix[1][1] * m.matrix[1][3] + old.matrix[1][2] * m.matrix[2][3] + old.matrix[1][3] * m.matrix[3][3];

		this->matrix[2][0] = old.matrix[2][0] * m.matrix[0][0] + old.matrix[2][1] * m.matrix[1][0] + old.matrix[2][2] * m.matrix[2][0] + old.matrix[2][3] * m.matrix[3][0];
		this->matrix[2][1] = old.matrix[2][0] * m.matrix[0][1] + old.matrix[2][1] * m.matrix[1][1] + old.matrix[2][2] * m.matrix[2][1] + old.matrix[2][3] * m.matrix[3][1];
		this->matrix[2][2] = old.matrix[2][0] * m.matrix[0][2] + old.matrix[2][1] * m.matrix[1][2] + old.matrix[2][2] * m.matrix[2][2] + old.matrix[2][3] * m.matrix[3][2];
		this->matrix[2][3] = old.matrix[2][0] * m.matrix[0][3] + old.matrix[2][1] * m.matrix[1][3] + old.matrix[2][2] * m.matrix[2][3] + old.matrix[2][3] * m.matrix[3][3];

		this->matrix[3][0] = old.matrix[3][0] * m.matrix[0][0] + old.matrix[3][1] * m.matrix[1][0] + old.matrix[3][2] * m.matrix[2][0] + old.matrix[3][3] * m.matrix[3][0];
		this->matrix[3][1] = old.matrix[3][0] * m.matrix[0][1] + old.matrix[3][1] * m.matrix[1][1] + old.matrix[3][2] * m.matrix[2][1] + old.matrix[3][3] * m.matrix[3][1];
		this->matrix[3][2] = old.matrix[3][0] * m.matrix[0][2] + old.matrix[3][1] * m.matrix[1][2] + old.matrix[3][2] * m.matrix[2][2] + old.matrix[3][3] * m.matrix[3][2];
		this->matrix[3][3] = old.matrix[3][0] * m.matrix[0][3] + old.matrix[3][1] * m.matrix[1][3] + old.matrix[3][2] * m.matrix[2][3] + old.matrix[3][3] * m.matrix[3][3];

		return *this;
	}

	template<class T>
	inline Matrix_4<T>  Matrix_4<T>::operator*(const Matrix_4<T>& m)const
	{
		Matrix_4<T> mat(*this);
		mat *= m;
		return mat;
	}

	// умножение на число

	template<class T>
	inline Matrix_4<T> & Matrix_4<T>::operator*=(const T & m)
	{
		for (int i(0); i <4; i++)
			for (int j(0); j < 4; j++)
				this->matrix[i][j] *= m;
		return *this;
	}

	template<class T>
	inline Matrix_4<T>  Matrix_4<T>::operator*(const T & m)
	{
		Matrix_4<T> mat(*this);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				mat.matrix[i][j] *= m;
		return mat;
	}

	template<class T1>
	inline Matrix_4<T1>operator*(const T1 & v, const Matrix_4<T1>& m)
	{
		Matrix_4<T1>mat(m);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				mat.matrix[i][j] *= v;
		return mat;
	}

	template<class T1>
	inline Matrix_4<T1>operator~(const Matrix_4<T1>& m)
	{
		Matrix_4<T1>res(m);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				res.matrix[i][j] = -res.matrix[i][j];
		return res;
	}

	// доступ к элементу
	template<class T>
	inline Vector_4<T> Matrix_4<T>::operator[](const int & i)const
	{
		if (i < 0 || i >= 4)
#ifdef _UNICODE
			throw gmexception::MatrixIndexLineException(L"Error line for Matrix4!");
#else
			throw gmexception::MatrixIndexLineException("Error line for Matrix4!");
#endif
		return Vector_4<T>(this->matrix[i][0], this->matrix[i][1], this->matrix[i][2], this->matrix[i][3]);
	}

	template<class T>
	inline const T & Matrix_4<T>::operator()(const int & i, const int &j) const
	{
		if (i < 0 || i >= 4)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index line for Matrix4!");
#else
			throw gmexception::MatrixIndexElementException("Error index line for Matrix4!");
#endif
		else if (j < 0 || j >= 4)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index column for Matrix4!");
#else
			throw gmexception::MatrixIndexElementException("Error index column for Matrix4!");
#endif
		else
			return this->matrix[i][j];
	}

	template<class T>
	inline T & Matrix_4<T>::operator()(const int & i, const int & j)
	{
		if (i < 0 || i >= 4)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index line for Matrix4!");
#else
			throw gmexception::MatrixIndexElementException("Error index line for Matrix4!");
#endif
		else if (j < 0 || j >= 4)
#ifdef _UNICODE
			throw gmexception::MatrixIndexElementException(L"Error index column for Matrix4!");
#else
			throw gmexception::MatrixIndexElementException("Error index column for Matrix4!");
#endif
		return this->matrix[i][j];
	}

	//  инкремент / декремент
	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::operator++(int)
	{
		Matrix_4<T> m(*this);
		for (auto& l : this->matrix)
			for (auto& e : l)
				e++;
		return m;
	}

	template<class T>
	inline Matrix_4<T> & gmath::Matrix_4<T>::operator++()
	{
		for (auto& l : this->matrix)
			for (auto& e : l)
				e++;
		return *this;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::operator--(int)
	{
		Matrix_4<T> m(*this);
		for (auto& l : this->matrix)
			for (auto& e : l)
				e--;
		return m;
	}

	template<class T>
	inline Matrix_4<T> & gmath::Matrix_4<T>::operator--()
	{
		for (auto& l : this->matrix)
			for (auto& e : l)
				e--;
		return *this;
	}

	// методы
	// абстрактные

	template<class T>
	inline void Matrix_4<T>::identity()
	{
		this->matrix[0][0] = this->matrix[1][1] = this->matrix[2][2] = this->matrix[3][3] = 1;
		this->matrix[0][1] = this->matrix[0][2] = this->matrix[0][3] = this->matrix[1][2] = this->matrix[1][3] = this->matrix[2][3] = 0;
		this->matrix[1][0] = this->matrix[2][0] = this->matrix[2][1] = this->matrix[3][0] = this->matrix[3][1] = this->matrix[3][2] = 0;
	}

	template<class T>
	inline void Matrix_4<T>::transponse()
	{
		Matrix_4<T> res(*this);
		for (int i(0); i < 4; i++)
			for (int j(0); j < 4; j++)
				res.matrix[i][j] = this->matrix[j][i];
		*this = res;
	}

	template<class T>
	inline void Matrix_4<T>::inverse()  
	{
		// детерминант(определитель)
		float determ(this->determinant());
		float tmp_determ(0.f); 
		if (determ == 0)
#ifdef _UNICODE
			throw gmexception::MatrixDeterminantException(L"Determinant matrix is by zero!");
#else
			throw gmexception::MatrixDeterminantException("Determinant matrix is by zero!");
#endif

		// транспонируем матрицу
		this->transponse(); 

		// считаем алгебраические дополнения
		Matrix_4<T> out;
		int _j(0); int _i(0);
		T array[9];
		T* ptr_array(array);
		int iter(0);
		while (iter < 16)
		{
			for (int i(0); i < 4; i++)
				for (int j(0); j < 4; j++)
					if (i != _i && j != _j)
						*ptr_array++ = this->matrix[i][j];
			Matrix_3<T> det(array, 9);
			float d = det.determinant();
			/*if (d == 0)
				out.matrix[_i][_j] = 0;
			else
			{*/
				float result(0.f);
				Matrix_2<T> m = { det(1, 1), det(1, 2), det(2, 1), det(2,2) };
				result = det(0,0) * m.determinant();

				m = { det(0,1), det(0,2), det(2,1), det(2,2) };
				result -= (det(1,0) * m.determinant());

				m = { det(0,1), det(0,2), det(1,1), det(1,2) };
				result += (det(2,0) * m.determinant());

				out.matrix[_i][_j] = result * powf(-1.f, _i + _j + 2.f);
			//}
			_j == 3 ? 0, _j = 0, ++_i : ++_j;
			iter++;
			ptr_array = array;
		}
		
		*this = (1 / determ) * out;
	}

	template<class T>
	inline float Matrix_4<T>::determinant()
	{
		float determ(0.f);
		Matrix_4<T> out;
		int _j(0); int _i(0);
		T array[9];
		T* ptr_array(array);
		while (_j < 4)
		{
			for (int i(0); i < 4; i++)
				for (int j(0); j < 4; j++)
					if (i != _i && j != _j)
						*ptr_array++ = this->matrix[i][j];
			Matrix_3<T> det(array, 9);
			T d = det.determinant();
			if (d != 0)
				_j % 2 == 0 ? determ += (this->matrix[_i][_j] * d) : determ -= (this->matrix[_i][_j] * d);
			ptr_array = array;
			_j++;
		}
		return determ;
	}

	template<class T>
	inline void Matrix_4<T>::reverse()
	{
		*this = ~(*this);
	}

	template<class T>
	inline std::string gmath::Matrix_4<T>::c_str()
	{
		float _max(0);
		for (auto& l : this->matrix)
		{
			T* tmp = std::max_element(l, l + 4, [](const T& t1, const T& t2)
			{
				return std::to_string((int)t1).length() < std::to_string((int)t2).length();
			});
			if (std::to_string((int)*tmp).length() > std::to_string((int)_max).length())
				_max = *tmp;
		}
		_max = std::to_string((int)_max).length() + 5;
		int i(0);

		std::stringstream buf;
		for (auto& line : this->matrix)
		{
			buf << "[ ";
			for (auto& e : line)
			{
				buf << std::right << std::setw(_max) << std::setprecision(3) << std::setiosflags(std::ios::fixed | std::ios::showpoint) << e;
				if (++i != 4)
					buf << "   ";
			}
			i = 0;
			buf << " ]" << std::endl;
		}
		return buf.str();
	}

	template<class T>
	inline std::wstring gmath::Matrix_4<T>::c_wstr()
	{
		float _max(0);
		for (auto& l : this->matrix)
		{
			T* tmp = std::max_element(l, l + 4, [](const T& t1, const T& t2)
			{
				return std::to_string((int)t1).length() < std::to_string((int)t2).length();
			});
			if (std::to_string((int)*tmp).length() > std::to_string((int)_max).length())
				_max = *tmp;
		}
		_max = std::to_string((int)_max).length() + 5;
		int i(0);

		std::wstringstream buf;
		for (auto& line : this->matrix)
		{
			buf << L"[ ";
			for (auto& e : line)
			{
				buf << std::right << std::setw(_max) << std::setprecision(3) << std::setiosflags(std::ios::fixed | std::ios::showpoint) << e;
				if (++i != 4)
					buf << L"   ";
			}
			i = 0;
			buf << L" ]" << std::endl;
		}
		return buf.str();
	}

	// аналоги абстрактных методов
	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::Identity()
	{
		Matrix_4<T> m;
		m.identity();
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::Transponse(const Matrix_4<T>& _m)
	{
		Matrix_4<T> m(_m);
		m.transponse();
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::Inverse(const Matrix_4<T>& _m)
	{
		Matrix_4<T> m(_m);
		m.inverse();
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::Reverse(const Matrix_4<T>& _m)
	{
		Matrix_4<T> m(_m);
		m.reverse();
		return m;
	}

	template<class T>
	inline float gmath::Matrix_4<T>::Determinant(const Matrix_4<T>& m)
	{
		return m.determinant();
	}

	// матричные преобразования, применимо к 3D
	template<class T>
	inline void gmath::Matrix_4<T>::createRotateMatrixX(const float & x)
	{
		this->identity();
		this->matrix[1][1] = this->matrix[2][2] = cos(x);
		this->matrix[1][2] = sin(x);
		this->matrix[2][1] = -sin(x);
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createRotateMatrixY(const float & y)
	{
		this->identity();
		this->matrix[0][0] = this->matrix[2][2] = cos(y);
		this->matrix[0][2] = -sin(y);
		this->matrix[2][0] = sin(y);
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createRotateMatrixZ(const float & z)
	{
		this->identity();
		this->matrix[1][1] = this->matrix[0][0] = cos(z);
		this->matrix[0][1] = sin(z);
		this->matrix[1][0] = -sin(z);
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createRotateMatrixXYZ(const float & x, const float & y, const float & z)
	{
		Matrix_4<T> mx; mx.identity(); mx.createRotateMatrixX(x);
		Matrix_4<T> my; my.identity(); my.createRotateMatrixY(y);
		Matrix_4<T> mz; mz.identity(); mz.createRotateMatrixZ(z);
		*this = mx * my * mz;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createRotateMatrix(const float & ang)
	{
		Matrix_4<T> mx; mx.identity(); mx.createRotateMatrixX(ang);
		Matrix_4<T> my; my.identity(); my.createRotateMatrixY(ang);
		Matrix_4<T> mz; mz.identity(); mz.createRotateMatrixZ(ang);
		*this = mx * my * mz;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createTranslationMatrixX(const float & x)
	{
		this->identity();
		this->matrix[3][0] = x;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createTranslationMatrixY(const float & y)
	{
		this->identity();
		this->matrix[3][1] = y;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createTranslationMatrixZ(const float & z)
	{
		this->identity();
		this->matrix[3][2] = z;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createTranslationMatrixXYZ(const float & x, const float & y, const float & z)
	{
		this->identity();
		this->matrix[3][0] = x;
		this->matrix[3][1] = y;
		this->matrix[3][2] = z;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createTranslationMatrix(const float & ang)
	{
		this->identity();
		this->matrix[3][0] = this->matrix[3][1] = this->matrix[3][2] = ang;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createScaleMatrixX(const float & x)
	{
		this->identity();
		this->matrix[0][0] = x;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createScaleMatrixY(const float & y)
	{
		this->identity();
		this->matrix[1][1] = y;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createScaleMatrixZ(const float & z)
	{
		this->identity();
		this->matrix[2][2] = z;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createScaleMatrixXYZ(const float & x, const float & y, const float & z)
	{
		this->identity();
		this->matrix[0][0] = x;
		this->matrix[1][1] = y;
		this->matrix[2][2] = z;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createScaleMatrix(const float & ang)
	{
		this->identity();
		this->matrix[0][0] = this->matrix[1][1] = this->matrix[2][2] = ang;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createLookAtLHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp)
	{
		this->createLookAtLHMatrix((Vector_3<T>)pEye, (Vector_3<T>)pAt, (Vector_3<T>)pUp);
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createLookAtLHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp)
	{
		Vector_3<T> zaxis = pAt - pEye; zaxis.normalize();
		Vector_3<T> xaxis = Vector_3<T>::cross(pUp, zaxis); xaxis.normalize();
		Vector_3<T> yaxis = Vector_3<T>::cross(zaxis, xaxis); yaxis.normalize();
		this->identity();
		this->matrix[0][0] = xaxis[gmtypes::x];
		this->matrix[0][1] = yaxis[gmtypes::x];
		this->matrix[0][2] = zaxis[gmtypes::x];

		this->matrix[1][0] = xaxis[gmtypes::y];
		this->matrix[1][1] = yaxis[gmtypes::y];
		this->matrix[1][2] = zaxis[gmtypes::y];

		this->matrix[2][0] = xaxis[gmtypes::z];
		this->matrix[2][1] = yaxis[gmtypes::z];
		this->matrix[2][2] = zaxis[gmtypes::z];

		this->matrix[3][0] = (T)(round(-Vector_3<T>::dot(xaxis, pEye) * 1000.) / 1000.);
		this->matrix[3][1] = (T)(round(-Vector_3<T>::dot(yaxis, pEye) * 1000.) / 1000.);
		this->matrix[3][2] = (T)(round(-Vector_3<T>::dot(zaxis, pEye) * 1000.) / 1000.);
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createLookAtRHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp)
	{
		this->createLookAtRHMatrix((Vector_3<T>)pEye, (Vector_3<T>)pAt, (Vector_3<T>)pUp);
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createLookAtRHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp)
	{
		Vector_3<T> zaxis = pEye - pAt; zaxis.normalize();
		Vector_3<T> xaxis = Vector_3<T>::cross(pUp, zaxis);  xaxis.normalize();
		Vector_3<T> yaxis = Vector_3<T>::cross(zaxis, xaxis); yaxis.normalize();
		this->identity();
		this->matrix[0][0] = xaxis[gmtypes::x];
		this->matrix[0][1] = yaxis[gmtypes::x];
		this->matrix[0][2] = zaxis[gmtypes::x];

		this->matrix[1][0] = xaxis[gmtypes::y];
		this->matrix[1][1] = yaxis[gmtypes::y];
		this->matrix[1][2] = zaxis[gmtypes::y];

		this->matrix[2][0] = xaxis[gmtypes::z];
		this->matrix[2][1] = yaxis[gmtypes::z];
		this->matrix[2][2] = zaxis[gmtypes::z];

		this->matrix[3][0] = round(Vector_3<T>::dot(xaxis, pEye) * 1000.) / 1000.;
		this->matrix[3][1] = round(Vector_3<T>::dot(yaxis, pEye) * 1000.) / 1000.;
		this->matrix[3][2] = round(Vector_3<T>::dot(zaxis, pEye) * 1000.) / 1000.;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createPerspectiveFovLHMatrix(const float & fovy, const float & aspect, const float & zn, const float & zf)
	{
		float yScale(fovy / 2); yScale = 1 / tan(yScale); yScale = (float)(round(yScale * 1000.) / 1000.);
		float xScale(yScale / aspect); xScale = (float)(round(xScale * 1000.) / 1000.);
		Matrix_4<T> m;
		m.matrix[0][0] = xScale;
		m.matrix[1][1] = yScale;
		m.matrix[2][2] = (T)(round(zf / (zf - zn) * 1000.) / 1000.);
		m.matrix[3][2] = (T)(round(-zn * zf / (zf - zn)* 1000.) / 1000.);
		m.matrix[2][3] = 1;
		*this = m;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createPerspectiveFovRHMatrix(const float & fovy, const float & aspect, const float & zn, const float & zf)
	{
		float yScale(fovy / 2); yScale = 1 / tan(yScale); yScale = round(yScale * 1000.) / 1000.;
		float xScale(yScale / aspect); xScale = round(xScale * 1000.) / 1000.;
		Matrix_4<T> m;
		m.matrix[0][0] = xScale;
		m.matrix[1][1] = yScale;
		m.matrix[2][2] = round(zf / (zn- zf) * 1000.) / 1000.;
		m.matrix[3][2] = round(zn * zf / (zn - zf) * 1000.) / 1000.;
		m.matrix[2][3] = -1;
		*this = m;
	}

	template<class T>
	inline void gmath::Matrix_4<T>::createViewportMatrix(float x, float y, float h, float w, float minz, float maxz)
	{
		this->identity();
		this->matrix[0][0] = w / 2;
		this->matrix[1][1] = -h / 2;
		this->matrix[2][2] = maxz - minz;
		this->matrix[3][0] = x + this->matrix[0][0];
		this->matrix[3][1] = y + h / 2;
		this->matrix[3][2] = minz;
	}

	// статические, аналоги матричных преобразований, применимо к 3D
	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateRotateMatrixX(const float & x)
	{
		Matrix_4<T> m; m.createRotateMatrixX(x);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateRotateMatrixY(const float & y)
	{
		Matrix_4<T> m; m.createRotateMatrixY(y);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateRotateMatrixZ(const float & z)
	{
		Matrix_4<T> m; m.createRotateMatrixZ(z);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateRotateMatrixXYZ(const float & x, const float & y, const float & z)
	{
		Matrix_4<T> m; m.createRotateMatrixXYZ(x,y,z);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateRotateMatrix(const float & ang)
	{
		Matrix_4<T> m; m.createRotateMatrix(ang);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateTranslationMatrixX(const float & x)
	{
		Matrix_4<T> m; m.createTranslationMatrixX(x);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateTranslationMatrixY(const float & y)
	{
		Matrix_4<T> m; m.createTranslationMatrixY(y);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateTranslationMatrixZ(const float & z)
	{
		Matrix_4<T> m; m.createTranslationMatrixZ(z);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateTranslationMatrixXYZ(const float & x, const float & y, const float & z)
	{
		Matrix_4<T> m; m.createTranslationMatrixXYZ(x,y,z);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateTranslationMatrix(const float & ang)
	{
		Matrix_4<T> m; m.createTranslationMatrix(ang);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateScaleMatrixX(const float & x)
	{
		Matrix_4<T> m; m.createScaleMatrixX(x);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateScaleMatrixY(const float & y)
	{
		Matrix_4<T> m; m.createScaleMatrixY(y);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateScaleMatrixZ(const float & z)
	{
		Matrix_4<T> m; m.createScaleMatrixZ(z);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateScaleMatrixXYZ(const float & x, const float & y, const float & z)
	{
		Matrix_4<T> m; m.createScaleMatrixXYZ(x, y, z);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateScaleMatrix(const float & ang)
	{
		Matrix_4<T> m; m.createScaleMatrix(ang);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateLookAtLHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp)
	{
		return Matrix_4<T>::CreateLookAtLHMatrix((Vector_3<T>)pEye, (Vector_3<T>)pAt, (Vector_3<T>)pUp);
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateLookAtLHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp)
	{
		Matrix_4<T> m; m.createLookAtLHMatrix(pEye, pAt, pUp);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateLookAtRHMatrix(const Vector_4<T>& pEye, const Vector_4<T>& pAt, const Vector_4<T>& pUp)
	{
		return Matrix_4<T>::CreateLookAtRHMatrix((Vector_3<T>)pEye, (Vector_3<T>)pAt, (Vector_3<T>)pUp);
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateLookAtRHMatrix(const Vector_3<T>& pEye, const Vector_3<T>& pAt, const Vector_3<T>& pUp)
	{
		Matrix_4<T> m; m.createLookAtRRHMatrix(pEye, pAt, pUp);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreatePerspectiveFovLHMatrix(const float & fovy, const float & aspect, const float & zn, const float & zf)
	{
		Matrix_4<T> m; m.createPerspectiveFovLHMatrix(fovy, aspect, zn, zf);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreatePerspectiveFovRHMatrix(const float & fovy, const float & aspect, const float & zn, const float & zf)
	{
		Matrix_4<T> m; m.createPerspectiveFovRHMatrix(fovy, aspect, zn, zf);
		return m;
	}

	template<class T>
	inline Matrix_4<T> gmath::Matrix_4<T>::CreateViewportMatrix(float x, float y, float w, float h, float minz, float maxz)
	{
		Matrix_4<T> m; m.createViewportMatrix(x, y, w, h, minz, maxz);
		return m;
	}
}
