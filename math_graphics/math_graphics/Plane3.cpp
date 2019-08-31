#pragma once

#include "Plane3.h"

namespace gmath
{
	plane_3::plane_3(const Vector3 & p0, const Vector3 & p1, const Vector3 & p2)
	{
		Vector3 u = p1 - p0;
		Vector3 v = p2 - p0;
		Vector3 n = u.cross(v);
		n.normalize();
		createPlaneNormalAndPoint(p0, n);
	}

	plane_3::plane_3(const Vector3 & p0, const Vector3 & n)
	{
		createPlaneNormalAndPoint(p0, n);
	}

	plane_3::plane_3(const plane_3 & p)
	{
		pl = p.pl;
	}

	plane_3 & plane_3::operator=(const plane_3 & p)
	{
		if (this != &p)
		{
			pl = p.pl;
		}
		return *this;
	}

	void plane_3::setNormal(const Vector3 & n)
	{
		pl[x] = n[x];
		pl[y] = n[y];
		pl[z] = n[z];
	}

	void plane_3::setD(float d)
	{
		pl[w] = d;
	}

	void plane_3::normalize()
	{
		float norm = sqrt(pl[x] * pl[x] + pl[y] * pl[y] + pl[z] * pl[z]);
		if (norm)
		{
			pl[x] /= norm;
			pl[y] /= norm;
			pl[z] /= norm;
			pl[w] /= norm;
		}
		else
		{
			pl[x] = 0.0f;
			pl[y] = 0.0f;
			pl[z] = 0.0f;
			pl[w] = 0.0f;
		}
	}

	void plane_3::createPlaneNormalAndPoint(const Vector3& p0, const Vector3& n)
	{
		pl[x] = n[x];
		pl[y] = n[y];
		pl[z] = n[z];
		pl[w] = -Vector3::dot(p0, n);
	}

	float plane_3::shortDistanseFromPointToPlane(const Point & p0)
	{
		return pl[x]*p0[x] + pl[y]*p0[y] + pl[z]*p0[z] + pl[w] * 1; // если расстрояние до плоскости < 0 - точка сзади(вектор смотрит от точки), иначе точка спереди(векто смотрит на точку),
		// == 0 - точка на плоскости
	}

	Vector3 plane_3::getNormal()
	{
		return (Vector3)pl;
	}

	float & plane_3::getD()
	{
		return pl[w];
	}

	Vector & plane_3::getPlane()
	{
		return pl;
	}

}