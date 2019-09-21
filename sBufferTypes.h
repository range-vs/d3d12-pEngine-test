#pragma once

#include "math_graphics/math_graphics/mathematics.h"

using namespace gmath;

struct sbufferInstancing
{
	Matrix4 world;
	Matrix4 normals;

	sbufferInstancing() :world(Matrix4::Identity()), normals(Matrix4::Identity()){}
	sbufferInstancing(const Matrix4& w, const Matrix4& n) :world(w), normals(n) {}
};