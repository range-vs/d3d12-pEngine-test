#pragma once

#include "light.h"
#include "timer.h"

class Sun
{
	LightDirection light;
	Vector position;
	SystemTimer updater;
	Matrix rotate;
	float angle;
	int wait;
	int currentWait;
	wstring modelSun;
	Vector center;

	bool isStop()
	{
		if (currentWait == 0)
			return true;
		if (updater.isSecond())
		{
			--currentWait;
			updater.setBeforeTime();
		}
		return false;
	}
public:
	void init(const Color& c, const Vector& p, int w)
	{
		position = p;
		center = { 0,0,0,0 };
		light = LightDirection(c, Vector::createVectorForDoublePoints(position, center));
		rotate = Matrix::Identity();
		updater.start();
		updater.setBeforeTime();
		wait = w;
		currentWait = 0;
	}

	Matrix update(bool& status)
	{
		status = false;
		Matrix newPosSun = Matrix::Identity();
		updater.setAfterTime();
		if (!isStop())
		{
			return Matrix::Identity();
		}
		if (updater.isSecond_1_100())
		{
			updater.setBeforeTime();
			angle += 0.2f;
			if (angle > 180)
			{
				angle = 0;
				currentWait = wait;
				//return; // расскомментировать, чтобы солнце замирало в конце
			}
			rotate = Matrix::CreateRotateMatrixZ(GeneralMath::degreesToRadians(-angle));
			newPosSun = Matrix::CreateTranslationMatrixXYZ(position[x], position[y], position[z]) * rotate;
			Vector v = position * rotate;
			light.setDirection(Vector::createVectorForDoublePoints(v, center));
			status = true;
		}
		return newPosSun;
	}

	LightDirection& getLightSun() { return light; }

	void setModel(const wstring& s) { modelSun = s; }

	wstring& getNameModel() { return modelSun; }

	Vector& getStartPosition() { return position; }
};
