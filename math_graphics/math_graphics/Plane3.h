#pragma once

#include "mathematics_typedef_vector.h"
#include "other_types.h"

using namespace gmtypes;

namespace gmath
{
	class plane_3
	{
		Vector pl;

	public:
#pragma region Constructors
		plane_3():pl(){}
		plane_3(const Vector& pl) :pl(pl){}
		plane_3(const Vector3& p0, const Vector3& n);
		plane_3(const Vector3& p0, const Vector3& p1, const Vector3& p2);
		plane_3(const plane_3& p);
		plane_3& operator=(const plane_3& p);
#pragma endregion

#pragma region Methods
		void setNormal(const Vector3& n); // ��������� ������� ���������
		void setD(float d); // ��������� d
		void normalize(); // ������������ ���������
		void createPlaneNormalAndPoint(const Vector3& p0, const Vector3& n); // ������� ��������� �� ����� � �������
		float shortDistanseFromPointToPlane(const Point& p); // ���������� ���������� �� ����� �� ���������
		Vector3 getNormal();
		float& getD();
		Vector& getPlane();
#pragma endregion


	};

}