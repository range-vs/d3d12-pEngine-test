#pragma once

#include "sBufferTypes.h"
#include <string>

using namespace gmath;
using namespace std;

using Index = unsigned int;

struct Vertex
{
	Vector3 pos;
	Vector3 normals;
	Vector4 color;

	Vertex(const Vector3& p, const Vector3& n, const Vector4& cl) :pos(p), normals(n), color(cl) {}
	Vertex() :pos(), normals(), color() {}
};

struct DateTime // время и дата
{
	int seconds;
	int minutes;
	int hours;
	int days;
	int mounts;
	int years;

	DateTime(int s, int m, int h, int d, int mo, int y) :seconds(s), minutes(m), hours(h), days(d), mounts(mo), years(1900 + y) {}
	DateTime() :seconds(0), minutes(0), hours(0), days(0), mounts(0), years(1900) {}
};

struct BoundingBoxData // данные для создания BBox
{
	float xl, yd, zf; // лево, низ и перед
	float xr, yu, zr; // право, верх и зад
	Vector center; // центр модели

	BoundingBoxData(float xl, float yd, float zf, float xr, float yu, float zr) :xl(xl), yd(yd), zf(zf), xr(xr), yu(yu), zr(zr) {}
	BoundingBoxData() :xl(0), yd(0), zf(0), xr(0), yu(0), zr(0) {}
};

struct BlankModel
{
	vector<Index> indexs;
	vector<size_t> shiftIndexs;
	vector<Vertex> vertexs;
	vector<wstring> materials;
	wstring psoName;
	vector<wstring> texturesName;
	vector<bool> doubleSides;
	wstring nameModel;
	bool instansing = false;
	vector< sbufferInstancing> instansingBuffer;
};

enum ModelConstants: wchar_t
{
	isColor = L'\n'
};
