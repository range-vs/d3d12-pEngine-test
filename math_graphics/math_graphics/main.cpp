#pragma once

#include "mathematics.h"
#include <string>

using namespace gmath;

int main()
{
	Matrix m;
	m.createScaleMatrixXYZ(4, 8, 6);
	std::cout << m.c_str() << std::endl;
	
	Matrix_2<float, 64> m2 = { 1,8,9,5 };
	m = (Matrix)m2;

	system("pause");
	return 0;
}
