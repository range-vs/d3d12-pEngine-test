#pragma once

#include "types.h"

enum TypeCamera : int
{
	STATIC_CAMERA,
	GAME_CAMERA
};

struct CameraProperties
{
	float moveLeftRight;
	float moveBackForward;
	float camYaw;
	float camPitch;
	float squat;
	Point2 lastPosMouse;
	Point2 mouseCurrState;

	CameraProperties() :moveLeftRight(0.f), moveBackForward(0.f), camYaw(0.f), camPitch(0.f)
	{
		lastPosMouse = { 0, 0 };
		mouseCurrState = { 0,0 };
	}
};

class Camera
{
	MatrixCPUAndGPU view;
	MatrixCPUAndGPU projection;
protected:
	static Vector3 hCamStandart;
	Vector3 camPosition;
	Vector3 camUp;
	Vector3 camTarget;
	Vector3 camForward;
	Vector3 camRight;

public:
	Camera() {}
	Camera(const Vector3& p, const Vector3& d)
	{
		updateView(p, d, hCamStandart);
	}
	Camera(const Matrix4& v, const Matrix4& p) :view(v), projection(p) {}
	void updateView(const Vector3& p, const Vector3& d, const Vector3& h) { view = Matrix4::CreateLookAtLHMatrix(p, d, h); }
	void updateProjection(float fov, float aspect, float zn, float zf) { projection = Matrix4::CreatePerspectiveFovLHMatrix(fov, aspect, zn, zf); }
	MatrixCPUAndGPU& getView() { return view; }
	MatrixCPUAndGPU& getProjection() { return projection; }
	Vector3& getPosition() { return camPosition; }
	Vector3& getUp() { return camUp; }
	Vector3& getTarget() { return camTarget; }
	Vector3& getForward() { return camForward; }
	Vector3& getRight() { return camRight; }

	virtual void updateCamera(map<char, bool>& keys, const Point2& mDelta, double time) = 0;
	virtual float getSpeed(float time, bool isX) = 0;
};

Vector3 Camera::hCamStandart(0.0f, 1.0f, 0.0f);

class StaticCamera : public Camera
{
public:
	StaticCamera(const Vector3& p, const Vector3& d) :Camera(p, d) {}

	void updateCamera(map<char, bool>& keys, const Point2& mDelta, double time) override
	{

	}
	float getSpeed(float time, bool isX) override
	{
		return 0;
	}
};

class GameCamera : public Camera
{
	float _step;
	float _run;
	CameraProperties currentProperties;
	Vector3 DefaultForward;
	Vector3 DefaultRight;


public:
	GameCamera(float step, float run, const Vector3& p, const Vector3& d) :_step(step), _run(run)
	{
		currentProperties = {};
		DefaultForward = Vector3(0.0f, 0.0f, 1.0f);
		DefaultRight = Vector3(1.0f, 0.0f, 0.0f);
		camForward = Vector3(0.0f, 0.0f, 1.0f);
		camRight = Vector3(1.0f, 0.0f, 0.0f);
		camPosition = p;
		camTarget = d;
		camUp = hCamStandart;
		updateView(camPosition, camTarget, camUp);
	}
	void updateCamera(map<char, bool>& keyStatus, const Point2& mDelta, double time) override
	{
		float speed = getSpeed(time, keyStatus['X']);

		if (keyStatus['A'])
		{
			OutputDebugString(L"A pressed\n");
			currentProperties.moveLeftRight -= speed;
		}
		if (keyStatus['D'])
		{
			OutputDebugString(L"D pressed\n");
			currentProperties.moveLeftRight += speed;
		}
		if (keyStatus['W'])
		{
			OutputDebugString(L"W pressed\n");
			currentProperties.moveBackForward += speed;
		}
		if (keyStatus['S'])
		{
			OutputDebugString(L"S pressed\n");
			currentProperties.moveBackForward -= speed;
		}

		if ((mDelta[x] != 0) || (mDelta[y] != 0))
		{
			OutputDebugString(L"Mouse moved\n");
			OutputDebugString((to_wstring(mDelta[x]) + L" " + to_wstring(mDelta[y]) + L"\n").c_str());

			currentProperties.camYaw += (mDelta[x] * 0.001f); // todo?
			currentProperties.camPitch += (mDelta[y] * 0.001f);
			currentProperties.lastPosMouse = currentProperties.mouseCurrState;
		}

		float angle = 1.483f; // 85 градусов
		if (currentProperties.camPitch > 0)
			currentProperties.camPitch = min(currentProperties.camPitch, angle);
		else
			currentProperties.camPitch = max(currentProperties.camPitch, -angle);

		Matrix4 camRotationMatrix = Matrix4::CreateRotateMatrixXYZ(currentProperties.camPitch, currentProperties.camYaw, 0);
		camTarget = DefaultForward;
		camTarget.vector3TransformCoord(camRotationMatrix);
		camTarget.normalize();

		Matrix4 RotateYTempMatrix = Matrix4::CreateRotateMatrixY(currentProperties.camYaw);
		camRight = DefaultRight;
		camRight.vector3TransformCoord(RotateYTempMatrix);
		camUp.vector3TransformCoord(RotateYTempMatrix);
		camForward = DefaultForward;
		camForward.vector3TransformCoord(RotateYTempMatrix);

		camPosition += camRight * currentProperties.moveLeftRight;
		camPosition += camForward * currentProperties.moveBackForward;

		camTarget = camPosition + camTarget;

		updateView(camPosition, camTarget, camUp);

		currentProperties.moveLeftRight = currentProperties.moveBackForward = 0.f;
	}
	float getSpeed(float time, bool isX) override
	{
		if (isX)
			return _run * time;
		return _step * time;
	}
};
