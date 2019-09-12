#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <string>
#include <chrono>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
using namespace std;
using Microsoft::WRL::ComPtr;

void updateSize();
bool InitializeWindow(HINSTANCE hInstance);
bool resize(int w, int h);
void mainloop();
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool InitD3D();
void UpdatePipeline();
void Render();
void WaitForPreviousFrame();
void flushGpu();
bool createBackBuffer();
void createViewport();
void createScissor();
bool changeModeDisplay();

HWND hwnd = nullptr;
int widthClient = 900;
int heightClient = 650;
int startWidth;
int startHeight;
int widthWindow;
int heightWindow;
int frameBufferCount = 3;
ComPtr<IDXGIFactory4> dxgiFactory;
DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
ComPtr < ID3D12Device> device;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr < ID3D12CommandQueue> commandQueue;
ComPtr < ID3D12DescriptorHeap> rtvDescriptorHeap;
vector< ComPtr<ID3D12Resource>> renderTargets;
vector< ComPtr<ID3D12CommandAllocator>> commandAllocator;
ComPtr < ID3D12GraphicsCommandList> commandList;
vector< ComPtr<ID3D12Fence>> fence;
HANDLE fenceEvent = nullptr;
vector<unsigned long long> fenceValue;
int frameIndex = 0;
int rtvDescriptorSize = -1;
D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;
bool vsync = false;
float color[] = { 0.53f, 0.53f, 0.53f, 1.f };
bool modeWindow = false;
bool isRunning = false;

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
//{
//	if (!InitializeWindow(hInstance)) return -1;
//	if (!InitD3D()) return -1;
//	mainloop();
//	WaitForPreviousFrame();
//	return 0;
//}

void updateSize()
{
	RECT windowRect;
	GetClientRect(hwnd, &windowRect);
	widthClient = windowRect.right;
	heightClient = windowRect.bottom;
	startWidth = windowRect.left;
	startHeight = windowRect.top;
	GetWindowRect(hwnd, &windowRect);
	widthWindow = windowRect.right - windowRect.left;
	heightWindow = windowRect.bottom - windowRect.top;
}

bool InitializeWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(color[0] * 255, color[1] * 255, color[2] * 255))); //(HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszClassName = L"dx12_className";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (!RegisterClassEx(&wc)) return false;
	if (!(hwnd = CreateWindowEx(NULL, L"dx12_className", L"DX12 Render Triangles", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, widthClient, heightClient, NULL, NULL, hInstance, NULL))) return false;
	updateSize();
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	return true;
}

void mainloop()
{
	MSG msg = {};
	while (true)
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (isRunning)
				Render();
			else
				Sleep(100);
		}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_HOME)
			if (!changeModeDisplay())
			{
				PostQuitMessage(0);
				return 0;
			}
		break;
	case WM_DESTROY: 
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		updateSize();
		if (device)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				isRunning = false;
				break;
			}
			else if (wParam == SIZE_RESTORED)
			{
				isRunning = true;
			}
			if(!resize(widthWindow, heightWindow))
			{
				PostQuitMessage(0);
				return 0;
			}
			Render();
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool changeModeDisplay()
{
	HRESULT hr = swapChain->SetFullscreenState(modeWindow, nullptr);
	if (FAILED(hr))
		return false;
	modeWindow = !modeWindow;
	return true;
}

bool InitD3D()
{
	// init
	renderTargets.resize(frameBufferCount);
	commandAllocator.resize(frameBufferCount);
	fence.resize(frameBufferCount);
	fenceValue = { 0,0,0 };
	// debug
	ComPtr<ID3D12Debug> debugController;
	if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) return false;
	debugController->EnableDebugLayer();
	// dxgi factory
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf())))) return false;
	// device
	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf())))) return false;
	// fence
	for (int i = 0; i < frameBufferCount; i++)
		if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence[i].GetAddressOf())))) return false;
	if (!(fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr))) return false;
	// queue
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(commandQueue.GetAddressOf())))) return false;
	commandQueue->SetName(L"Command queue");
	// allocators
	for (int i = 0; i < frameBufferCount; i++)
		if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator[i].GetAddressOf())))) return false;
	// list
	if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex].Get(), NULL, IID_PPV_ARGS(commandList.GetAddressOf())))) return false;
	commandList->Close();
	// swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Format = backBufferFormat;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	ComPtr<IDXGISwapChain1> tempSwapChain;
	if (FAILED(dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, tempSwapChain.GetAddressOf()))) return false;
	if (!(swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain.Get()))) return false;
	frameIndex = swapChain->GetCurrentBackBufferIndex();
	DXGI_RGBA swapChainBkColor = { color[0], color[1], color[2], color[3] };
	if (FAILED(swapChain->SetBackgroundColor(&swapChainBkColor))) return false;
	// resize
	isRunning = true;
	if (FAILED(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)))
		return false;
	if (!changeModeDisplay())
		return false;
	if (!resize(widthWindow, heightWindow))
		return false;
	return true;
}

bool resize(int w, int h)
{
	if (device && swapChain && commandAllocator[frameIndex])
	{
		flushGpu();
		for (int i(0); i < frameBufferCount; i++)
			renderTargets[i].Reset();
		if (FAILED(swapChain->ResizeBuffers(frameBufferCount, w, h, backBufferFormat, NULL))) return false;
		if (!createBackBuffer()) return false;
		createViewport();
		createScissor();
		return true;
	}
	return false;
}

void UpdatePipeline()
{
	WaitForPreviousFrame();
	commandAllocator[frameIndex]->Reset();
	commandList->Reset(commandAllocator[frameIndex].Get(), nullptr);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	commandList->Close();
}

void Render()
{
	UpdatePipeline();
	vector<ID3D12CommandList*> ppCommandLists = { commandList.Get() };
	commandQueue->ExecuteCommandLists(ppCommandLists.size(), ppCommandLists.data());
	commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
	swapChain->Present(vsync ? 1 : 0, 0);
}

void WaitForPreviousFrame()
{
	frameIndex = swapChain->GetCurrentBackBufferIndex();
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	fenceValue[frameIndex]++;
}

void flushGpu()
{
	for (int i = 0; i < frameBufferCount; i++)
	{
		uint64_t fenceValueForSignal = ++fenceValue[i];
		commandQueue->Signal(fence[i].Get(), fenceValueForSignal);
		if (fence[i]->GetCompletedValue() < fenceValue[i])
		{
			fence[i]->SetEventOnCompletion(fenceValueForSignal, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}
	}
	frameIndex = 0;
}

bool createBackBuffer()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvDescriptorHeap.GetAddressOf()))))
		return false;
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	wstring nameBb = L"back buffer #1";
	for (int i = 0; i < frameBufferCount; i++)
	{
		if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargets[i].GetAddressOf()))))
			return false;
		device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
		renderTargets[i]->SetName(nameBb.c_str());
		nameBb[nameBb.size() - 1]++;

		rtvHandle.Offset(1, rtvDescriptorSize);
	}
	return true;
}

void createViewport()
{
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = widthClient;
	viewport.Height = heightClient;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void createScissor()
{
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = widthClient;
	scissorRect.bottom = heightClient;
}

// перенести на рабочую сцену