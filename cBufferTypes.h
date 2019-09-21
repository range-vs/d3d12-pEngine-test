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

struct cbufferLight
{
	Matrix4 normals;
	Color colorDirectional;
	Vector directionalDirectional;
	Color colorPoint;
	Vector positionPoint;
	Vector cameraPosition;
	Vector directionalSpot;
	Vector positionSpot;
	Vector colorSpot;
	Vector propertySpot;

	cbufferLight() :colorDirectional(), directionalDirectional(), colorPoint(), positionPoint(), cameraPosition(), normals(Matrix4::Identity()), colorSpot(),
	directionalSpot(), positionSpot(), propertySpot(){}
	cbufferLight(const Color& c, const Vector& d, const Color& colp, const Vector& pp, const Vector& cp, const Matrix4& n,
		const Color& cs, const Vector& ds, const Vector& ps, const Vector& prop) :colorDirectional(c), directionalDirectional(d),
		colorPoint(colp), positionPoint(pp), cameraPosition(cp), normals(n), colorSpot(cs), directionalSpot(ds), positionSpot(ps), propertySpot(prop) {}
};
