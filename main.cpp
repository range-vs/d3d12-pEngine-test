#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include <wrl.h>
#include <vector>
#include <string>
#include <chrono>
#include <D3Dcompiler.h>
#include <DirectXMath.h>


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "D3Dcompiler.lib")

using namespace std;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct Vertex1
{
	XMFLOAT3 pos;
};

struct cbufferCamera
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;

	cbufferCamera() :world(), view(), projection() {}
	cbufferCamera(const XMMATRIX& w, const XMMATRIX& v, const XMMATRIX& p) :world(w), view(v), projection(p) {}
};

struct cbufferLight
{
	XMFLOAT4 colorDirectional;
	XMFLOAT4 directionalDirectional;
	XMFLOAT4 cameraPosition;
	XMMATRIX normals;

	cbufferLight() :colorDirectional(), directionalDirectional(), cameraPosition(), normals() {}
	cbufferLight(const XMFLOAT4& c, const XMFLOAT4& d, const XMFLOAT4& cp, const XMMATRIX& n) :colorDirectional(c), directionalDirectional(d), cameraPosition(cp), normals(n) {}
};

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
ID3D12RootSignature* rootSignature;
ID3D12PipelineState* pipelineStateObject;
ID3D12Resource* vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

vector < ComPtr < ID3D12Resource>> constantBufferUploadHeap1;
vector<unsigned char*> bufferGpuAddress1;
size_t shift1;
vector < ComPtr < ID3D12Resource>> constantBufferUploadHeap2;
vector<unsigned char*> bufferGpuAddress2;
size_t shift2;

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
	//commandList->Close();
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

	// TEST CBs //
	// create root signature
	HRESULT hr;

	constantBufferUploadHeap1.resize(frameBufferCount);
	bufferGpuAddress1.resize(frameBufferCount);
	shift1 = (sizeof(cbufferCamera) + 255) & ~255;
	for (int i(0); i < frameBufferCount; ++i)
	{
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(constantBufferUploadHeap1[i].GetAddressOf()));
		if (FAILED(hr))
			return false;
		hr = constantBufferUploadHeap1[i]->SetName(L"camera");
		if (FAILED(hr))
			return false;
		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
		hr = constantBufferUploadHeap1[i]->Map(0, &readRange, reinterpret_cast<void**>(&bufferGpuAddress1[i]));
		if (FAILED(hr))
			return false;
	}
	constantBufferUploadHeap2.resize(frameBufferCount);
	bufferGpuAddress2.resize(frameBufferCount);
	shift2 = (sizeof(cbufferLight) + 255) & ~255;
	for (int i(0); i < frameBufferCount; ++i)
	{
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(constantBufferUploadHeap2[i].GetAddressOf()));
		if (FAILED(hr))
			return false;
		hr = constantBufferUploadHeap2[i]->SetName(L"dir light");
		if (FAILED(hr))
			return false;
		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
		hr = constantBufferUploadHeap2[i]->Map(0, &readRange, reinterpret_cast<void**>(&bufferGpuAddress2[i]));
		if (FAILED(hr))
			return false;
	}

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	vector<CD3DX12_ROOT_PARAMETER> slotRootParameter;
	CD3DX12_ROOT_PARAMETER rpt;
	rpt.InitAsDescriptorTable(1, &texTable);
	slotRootParameter.push_back(rpt);
	for (int i(0); i < 2; i++)
	{
		CD3DX12_ROOT_PARAMETER rp;
		rp.InitAsConstantBufferView(i);
		slotRootParameter.push_back(rp);
	}

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(slotRootParameter.size(), slotRootParameter.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	if (FAILED(hr))
	{
		return false;
	}
	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr))
	{
		return false;
	}
	// create vertex and pixel shaders
	// compile vertex shader
	ID3DBlob* vertexShader; // d3d blob for holding vertex shader bytecode
	ID3DBlob* errorBuff; // a buffer holding the error data if any
	hr = D3DCompileFromFile(L"VertexShaderCBs.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}
	D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
	vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
	vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();
	// compile pixel shader
	ID3DBlob* pixelShader;
	hr = D3DCompileFromFile(L"PixelShaderCBs.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}
	D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
	pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();
	// create input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;
	// create a pipeline state object (PSO)
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
	psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
	psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
	psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
	psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
	DXGI_SAMPLE_DESC sDesc = {};
	sDesc.Count = 1;
	psoDesc.SampleDesc = sDesc; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
	psoDesc.NumRenderTargets = 1; // we are only binding one render target

	// create the pso
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
	if (FAILED(hr))
	{
		return false;
	}

	// Create vertex buffer

	// a triangle
	Vertex1 vList[] = {
		{ { 0.0f, 0.5f, 0.5f } },
		{ { 0.5f, -0.5f, 0.5f } },
		{ { -0.5f, -0.5f, 0.5f } },
	};

	int vBufferSize = sizeof(vList);

	// create default heap
	// default heap is memory on the GPU. Only the GPU has access to this memory
	// To get data into this heap, we will have to upload the data using
	// an upload heap
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&vertexBuffer));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// create upload heap
	// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
	// We will upload the vertex buffer using this heap to the default heap
	ID3D12Resource* vBufferUploadHeap;
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(commandList.Get(), vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// Now we execute the command list to upload the initial assets (triangle data)
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue[frameIndex]++;
	hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
	if (FAILED(hr))
		return false;

	// create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex1);
	vertexBufferView.SizeInBytes = vBufferSize;
	//


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

	commandList->SetPipelineState(pipelineStateObject); // set the root signature
	commandList->SetGraphicsRootSignature(rootSignature); // set the root signature

	cbufferCamera cbCam(XMMatrixIdentity(), XMMatrixIdentity(), XMMatrixIdentity());
	memcpy(bufferGpuAddress1[frameIndex] + (shift1 * 0), &cbCam, sizeof(cbufferCamera));
	commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeap1[frameIndex]->GetGPUVirtualAddress() + (shift1 * 0));

	cbufferLight cbDirLight(XMFLOAT4(), XMFLOAT4(), XMFLOAT4(), XMMatrixIdentity());
	memcpy(bufferGpuAddress2[frameIndex] + (shift2 * 0), &cbDirLight, sizeof(cbufferLight));
	commandList->SetGraphicsRootConstantBufferView(2, constantBufferUploadHeap2[frameIndex]->GetGPUVirtualAddress() + (shift2 * 0));

	commandList->RSSetViewports(1, &viewport); // set the viewports
	commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
	commandList->DrawInstanced(3, 1, 0, 0); // finally draw 3 vertices (draw the triangle)

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





