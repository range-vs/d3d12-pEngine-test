#pragma once

#include "types.h"

class Light
{
protected:
	Color color;

public:
	Light() :color() {}
	Light(const Color& c) :color(c) {}
	Color& getColor() { return color; }
	void setColor(const Color& c) { color = c; }
};

class LightDirection : public Light
{
	Vector dir;

public:
	LightDirection() :Light(), dir() {}
	LightDirection(const Color& c, const Vector& d) :Light(c), dir(d) {}
	Vector& getDirection() { return dir; }
	void setDirection(const Vector& d) { dir = d; }
};

class LightPoint : public Light
{
	Vector pos;

public:
	LightPoint() : Light(), pos() {}
	LightPoint(const Color& c, const Vector& p) :Light(c), pos(p) {}
	Vector& getPosition() { return pos; }
	void setPosition(const Vector& p) { pos = p; }
};

class LightSpot : public Light
{
	Vector pos;
	Vector dir;
	float cutOff;
	float outerCutOff;

public:
	LightSpot() : Light(), pos(), dir(), cutOff(0.f), outerCutOff(0.f) {}
	LightSpot(const Color& c, const Vector& p, const Vector& d, float coff, float outerCutOff) :Light(c), pos(p), dir(d), cutOff(coff), outerCutOff(outerCutOff) {}
	Vector& getPosition() { return pos; }
	void setPosition(const Vector& p) { pos = p; }
	Vector& getDirection() { return dir; }
	void setDirection(const Vector& d) { dir = d; }
	float& getCutOff() { return cutOff; }
	void setCutOff(float coff) { cutOff = coff; }
	float& getOuterCutOff() { return outerCutOff; }
	void setOuterCutOff(float outerCutOff) { this->outerCutOff = outerCutOff; }
};

