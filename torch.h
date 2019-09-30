#pragma once

#include "light.h"

class Torch
{
	bool _enable;
	bool _used;
	LightSpot light;
public:
	void init(const Color& c, const Vector& p, const Vector& d, float coff, float outerCutOff)
	{
		light = LightSpot(c, p, d, cos(GeneralMath::degreesToRadians(coff)), cos(GeneralMath::degreesToRadians(outerCutOff)));
	}

	void update(const Vector& camPos, const Vector& camDir)
	{
		light.setPosition(camPos);
		light.setDirection(Vector::Normalize(camDir));
	}

	void enable()
	{
		_enable = true;
	}
	void disable()
	{
		_enable = false;
	}
	bool isEnable() { return _enable; }
	LightSpot& getLight() { return light; }
	void torchUsed(bool use) { _used = use; }
	bool isTorchUsed() { return _used; }
};