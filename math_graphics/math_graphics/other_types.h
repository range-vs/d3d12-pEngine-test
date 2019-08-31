#pragma once

#include <string>

namespace gmtypes
{
	enum VectorCoordinate
	{
		x = 0,
		y = 1,
		z = 2,
		w = 3,
		r = 0,
		g = 1,
		b = 2,
		a = 3
	};

	enum PlanesParts
	{
		TOP = 0,
		BOTTOM,
		LEFT,
		RIGHT,
		FARP, // зад
		NEARP // перед
	};

}