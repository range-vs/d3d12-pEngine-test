#pragma once

#include <math.h>
#include "mathematics_typedef_vector.h"

namespace gmath
{
	class GeneralMath // общие математические функции
	{
	public:
		// набор констант Пи
		static const float PI;
		static const float _2PI;
		static const float _4PI;
		static const float PI_2;
		static const float PI_4;
		
		// работа с градусами и радианами
		static float degreesToRadians(const float& d); // градусы в радианы
		static float radiansToDigrees(const float& r); // радианы в градусы
		static float clamp(float value, float min, float max);

	};

}