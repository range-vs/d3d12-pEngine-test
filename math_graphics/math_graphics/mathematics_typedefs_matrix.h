#pragma once

// math
#include "Matrix.h"

// с++
#include <iomanip>
#include <sstream>
#include <algorithm>

// объ€влени€ всех классов
namespace gmath
{
	template<class T> class Vector_2;
	template<class T> class Vector_3;
	template<class T> class Vector_4;
	template<class T> class Matrix_2;
	template<class T> class Matrix_3;
	template<class T> class Matrix_4;
}

// код всех классов - матриц
#include "Matrix2.h"
#include "Matrix3.h"
#include "Matrix4.h"

// псевдонимы
namespace gmath
{
	using Matrix4 = Matrix_4<float>;
	using Matrix3 = Matrix_3<float>;
	using Matrix = Matrix4;
}