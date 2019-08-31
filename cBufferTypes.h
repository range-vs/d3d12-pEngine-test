#pragma once

#include "types.h"

struct cbufferCamera
{
	Matrix4 world;
	Matrix4 view;
	Matrix4 projection;

	cbufferCamera() :world(Matrix4::Identity()), view(Matrix4::Identity()), projection(Matrix4::Identity()) {}
	cbufferCamera(const Matrix4& w, const Matrix4& v, const Matrix4& p) :world(w), view(v), projection(p) {}
};
