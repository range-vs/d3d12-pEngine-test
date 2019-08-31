#pragma once

#include <vector>

#include "math_graphics/math_graphics/mathematics.h"
using namespace gmath;

using namespace std;

using uint = unsigned int;
using uint8 = unsigned char;
using uint64 = unsigned long long;

using Index = uint;

struct Vertex
{
	Vector3 pos;
	Vector3 normals;
	Vector4 color;

	Vertex(const Vector3& p, const Vector3& n, const Vector4& cl) :pos(p), normals(n), color(cl) {}
};

struct BlankModel
{
	vector<Index> indexs;
	vector<Vertex> vertexs;
	wstring material;
	wstring psoName;

};