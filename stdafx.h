#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN    // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>

#include <map>

using namespace std;

// this will only call release if an object exists (prevents exceptions calling release on non existant objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

using namespace DirectX; // we will be using the directxmath library

// Handle to the window
HWND hwnd = NULL;

// name of the window (not the title)
LPCTSTR WindowName = L"BzTutsApp";

// title of the window
LPCTSTR WindowTitle = L"Bz Window";

// width and height of the window
int Width = 800;
int Height = 600;

// is window full screen?
bool FullScreen = true;

// we will exit the program when this becomes false
bool Running = true;

// create a window
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	bool fullscreen);

// main application loop
void mainloop();

// callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);

// direct3d stuff
const int frameBufferCount = 3; // number of buffers we want, 2 for double buffering, 3 for tripple buffering

ID3D12Device* device; // direct3d device

IDXGISwapChain3* swapChain; // swapchain used to switch between render targets

ID3D12CommandQueue* commandQueue; // container for command lists

ID3D12DescriptorHeap* rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets

ID3D12Resource* renderTargets[frameBufferCount]; // number of render targets equal to buffer count

ID3D12CommandAllocator* commandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)

ID3D12GraphicsCommandList* commandList; // a command list we can record commands into, then execute them to render the frame

ID3D12Fence* fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu. We need as many 
										 //as we have allocators (more if we want to know when the gpu is finished with an asset)

HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu

UINT64 fenceValue[frameBufferCount]; // this value is incremented each frame. each fence will have its own value

int frameIndex; // current rtv we are on

int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)
					   // function declarations

bool InitD3D(); // initializes direct3d 12

void Update(); // update the game logic

void UpdatePipeline(); // update the direct3d pipeline (update command lists)

void Render(); // execute the command list

void Cleanup(); // release com ojects and clean up memory

void WaitForPreviousFrame(); // wait until gpu is finished with command list

ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state

ID3D12RootSignature* rootSignature; // root signature defines data shaders will access

D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.

D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

ID3D12Resource* vertexBuffer[10]; // a default buffer in GPU memory that we will load vertex data for our triangle into
ID3D12Resource* indexBuffer[10]; // a default buffer in GPU memory that we will load index data for our triangle into

D3D12_VERTEX_BUFFER_VIEW vertexBufferView[10]; // a structure containing a pointer to the vertex data in gpu memory
										   // the total size of the buffer, and the size of each element (vertex)

D3D12_INDEX_BUFFER_VIEW indexBufferView[10]; // a structure holding information about the index buffer

ID3D12Resource* depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
ID3D12DescriptorHeap* dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

// this is the structure of our constant buffer.
struct ConstantBufferPerObject {
	XMFLOAT4X4 wvpMat;
};

// Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
// We are only able to read at 256 byte intervals from the start of a resource heap, so we will
// make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
// Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
// we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
// The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
// buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
// were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
// being copied.
int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

ConstantBufferPerObject cbPerObject; // this is the constant buffer data we will send to the gpu 
										// (which will be placed in the resource we created above)

ID3D12Resource* constantBufferUploadHeaps[frameBufferCount]; // this is the memory on the gpu where constant buffers for each frame will be placed

UINT8* cbvGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps

XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
XMFLOAT4X4 cameraViewMat; // this will store our view matrix

XMFLOAT4 cameraPosition; // this is our cameras position vector
XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
XMFLOAT4 cameraUp; // the worlds up vector

XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
XMFLOAT4 cube1Position; // our first cubes position in space

XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

int numCubeIndices; // the number of indices to draw the cube

//map<char, bool> keyStatus;
bool updateCamera();
double GetFrameTime();
struct CameraProperties
{
	float moveLeftRight;
	float moveBackForward;
	float camYaw;
	float camPitch;
	POINT lastPosMouse;
	POINT mouseCurrState;

	CameraProperties() :moveLeftRight(0.f), moveBackForward(0.f), camYaw(0.f), camPitch(0.f)
	{
		lastPosMouse = { 0, 0 };
	}
};

CameraProperties camProp;
XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camPosition = XMVectorSet(0, 2, -4, 0);
XMVECTOR camTarget = XMVectorSet(0, 0, 0, 0);
XMVECTOR camUp = XMVectorSet(0, 1, 0, 0);
CameraProperties currentProperties;

int vBufferSize;
int iBufferSize;

class MatrixCPUAndGPU
{
	XMMATRIX _m;
	XMMATRIX _mT;

	void createTransponse()
	{
		_mT = XMMatrixTranspose(_m);
	}

public:
	MatrixCPUAndGPU() :_m(XMMatrixIdentity()), _mT(XMMatrixIdentity()) {}

	MatrixCPUAndGPU(const XMMATRIX& m) :_m(m)
	{
		createTransponse();
	}

	XMMATRIX getCPUMatrix()
	{
		return _m;
	}

	XMMATRIX getGPUMatrix()
	{
		return _mT;
	}

	void updateCPUMatrix(const XMMATRIX& m)
	{
		_m = m;
		createTransponse();
	}

};

class Camera
{
	MatrixCPUAndGPU view;
	MatrixCPUAndGPU projection;
public:
	Camera() {}
	Camera(const XMVECTOR& p, const XMVECTOR& d)
	{
		updateView(p, d, XMVectorSet(0,1,0,0));
	}
	Camera(const XMMATRIX& v, const XMMATRIX& p) :view(v), projection(p) {}
	void updateView(const XMVECTOR& p, const XMVECTOR& d, const XMVECTOR& h) { view = XMMatrixLookAtLH(p, d, h); }
	void updateProjection(float fov, float aspect, float zn, float zf) { projection = XMMatrixPerspectiveFovLH(fov, aspect, zn, zf); }
	MatrixCPUAndGPU& getView() { return view; }
	MatrixCPUAndGPU& getProjection() { return projection; }
	virtual void updateCamera(const CameraProperties& cProp) = 0;
	virtual float getSpeed(float time, bool isX) = 0;
};

class StaticCamera : public Camera
{
public:
	StaticCamera(const XMVECTOR& p, const XMVECTOR& d) :Camera(p, d) {}

	void updateCamera(const CameraProperties& cProp) override
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
	XMVECTOR DefaultForward;
	XMVECTOR DefaultRight;
	XMVECTOR camForward;
	XMVECTOR camRight;
	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

public:
	GameCamera(float step, float run, const XMVECTOR& p, const XMVECTOR& d) :_step(step), _run(run)
	{
		currentProperties = {};
		DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		camPosition = p;
		camTarget = d;
		camUp = XMVectorSet(0,1,0,0);
		updateView(camPosition, camTarget, camUp);

	}

	void updateCamera(const CameraProperties& cProp) override
	{
		currentProperties = cProp;
		XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(currentProperties.camPitch, currentProperties.camYaw, 0);
		camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
		camTarget = XMVector3Normalize(camTarget);

		XMMATRIX RotateYTempMatrix;
		RotateYTempMatrix = XMMatrixRotationY(currentProperties.camYaw);

		camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
		camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
		camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

		camPosition += currentProperties.moveLeftRight * camRight;
		camPosition += currentProperties.moveBackForward * camForward;

		camTarget = camPosition + camTarget;

		updateView(camPosition, camTarget, camUp);
	}
	float getSpeed(float time, bool isX) override
	{
		if (isX)
			return _run * time;
		return _step * time;
	}
};

GameCamera* gc;