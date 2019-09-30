#pragma once

#include "types.h"

class FPSCounter;

class SystemTimer
{
	clock_t oldTime;
	clock_t newTime;
	clock_t allTime;

	void clear()
	{
		oldTime = newTime = allTime = 0;
	}

	friend class FPSCounter;
public:
	SystemTimer()
	{
		clear();
	}

	void start()
	{
		clear();
	}

	void setBeforeTime()
	{
		oldTime = clock();
	}

	void setAfterTime()
	{
		newTime = clock();
		allTime = newTime - oldTime;
	}

	bool isSecond() const
	{
		return allTime >= 1000.f;
	}

	bool isSecond_1_10() const
	{
		return allTime >= 100.f;
	}

	bool isSecond_1_100() const
	{
		return allTime >= 10.f;
	}

	bool isSecond_1_1000() const
	{
		return allTime >= 1.f;
	}

	float getCurrentTime()
	{
		return allTime;
	}

};

class FPSCounter
{
	SystemTimer timerFps;
	size_t frameCount;

	void clear()
	{
		timerFps.clear();
		frameCount = 0;
	}
public:
	FPSCounter()
	{
		clear();
	}

	void start()
	{
		clear();
	}

	void setBeforeTime()
	{
		timerFps.setBeforeTime();
	}

	void setAfterTime()
	{
		timerFps.setAfterTime();
	}

	bool isReady() const
	{
		return timerFps.allTime >= 500.f;
	}

	size_t getFps()
	{
		size_t fps = 1000 * frameCount / timerFps.allTime;
		clear();
		return fps;
	}

	FPSCounter& operator++()
	{
		frameCount++;
		return *this;
	}
};


