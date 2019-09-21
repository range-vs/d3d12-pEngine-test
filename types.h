#pragma once

#include "sBufferTypes.h"

#include <vector>

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
	Vertex():pos(), normals(), color(){}
};

struct BlankModel
{
	vector<Index> indexs;
	vector<size_t> shiftIndexs;
	vector<Vertex> vertexs;
	vector<wstring> materials;
	wstring psoName;
	vector<wstring> texturesName;
	wstring nameModel;
	bool instansing = false;
	vector< sbufferInstancing> instansingBuffer;
};

