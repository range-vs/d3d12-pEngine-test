#pragma once

#include <math.h>
#include "mathematics_typedef_vector.h"

namespace gmath
{
	class GeneralMath // ����� �������������� �������
	{
	public:
		// ����� �������� ��
		static const float PI;
		static const float _2PI;
		static const float _4PI;
		static const float PI_2;
		static const float PI_4;
		
		// ������ � ��������� � ���������
		static float degreesToRadians(const float& d); // ������� � �������
		static float radiansToDigrees(const float& r); // ������� � �������
		static float clamp(float value, float min, float max);

	};

}