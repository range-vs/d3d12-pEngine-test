#include "model.h"

extern "C"
{
	__declspec(dllexport) unsigned int NvOptimusEnablement;
}


class GraphicOutput
{
	ComPtr<IDXGIOutput> output;
	vector<DXGI_MODE_DESC> supportedModes;
public:
	GraphicOutput(){}
	GraphicOutput(const ComPtr<IDXGIOutput>& o):output(o){}
	bool initGraphicOutput(const DXGI_FORMAT& format)
	{
		uint numberOfSupportedModes(-1);
		if (FAILED(output->GetDisplayModeList(format, 0, &numberOfSupportedModes, NULL)))
			return false;

		// set up array for the supported modes
		supportedModes.resize(numberOfSupportedModes);

		// fill the array with the available display modes
		if (FAILED(output->GetDisplayModeList(format, 0, &numberOfSupportedModes, supportedModes.data())))
			return false;

		return true;
	}

	ComPtr<IDXGIOutput>& getOutput() { return output; }
	vector<DXGI_MODE_DESC>& getSupportesModes() { return supportedModes; }
};

class GraphicAdapter
{
	ComPtr<IDXGIAdapter1> adapter;
	vector<D3D_FEATURE_LEVEL> supportedFeatureLevels;
	vector< GraphicOutput> adapterOutputs;
	D3D_FEATURE_LEVEL usingFeatureLevel;

	bool getAllOutputsForGraphicAdapter(const DXGI_FORMAT& format)
	{
		UINT i = 0;
		ComPtr < IDXGIOutput> output;
		while (adapter->EnumOutputs(i, output.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
		{
			GraphicOutput go(output);
			if (!go.initGraphicOutput(format))
				return false;
			adapterOutputs.push_back(go);
			++i;
		}
		return true;
	}

	void checkSupportGraphicAdaptersDirect3D(const array<D3D_FEATURE_LEVEL, featureLevelsSize>& featureLevels)
	{
		for (auto&& level : featureLevels)
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), level, _uuidof(ID3D12Device), nullptr)))
				supportedFeatureLevels.push_back(level);
	}

public:
	GraphicAdapter(){}
	GraphicAdapter(const ComPtr<IDXGIAdapter1>& a):adapter(a){}
	bool initGraphicAdapter(const DXGI_FORMAT& format, const array<D3D_FEATURE_LEVEL, featureLevelsSize>& featureLevels)
	{
		checkSupportGraphicAdaptersDirect3D(featureLevels);
		if (!getAllOutputsForGraphicAdapter(format))
			return false;
		return true;
	}

	ComPtr<IDXGIAdapter1>& getAdapter() { return adapter; }
	vector<D3D_FEATURE_LEVEL>& getSupportedFeatureLevels() { return supportedFeatureLevels; }
	vector< GraphicOutput>& getAdapterOutputs() { return adapterOutputs; }
	D3D_FEATURE_LEVEL& getUsingFeatureLevel() { return usingFeatureLevel; }

	void setUsingFeatureLevel(size_t i) { usingFeatureLevel = supportedFeatureLevels[i]; }
};


class Window
{
	HWND hwnd;
	LPCTSTR windowName;
	LPCTSTR windowTitle;
	int widthClient;
	int heightClient;
	int startWidth;
	int startHeight;
	int widthWindow;
	int heightWindow;

	int frameBufferCount;
	ComPtr<IDXGIFactory4> dxgiFactory;

	array<D3D_FEATURE_LEVEL, featureLevelsSize> featureLevels;
	vector<GraphicAdapter> adapters;
	int indexCurrentAdapter;
	bool isWarpAdapter;
	GraphicAdapter warpAdapter;

	vector<uint> msaaQuality;
	DXGI_FORMAT backBufferFormat;
	bool msaaState;
	ComPtr < ID3D12Device> device;
	ComPtr<IDXGISwapChain3> swapChain;
	ComPtr < ID3D12CommandQueue> commandQueue;
	ComPtr < ID3D12DescriptorHeap> rtvDescriptorHeap;
	vector< ComPtr<ID3D12Resource>> renderTargets; //  [frameBufferCount] ; // number of render targets equal to buffer count
	vector< ComPtr<ID3D12CommandAllocator>> commandAllocator; // [frameBufferCount] ; // we want enough allocators for each buffer * number of threads (we only have one thread)
	ComPtr < ID3D12GraphicsCommandList> commandList; // a command list we can record commands into, then execute them to render the frame
	vector< ComPtr<ID3D12Fence>> fence; // [frameBufferCount] ;    // an object that is locked while our command list is being executed by the gpu. We need as many 
											 //as we have allocators (more if we want to know when the gpu is finished with an asset)
	HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu
	vector<uint64> fenceValue; // [frameBufferCount] ; // this value is incremented each frame. each fence will have its own value
	int frameIndex; // current rtv we are on
	int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)
						   // function declarations
	D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.
	D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto
	ComPtr<ID3D12Resource> depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
	ComPtr<ID3D12DescriptorHeap> dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

	vector<ComPtr<ID3D12DescriptorHeap>> mainDescriptorHeap; // this heap will store the descripor to our constant buffer
	//ComPtr<ID3D12DescriptorHeap> mainDescriptorHeap; // this heap will store the descripor to our constant buffer

	shared_ptr<Camera> camera;
	TypeCamera typeCamera;
	CameraProperties camProp;
	map<char, bool> keyStatus;

	bool vsync;
	Color backBufferColor;
	bool modeWindow;
	bool isRunning;
	bool isInit;
	static Window* current;
	FPSCounter fpsCounter;

	ContainerShader shaders;

	ContainerTextures textures;
	size_t mCbvSrvDescriptorSize;

	map<wstring, PipelineStateObjectPtr> psos;
	vector<ModelPtr> models;
	map<wstring, size_t> modelsIndex;
	map<wstring, CBufferGeneralPtr> cBuffers;

	shared_ptr<SoundDevice> sndDevice;

	Sun sunLight;
	LightPoint lightCenter;
	Torch torch;

	GameParameters gameParam;

	ContainerMaterials materials;
	GameQualityPtr gameQuality;

	// test camera timing
	chrono::time_point<std::chrono::high_resolution_clock> timeFrameBefore;
	chrono::time_point<std::chrono::high_resolution_clock> timeFrameAfter;
	double countsPerSecond;

public:

	Window(int buffering, int vsync, bool msaaState, int w, int h, GameQualityConst gq = GameQualityConst::QUALITY_VERY_LOW,  TypeCamera tc = TypeCamera::STATIC_CAMERA, bool md = false)
	{
		gameQuality.reset(new GameQuality(gq));
		backBufferColor = { 0.f, 0.f, 0.f, 1.f }; // 0.53f
		hwnd = nullptr;
		windowName = L"dx12_className";
		windowTitle = L"DX12 Render Triangles";
		widthClient = w;
		heightClient = h;
		frameBufferCount = buffering;
		renderTargets.resize(buffering);
		commandAllocator.resize(buffering);
		fence.resize(buffering);
		fenceEvent = nullptr;
		fenceValue.resize(buffering);
		frameIndex = 0;
		rtvDescriptorSize = -1;
		this->vsync = vsync;
		featureLevels =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			/*D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1*/
		};
		backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		indexCurrentAdapter = -1;
		isWarpAdapter = false;
		this->msaaState = msaaState;
		current = this;
		modeWindow = md;
		mainDescriptorHeap.resize(buffering);
		typeCamera = tc;
		isRunning = false;
		isInit = false;
	}

	void updateSize()
	{
		RECT clientWindowRect;
		GetClientRect(hwnd, &clientWindowRect);
		widthClient = clientWindowRect.right;
		heightClient = clientWindowRect.bottom;
		startWidth = clientWindowRect.left;
		startHeight = clientWindowRect.top;
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
		widthWindow = windowRect.right - windowRect.left;
		heightWindow = windowRect.bottom - windowRect.top;
	}

	bool initializeWindow(HINSTANCE hInstance)
	{
		WNDCLASSEX wc;

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = NULL;
		wc.cbWndExtra = NULL;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(backBufferColor[r] * 255, backBufferColor[g] * 255, backBufferColor[b] * 255)));
		wc.lpszMenuName = NULL;
		wc.lpszClassName = windowName;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
		{
			MessageBox(NULL, L"Error registering class",
				L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		hwnd = CreateWindowEx(NULL,
			windowName,
			windowTitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			widthClient, heightClient,
			NULL,
			NULL,
			hInstance,
			NULL);

		if (!hwnd)
		{
			MessageBox(NULL, L"Error creating window",
				L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		updateSize();

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
		return true;
	}
	
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				DestroyWindow(hwnd);
				return 0;
			}
			else if (wParam == VK_HOME)
			{
				if (!Window::current->changeModeDisplay())
				{
					DestroyWindow(hwnd);
					return 0;
				}
			}
			else  if (Window::current->keyStatus.find((char)wParam) != Window::current->keyStatus.end())
				Window::current->keyStatus[(char)wParam] = true;
			break;
		}

		case WM_KEYUP:
		{
			if (Window::current->keyStatus.find((char)wParam) != Window::current->keyStatus.end())
				Window::current->keyStatus[(char)wParam] = false;
			if (!Window::current->keyStatus['L'])
				Window::current->torch.torchUsed(false);
			break;
		}

		case WM_SIZE:
		{
			Window::current->updateSize();
			if (Window::current->isInit)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					Window::current->isRunning = false;
					break;
				}
				else if (wParam == SIZE_RESTORED)
				{
					Window::current->isRunning = true;
				}
				if (!Window::current->resize(Window::current->widthWindow, Window::current->heightWindow))
				{
					PostQuitMessage(0);
					return 0;
				}
				Window::current->drawFrame();
			}
			break;
		}
		
		case WM_MOUSEMOVE: 
		{
			POINT p;
			LONG xPos;
			LONG yPos;
			if (GetCursorPos(&p))
			{
				xPos = p.x; 
				yPos = p.y;
			}
			if (Window::current->typeCamera == GAME_CAMERA)
			{
				RECT winRect;
				GetClientRect(hwnd, &winRect);
				int _x = ((winRect.right + winRect.left) / 2);
				int _y = ((winRect.bottom + winRect.top) / 2);
				SetCursorPos(_x, _y);
			}
			Window::current->camProp.mouseCurrState = { xPos, yPos };
			break;
		}

		case WM_DESTROY: 
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	bool initializeDirect3D()
	{
		HRESULT hr;

		//if (!enableDebugLayer())
		//	return false;
		if (!createFactory())
			return false;
		getAllGraphicAdapters();
		if (!tryCreateRealDevice())
			return false;
		/*if (!checkSupportMSAAQuality())
			return false;*/
		if (!createFence())
			return false;
		if (!createCommandQueue())
			return false;
		if (!createCommandAllocators())
			return false;
		if (!createCommandList())
			return false;
		createCamera();
		if (!createSwapChain())
			return false;

		// resize
		//isRunning = true;
		if (!setProperties())
			return false;
		if (!changeModeDisplay())
			return false;
		if (!resize(widthWindow, heightWindow))
			return false;

		// create PSOs
		if (!loadPSO(L"engine_resource/configs/pso.pCfg"))
			return false;

		// create light
		if (!loadMaterials())
			return false;
		if (!initLight())
			return false;

		// create model
		mCbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		if (!initializeSceneDirect3D())
			return false;
		if (!createBuffersDescriptorHeap())
			return false;

		// close command list
		commandList->Close();
		vector<ID3D12CommandList*> cmdsLists = { commandList.Get() };
		commandQueue->ExecuteCommandLists(cmdsLists.size(), cmdsLists.data());

		// create views buffer for all model (vb/ib)
		createBuffersViews();

		// fps counter
		fpsCounter.start();
		fpsCounter.setBeforeTime();

		// init snd
		if (!initSound())
			return false;
		// start snd
		startSound();

		// start!
		initGameParam();
		isInit = true;
		isRunning = true;

		return true;
	}

	bool enableDebugLayer()
	{
		ComPtr<ID3D12Debug> debugController;
		if(FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			return false;
		debugController->EnableDebugLayer();
		return true;
	}

	bool createFactory()
	{
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf()))))
			return false;
		return true;
	}

	bool getAllGraphicAdapters()
	{
		uint i = 0;
		ComPtr < IDXGIAdapter1> adapter;
		while (dxgiFactory->EnumAdapters1(i, adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
		{
			++i;
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) // пока из выборки исключается устройство WARP
				continue;

			GraphicAdapter ga(adapter);
			if (!ga.initGraphicAdapter(backBufferFormat, featureLevels))
				return false;
			adapters.push_back(ga);
		}
		return true;
	}

	bool checkSupportMSAAQuality()
	{
		for (uint i(2);; i *= 2)
		{
			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
			msQualityLevels.Format = backBufferFormat;
			msQualityLevels.SampleCount = i;
			msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
			msQualityLevels.NumQualityLevels = 0;
			if (FAILED(device->CheckFeatureSupport(
				D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
				&msQualityLevels,
				sizeof(msQualityLevels))))
				return false;
			if (msQualityLevels.NumQualityLevels < 1)
				break;
			msaaQuality.push_back(i);
		}
		return true;
	}

	bool tryCreateRealDevice()
	{
		int indexAdapter(0);
		for (auto&& adapter : adapters)
		{
			int indexFeatureLevel(0);
			for (auto&& level : adapter.getSupportedFeatureLevels())
			{
				if (SUCCEEDED(D3D12CreateDevice(adapter.getAdapter().Get(), level, IID_PPV_ARGS(device.GetAddressOf()))))
				{
					adapter.setUsingFeatureLevel(indexFeatureLevel);
					indexCurrentAdapter = indexAdapter;
					return true;
				}
				indexFeatureLevel++;
			}
			indexAdapter++;
		}
		return tryCreateWARPDevice();
	}

	bool tryCreateWARPDevice()
	{
		ComPtr<IDXGIAdapter1> warp;
		if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(warp.GetAddressOf()))))
			return false;
		warpAdapter = GraphicAdapter(warp);
		if (!warpAdapter.initGraphicAdapter(backBufferFormat, featureLevels))
			return false;
		int indexFeatureLevel(0);
		for (auto&& level : warpAdapter.getSupportedFeatureLevels())
		{
			HRESULT hardwareResult = D3D12CreateDevice(warp.Get(), level, IID_PPV_ARGS(device.GetAddressOf()));
			if (SUCCEEDED(hardwareResult))
			{
				isWarpAdapter = true;
				warpAdapter.setUsingFeatureLevel(indexFeatureLevel);
				return true;
			}
			indexFeatureLevel++;
		}
		return false;
	}

	bool createCommandQueue()
	{
		D3D12_COMMAND_QUEUE_DESC cqDesc = {};
		cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

		if (FAILED(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()))))
			return false;
		commandQueue->SetName(L"Command queue");
		return true;
	}

	bool createSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = 0;
		swapChainDesc.Height = 0;
		swapChainDesc.Format = backBufferFormat;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = frameBufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;

		auto output = !isWarpAdapter ? adapters[indexCurrentAdapter].getAdapterOutputs().at(0).getOutput().Get() : warpAdapter.getAdapterOutputs().at(0).getOutput().Get();
		ComPtr<IDXGISwapChain1> tempSwapChain;
		HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd,
			&swapChainDesc, nullptr,
			output, tempSwapChain.GetAddressOf());
		if (FAILED(hr))
			return false;

		swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain.Get());
		if (!swapChain)
			return false;
		frameIndex = swapChain->GetCurrentBackBufferIndex();

		DXGI_RGBA swapChainBkColor = { backBufferColor[r], backBufferColor[g], backBufferColor[b], backBufferColor[a]};
		if(FAILED(swapChain->SetBackgroundColor(&swapChainBkColor)))
			return false;

		return true;
	}

	bool getFrequencyOutput(ComPtr<IDXGIOutput>& outputs, pair<uint, uint>* p) // OLD. не уверен, нужно ли
	{
		std::pair<uint, uint> result = { 0, 0 };
		uint numModes(0);
		HRESULT hr = outputs->GetDisplayModeList(backBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numModes, 0);
		if (FAILED(hr))
			return false;
		vector< DXGI_MODE_DESC> displayModeList(numModes);
		hr = outputs->GetDisplayModeList(backBufferFormat, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data());
		if (FAILED(hr))
			return false;
		bool isStop(false);
		for (size_t i(0); i < numModes; i++)
		{
			if (displayModeList[i].Width == (uint)widthClient)
			{
				if (displayModeList[i].Height == (uint)heightClient)
				{
					uint numerator = displayModeList[i].RefreshRate.Numerator;
					uint denominator = displayModeList[i].RefreshRate.Denominator;
					*p = { numerator, denominator };
					isStop = true;
					break;
				}
				if (!isStop)
					break;
			}
		}
		return true;
	}

	bool createBackBuffer()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = frameBufferCount; // number of descriptors for this heap.
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

		// This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
		// otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvDescriptorHeap.GetAddressOf()))))
			return false;

		// get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors should be stored in it.
	// descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
	// device to give us the size. we will use this size to increment a descriptor handle offset
		rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// get a handle to the first descriptor in the descriptor heap. a handle is basically a Point2er,
		// but we cannot literally use it like a c++ Point2er.
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
		for (int i = 0; i < frameBufferCount; i++)
		{
			// first we get the n'th buffer in the swap chain and store it in the n'th
			// position of our ID3D12Resource array
			if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargets[i].GetAddressOf()))))
				return false;

			// the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
			device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);

			// we increment the rtv handle by the rtv descriptor size we got above
			rtvHandle.Offset(1, rtvDescriptorSize);
		}

		return true;
	}

	bool createCommandAllocators()
	{
		for (int i = 0; i < frameBufferCount; i++)
		{
			if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator[i].GetAddressOf()))))
				return false;
		}
		return true;
	}

	bool createCommandList()
	{
		if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex].Get(), NULL, IID_PPV_ARGS(commandList.GetAddressOf()))))
			return false;
		return true;
	}

	bool createFence()
	{
		for (int i = 0; i < frameBufferCount; i++)
		{
			if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence[i].GetAddressOf()))))
				return false;
			fenceValue[i] = 0; // set the initial fence value to 0
		}

		// create a handle to a fence event
		fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent == nullptr)
			return false;
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

	bool createDepthAndStencilBuffer()
	{
		// create a depth stencil descriptor heap so we can get a Point2er to the depth stencil buffer
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsDescriptorHeap.GetAddressOf()))))
			return false;

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		if(FAILED(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, widthClient, heightClient, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(depthStencilBuffer.GetAddressOf())
		)))
			return false;
		dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

		device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		return true;
	}

	bool setProperties()
	{
		if (FAILED(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)))
			return false;
		return true;
	}

	bool createBuffersDescriptorHeap()
	{
		if (textures.getSize() == 0)
			return true;
		for (int i = 0; i < frameBufferCount; ++i)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = textures.getSize();
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mainDescriptorHeap[i].GetAddressOf()))))
				return false;
			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = -1;
			size_t index(0);
			for (auto it(textures.getTexturesBuffer().begin()); it != textures.getTexturesBuffer().end(); ++it)
			{
				auto texture = it->second.getTexture();
				srvDesc.Format = texture->GetDesc().Format;
				device->CreateShaderResourceView(texture.Get(), &srvDesc, hDescriptor);
				Texture* _texture(nullptr);
				if (!textures.getTexture(it->first, &_texture))
					return false;
				_texture->setIndexSrvHeap(index++);
				// next descriptor
				hDescriptor.Offset(1, mCbvSrvDescriptorSize);
			}
		}

		//D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		//heapDesc.NumDescriptors = textures.getSize();
		//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mainDescriptorHeap.GetAddressOf()))))
		//	return false;
		//CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//srvDesc.Texture2D.MostDetailedMip = 0;
		//srvDesc.Texture2D.MipLevels = -1;
		//size_t index(0);
		//for (auto it(textures.getTexturesBuffer().begin()); it != textures.getTexturesBuffer().end(); ++it)
		//{
		//	auto texture = it->second.getTexture();
		//	srvDesc.Format = texture->GetDesc().Format;
		//	device->CreateShaderResourceView(texture.Get(), &srvDesc, hDescriptor);
		//	textures.getTexture(it->first).setIndexSrvHeap(index++);
		//	// next descriptor
		//	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
		//}

		return true;
	}

	bool loadPSO(const wstring cfg)
	{
		//load PSO
		PSOLoader psoLoader;
		bool status(false);
		auto psoProp = psoLoader.load(status, cfg);
		if (!status)
		{
			// error
			exit(1);
		}
		// go to create PSO (and load shaders, create cBuffers etc)
		CBufferCreator cBCreator;
		for (auto&& pP : psoProp)
		{
			// load shaders
			if(pP.pathShader)
			{ 
				pP.vsPath = L"engine_resource/shaders/" + gameQuality->getFolder() + L"/" + pP.vsPath;
				pP.psPath = L"engine_resource/shaders/" + gameQuality->getFolder() + L"/" + pP.psPath;
			}
			shaders.addVertexShader(pP.vsPath);
			shaders.addPixelShader(pP.psPath);
			// create cBuffers
			for_each(pP.cBuffNames.begin(), pP.cBuffNames.end(), [this, &cBCreator](const wstring& cBuffName)
				{
					vector<ComPtr<ID3D12DescriptorHeap>> stop;
					if (cBuffers.find(cBuffName) == cBuffers.end())
						cBuffers.insert({ cBuffName , cBCreator.createCBuffer(cBuffName, device, stop, frameBufferCount) });
				}
			);
			// create PSO
			PipelineStateObjectPtr _pso(new PipelineStateObject());
			for_each(pP.cBuffNames.begin(), pP.cBuffNames.end(), [&_pso](const wstring& key)
				{
					if(find(_pso->getCBufferKeys().begin(), _pso->getCBufferKeys().end(), key) == _pso->getCBufferKeys().end())
						_pso->addCBufferKey(key);
				}
			);
			for_each(pP.sBuffNames.begin(), pP.sBuffNames.end(), [&_pso](const wstring& key)
				{
					if (find(_pso->getSBufferKeys().begin(), _pso->getSBufferKeys().end(), key) == _pso->getSBufferKeys().end())
						_pso->addSBufferKey(key);
				}
			);
			if (!_pso->initPipelineStateObject(device, msaaState, msaaQuality, shaders, pP))
				return false;
			psos.insert({ pP.name, _pso });
		}
		return true;
	}


	void createCamera()
	{
		keyStatus = 
		{
			{'A', false},
			{'W', false},
			{'S', false},
			{'D', false},
			{'X', false},
		};
		Vector3 pos(20.f, 5.f, -100.f);
		Vector3 dir(25.f, 5.f, -100.f);
		switch (typeCamera)
		{
		case STATIC_CAMERA:
			camera.reset(new StaticCamera(pos, dir));
			break;

		case GAME_CAMERA:
			float step(0.3f);
			float run(0.5f);
			camera.reset(new GameCamera(step, run, pos, dir));
			ShowCursor(0);
			SetCapture(hwnd);
			SetCursorPos((widthClient + startWidth) / 2, (heightClient + startHeight) / 2);
			break;
		}
		camera->updateProjection(3.14159f / 4.f, widthClient / (FLOAT)heightClient, 0.01f, 1000.f);

		LARGE_INTEGER frequencyCount;
		QueryPerformanceFrequency(&frequencyCount);
		countsPerSecond = double(frequencyCount.QuadPart);
	}



	Point generateNormal(const Point& a, const Point& b, const Point& c)
	{
		Vector p0(a);
		Vector p1(b);
		Vector p2(c);
		Vector u = p1 - p0;
		Vector v = p2 - p0;
		Vector p = Vector::cross(u, v);
		p = Vector::Normalize(p);
		return (Point)p;
	}

	template<typename T, typename ...Vertexs>
	T genereateAverageNormal(int c, T sum, Vertexs& ...vert)
	{
		Point s = _genereateAverageNormal(sum, vert...);
		s *= 1.f / c;
		return s;
	}

	template<typename T, typename... Vertexs>
	T _genereateAverageNormal(T sum, Vertexs&... vert)
	{
		return sum + _genereateAverageNormal(vert...);
	}

	template<typename T>
	T _genereateAverageNormal(T p)
	{
		return p;
	}



	bool initializeSceneDirect3D() 
	{
		array<Color, 2> colors =
		{
			Color(0.f, .49f, .05f, 1.f),
			Color(0.41f, 0.24f, 0.49f, 1.f)
		};
		Matrix4 trans;

		//{
		//	// cube color
		//	BlankModel bm;
		//	bm.nameModel = L"cube color";
		//	float value(5.f);
		//	vector<Vector3> _v = {
		//		 Vector3(-value, value, -value), // top ( против часовой, с самой нижней левой)
		//		 Vector3(value, value, -value) ,
		//		 Vector3(value, value, value),
		//		 Vector3(-value, value, value),
		//		 Vector3(-value, -value, -value), // bottom ( против часовой, с самой нижней левой)
		//		 Vector3(value, -value, -value),
		//		 Vector3(value, -value, value),
		//		 Vector3(-value, -value, value)
		//	};
		//	vector<Point> norm = {
		//generateNormal(_v[0], _v[1], _v[4]), // near 0
		//generateNormal(_v[5], _v[1], _v[2]), // right 1
		//generateNormal(_v[6], _v[2], _v[3]), // far 2
		//generateNormal(_v[7], _v[3], _v[0]), // left 3
		//generateNormal(_v[0], _v[3], _v[2]), // top 4 
		//generateNormal(_v[7], _v[4], _v[5]), // bottom 5
		//	};
		//	vector<Point> real_norm = {
		//genereateAverageNormal(3, norm[0], norm[3], norm[4]),  // top ( против часовой, с самой нижней левой)
		//genereateAverageNormal(3, norm[0], norm[1], norm[4]),
		//genereateAverageNormal(3, norm[2], norm[1], norm[4]),
		//genereateAverageNormal(3, norm[2], norm[3], norm[4]),
		//genereateAverageNormal(3, norm[0], norm[3], norm[5]),  // bottom ( против часовой, с самой нижней левой)
		//genereateAverageNormal(3, norm[0], norm[1], norm[5]),
		//genereateAverageNormal(3, norm[2], norm[1], norm[5]),
		//genereateAverageNormal(3, norm[2], norm[3], norm[5]),
		//	};
		//	bm.vertexs = {
		//		Vertex(_v[0],real_norm[0],  colors[1]),
		//		Vertex(_v[1],real_norm[1],  colors[1]),
		//		Vertex(_v[2],real_norm[2],  colors[1]),
		//		Vertex(_v[3],real_norm[3],  colors[1]),
		//		Vertex(_v[4],real_norm[4],  colors[1]),
		//		Vertex(_v[5],real_norm[5],  colors[1]),
		//		Vertex(_v[6] ,real_norm[6],  colors[1]),
		//		Vertex(_v[7],real_norm[7],  colors[1])
		//	};
		//	bm.indexs = {
		//		3,1,0,
		//	2,1,3,
		//	0,5,4,
		//	1,5,0,
		//	3,4,7,
		//	0,4,3,
		//	1,6,5,
		//	2,6,1,
		//	2,7,6,
		//	3,7,2,
		//	6,4,5,
		//	7,4,6
		//	};
		//	bm.shiftIndexs.push_back(bm.indexs.size());
		//	bm.texturesName.push_back(Model::isColor()); // color
		//	bm.materials.push_back(L"default");
		//	bm.doubleSides.push_back(false);
		//	bm.psoName = L"PSOLightPhong";

		//	array<pair<float, float>, 4> coord = { pair<float, float>{-15.f, -15.f}, pair<float, float>{-15.f, 15.f}, pair<float, float>{15.f, -15.f}, pair<float, float>{15.f, 15.f} };
		//	for (int i(0), y(10); i < 3; i++, y += 20)
		//	{
		//		for (int j(0); j < coord.size(); j++)
		//		{
		//			auto world1(Matrix4::CreateTranslationMatrixXYZ(coord[j].first, (float)y, coord[j].second) * Matrix::CreateTranslationMatrixX(-80.f));
		//			if (!createModel(bm, world1))
		//				return false;
		//		}
		//	}
		//}

		//{ 
		//	// plane grass
		//	BlankModel bm;
		//	bm.nameModel = L"plane grass";
		//	bm.doubleSides.push_back(false);
		//	bm.texturesName.push_back(L"engine_resource/textures/pGrass.dds");
		//	bm.indexs =
		//	{
		//		0, 3, 2,
		//		0, 2, 1
		//	};
		//	bm.shiftIndexs.push_back(bm.indexs.size());
		//	bm.vertexs =
		//	{
		//		Vertex({-10000.f,  0.f,  -10000.f}, {0,1,0},  {-129,130,0,-1}),
		//		Vertex({10000.f,  0.f,  -10000.f},{0,1,0}, {130,130,0,-1}),
		//		Vertex({10000.f,  0.f, 10000.f},{0,1,0}, {130,-129,0,-1}),
		//		Vertex({-10000.f,  0.f,  10000.f},{0,1,0}, {-129,-129,0,-1})
		//	};
		//	bm.psoName = L"PSOLightBlinnPhong";
		//	bm.materials.push_back(L"default");
		//	if (!createModel(bm))
		//		return false;
		//}

		//{
		//	// cube wood
		//	BlankModel bm;
		//	float value = 10.f;
		//	vector<Vector3> _v = {
		//		 Vector3(-value, value, -value), // top ( против часовой, с самой нижней левой)
		//		 Vector3(value, value, -value) ,
		//		 Vector3(value, value, value),
		//		 Vector3(-value, value, value),
		//		 Vector3(-value, -value, -value), // bottom ( против часовой, с самой нижней левой)
		//		 Vector3(value, -value, -value),
		//		 Vector3(value, -value, value),
		//		 Vector3(-value, -value, value)
		//	};
		//	vector<Point> norm = {
		//generateNormal(_v[0], _v[1], _v[4]), // near
		//generateNormal(_v[5], _v[1], _v[2]), // right
		//generateNormal(_v[6], _v[2], _v[3]), // far
		//generateNormal(_v[7], _v[3], _v[0]), // left
		//generateNormal(_v[0], _v[3], _v[2]), // top
		//generateNormal(_v[7], _v[4], _v[5]), // bottom
		//	};
		//	vector<Point> real_norm = {
		//		genereateAverageNormal(3, norm[0], norm[3], norm[4]),  // top ( против часовой, с самой нижней левой)
		//		genereateAverageNormal(3, norm[0], norm[1], norm[4]),
		//		genereateAverageNormal(3, norm[2], norm[1], norm[4]),
		//		genereateAverageNormal(3, norm[2], norm[3], norm[4]),
		//		genereateAverageNormal(3, norm[0], norm[3], norm[5]),  // bottom ( против часовой, с самой нижней левой)
		//		genereateAverageNormal(3, norm[0], norm[1], norm[5]),
		//		genereateAverageNormal(3, norm[2], norm[1], norm[5]),
		//		genereateAverageNormal(3, norm[2], norm[3], norm[5]),
		//	};
		//	bm.vertexs = {
		//		Vertex(_v[0], real_norm[0], {0.35f, 1.f, 0, -1}),
		//		Vertex(_v[1], real_norm[1],{0.63f, 1.f, 0, -1}),
		//		Vertex(_v[2],real_norm[2], {0.63f, 0.75f, 0, -1}),
		//		Vertex(_v[3], real_norm[3],{0.35f, 0.75f, 0, -1}),
		//		Vertex(_v[7], real_norm[7],{0.12f, 0.75f, 0, -1}),
		//		Vertex(_v[4], real_norm[4],{0.12f, 1.f, 0, -1}),
		//		Vertex(_v[5], real_norm[5],{0.85f, 1.f, 0, -1}),
		//		Vertex(_v[6], real_norm[6],{0.85f, 0.75f, 0, -1}),
		//		Vertex(_v[7], real_norm[7],{0.35f, 0.5f, 0, -1}),
		//		Vertex(_v[6], real_norm[6],{0.63f, 0.5f, 0, -1}),
		//		Vertex(_v[4],real_norm[4],{0.35f, 0.25f, 0, -1}),
		//		Vertex(_v[5], real_norm[5],{0.63f, 0.25f, 0, -1}),
		//		Vertex(_v[0],real_norm[0], {0.35f, 0.f, 0, -1}),
		//		Vertex(_v[1], real_norm[1],{0.63f, 0.f, 0, -1})
		//	};
		//	bm.indexs = {
		//		10,12,11,
		//		11,12,13,
		//		8,10,9,
		//		9,10,11,
		//		3,8,2,
		//		2,8,9,
		//		5,4,0,
		//		0,4,3,
		//		0,3,1,
		//		1,3,2,
		//		1,2,6,
		//		6,2,7
		//	};
		//	bm.shiftIndexs.push_back(bm.indexs.size());
		//	bm.nameModel = L"cube wood";
		//	bm.texturesName.push_back(L"engine_resource/textures/pWoodDoski.dds");
		//	bm.psoName = L"PSOLightBlinnPhong";
		//	bm.materials.push_back(L"default");
		//	bm.doubleSides.push_back(false);
		//	float val(20);
		//	array<pair<float, float>, 4> coord = { pair<float, float>{-val, -val}, pair<float, float>{-val, val}, pair<float, float>{val, -val}, pair<float, float>{val, val} };
		//	for (int i(0), y(10); i < 3; i++, y += 30)
		//	{
		//		for (int j(0); j < coord.size(); j++)
		//		{
		//			auto world1(Matrix4::CreateTranslationMatrixXYZ(coord[j].first, (float)y, coord[j].second));
		//			if (!createModel(bm, world1))
		//				return false;
		//		}
		//	}
		//}

		//{
		//	// plane grass, wood and cirpic
		//	BlankModel bm;
		//	bm.nameModel = L"plane grass, cirpic and wood";
		//	bm.texturesName.push_back(L"engine_resource/textures/pHouse.dds");
		//	bm.texturesName.push_back(L"engine_resource/textures/pWoodDesc.dds");
		//	bm.materials.push_back(L"default");
		//	bm.materials.push_back(L"default");
		//	bm.doubleSides.push_back(true);
		//	bm.doubleSides.push_back(true);
		//	bm.indexs =
		//	{
		//		5, 7, 6,
		//		5, 6, 4,

		//		9, 11, 10,
		//		9, 10, 8,
		//	};
		//	bm.shiftIndexs.push_back(6);
		//	bm.shiftIndexs.push_back(6);
		//	bm.vertexs =
		//	{
		//		Vertex({-100.f,  0.f,  -250.f}, {0,1,0},  {-2,3,0,-1}),
		//		Vertex({50.f,  0.f,  -250.f},{0,1,0}, {3,3,0,-1}),
		//		Vertex({50.f,  0.f, -50.f},{0,1,0}, {3,-2,0,-1}),
		//		Vertex({-100.f,  0.f,  -50.f},{0,1,0}, {-2,-2,0,-1}),

		//		Vertex({50.f,  0.f,  -150.f},{-1,0,0}, {2, 2 ,0,-1}),
		//		Vertex({50.f,  0.f, -50.f},{-1,0,0}, {-2, 2,0,-1}),
		//		Vertex({50.f,  100.f,  -150.f},{-1,0,0}, {2,-2,0,-1}),
		//		Vertex({50.f,  100.f, -50.f},{-1,0,0}, {-2,-2,0,-1}),

		//		Vertex({50.f,  100.f,  -150.f},{-1,0,0}, {1, 1 ,0,-1}), // 8
		//		Vertex({50.f,  100.f, -50.f},{-1,0,0}, {0, 1,0,-1}), // 9
		//		Vertex({50.f,  200.f,  -150.f},{-1,0,0}, {1,0,0,-1}), // 10
		//		Vertex({50.f,  200.f, -50.f},{-1,0,0}, {0,0,0,-1}) // 11
		//	};
		//	bm.psoName = L"PSOLightBlinnPhong";
		//	if (!createModel(bm))
		//		return false;
		//}

		//{
		//	// dimes roft
		//	BlankModel bm;
		//	bm.nameModel = L"dimes-test-rofl";
		//	bm.psoName = L"PSOLightBlinnPhong";
		//	if (!loadModelFromFile("model_dimes/dimes.pro_object", bm))
		//		return false;
		//	auto transform = Matrix::CreateTranslationMatrixXYZ(-50.f, 0.f, -200.f);
		//	if (!createModel(bm, transform))
		//		return false;
		//}

		//{
		//	// houses
		//	vector<BlankModel> bm(3);
		//	vector<string> namesModel = { "model_houses/house1.pro_object", "model_houses/house2.pro_object", "model_houses/house3.pro_object" };
		//	for (int i(0); i< bm.size(); i++)
		//	{
		//		bm[i].nameModel = L"house " + to_wstring(i) + L" instansing";
		//		bm[i].psoName = L"PSOLightBlinnPhongInstansed";
		//		if (!loadModelFromFile(namesModel[i], bm[i]))
		//			return false;
		//	}
		//	srand(time(nullptr));
		//	vector<vector<sbufferInstancing>> worlds(3);
		//	for (int x(-5000); x < 5000; x+=250)
		//	{
		//		for (int z(-5000); z < 5000; z += 250)
		//		{
		//			int indexModel = 0 + rand() % 3;
		//			if ((x >= -150 && x <= 150) || (z >= -150 && z <= 150))
		//				continue;
		//			array<float, 7> degrees = { 0.f, 90.f, 180.f, 270.f, -90.f, -180.f, -270.f };
		//			int indexRotate = 0 + rand() % degrees.size();
		//			float rotate = degrees[indexRotate];
		//			ModelPosition world(Matrix::CreateRotateMatrixY(GeneralMath::degreesToRadians(rotate)) * Matrix4::CreateTranslationMatrixXYZ(x, 0, z));
		//			worlds[indexModel].push_back({ world.getWorld().getGPUMatrix(), world.getNormals().getGPUMatrix() });
		//		}
		//	}
		//	for (int i(0); i < bm.size(); i++)
		//	{
		//		bm[i].instansing = true;
		//		bm[i].instansingBuffer = std::move(worlds[i]);
		//		if (!createModel(bm[i]))
		//			return false;
		//	}
		//}

	/*	{
			BlankModel bm;
			string pathModel = "engine_resource/models/statics/test.pObject";
			if (!ModelLoader::loadFileFromExpansion(pathModel, &bm))
				return false;
			bm.nameModel = wstring(pathModel.begin(), pathModel.end());
			if (!createModel(bm))
				return false;
		}*/

		{
			if (!preloadObjFile("engine_resource/models/statics/d.obj", L"PSOLightBlinnPhong"))
				return false;
		}

		return true;
	}

	bool preloadObjFile(const string& path, const wstring& pso)
	{
		BlankModel bm;
		if (!ModelLoader::loadFileFromExpansion(path, &bm))
			return false;
		bm.psoName = pso;
		bm.nameModel = wstring(path.begin(), path.end());
		wstring firstMaterial;
		if (!materials.getFirst(firstMaterial))
		{
			// TODO: error
			return false;
		}
		for (auto&& t : bm.texturesName)
			bm.materials.push_back(firstMaterial);
		if (!createModel(bm))
			return false;
		return true;
	}

	bool createModel(BlankModel& data, Matrix4 world = Matrix4::Identity())
	{
		// todo: потом прикрутить фабрику для создания моделей
		ModelPtr model;
		wstring _isColor(1, ModelConstants::isColor);
		if (modelsIndex.find(data.nameModel) == modelsIndex.end())
		{
			for (auto&& t : data.texturesName)
			{
				if (t == _isColor)
					continue;
				if (!data.texturesName.empty())
					if (!textures.addTexture(device, commandList, t))
						return false;
			}
		}

		if (data.instansing)
		{
			ModelInstancing* m = new ModelInstancing;
			if(!m->addStructuredBuffer(device, mainDescriptorHeap, frameBufferCount, L"sbInstancedPosition", data.instansingBuffer))
				return false;
			m->setCountInstanced(data.instansingBuffer.size());
			model.reset(std::move(m));
		}
		else
			model.reset(new ModelIdentity);

		if(modelsIndex.find(data.nameModel) == modelsIndex.end())
		{
			if (!model->createModel(device, data))
				return false;
			model->translationBufferForHeapGPU(commandList);
			model->addWorld(world);
			models.push_back(model);
			modelsIndex.insert({ data.nameModel, models.size() - 1 });
		}
		else
		{
			size_t index = modelsIndex[data.nameModel];
			models[index]->addWorld(world);
		}
		return true;
	}

	void createBuffersViews()
	{
		for (auto&& model : models)
		{
			model->createVertexBufferView();
			model->createIndexBufferView();
		}
	}

	template <class T>
	Vector_3<T> direct3DVector3ToOpenGLVector3(const Vector_3<T>& v)
	{
		return { v[x], v[y], -v[z] };
	}

	bool initSound()
	{
		sndDevice.reset(new SoundDevice);
		Vector3 pos = direct3DVector3ToOpenGLVector3(camera->getPosition());
		Vector3 tar = direct3DVector3ToOpenGLVector3(camera->getTarget());
		Vector3 up = direct3DVector3ToOpenGLVector3(camera->getUp());
		if (!sndDevice->init(pos, tar, up, 0.f))
			return false;
		if (!sndDevice->addSound( L"engine_resource/sounds/rnd_night_1.wav", { 0.f, 0.f, 0.f }, STATIC_SOUND, true, false, 50.f))
			return false;
		if (!sndDevice->addSound(L"engine_resource/sounds/step.wav", { 0.f, 0.f, 0.f }, ACTOR_SOUND, false, false, 0.5f))
			return false;
		if (!sndDevice->addSound(L"engine_resource/sounds/run.wav", { 0.f, 0.f, 0.f }, ACTOR_SOUND, false, false, 0.5f))
			return false;
		if (!sndDevice->addSound(L"engine_resource/sounds/ambient2.wav", { 0.f, 0.f, 0.f }, AMBIENT_SOUND, true, false, 0.1f))
			return false;
		if (!sndDevice->addSound(L"engine_resource/sounds/heavy_wind_2.wav", direct3DVector3ToOpenGLVector3(Vector3(-90.f, 0.f, 90.f)), STATIC_SOUND, true, false, 40.f))
			return false;
		if (!sndDevice->addSound(L"engine_resource/sounds/torch-on.wav", { 0.f, 0.f, 0.f }, ACTOR_SOUND, false, false, 0.6f))
			return false;
		if (!sndDevice->addSound(L"engine_resource/sounds/torch-off.wav", { 0.f, 0.f, 0.f }, ACTOR_SOUND, false, false, 0.6f))
			return false;
		return true;
	}

	void startSound()
	{
		sndDevice->start();
	}



	bool loadMaterials()
	{
		MaterialLoader mLoader;
		bool status;
		auto materialsBlank = mLoader.load(status, L"engine_resource/configs/materials.pCfg");
		if (!status)
			return false;
		for (auto& m : materialsBlank)
		{
			Material mat(
				{ m.ambient[x], m.ambient[y], m.ambient[z], m.ambient[w] },
				{ m.diffuse[x], m.diffuse[y], m.diffuse[z], m.diffuse[w] },
				{ m.specular[x] , m.specular[y], m.specular[z], m.specular[w] },
				m.shininess);
			if (!materials.addMaterial(m.name, mat))
				OutputDebugString(wstring(L"The material \"" + m.name + L"\" already exists. Ignoring.").c_str());
		}
		return true;
	}

	bool initLight()
	{
		sunLight.init(Color(1.f, 1.f, 0.776f, 1.f), { -350.f, 0.f, 0.f, 0.f }, 5);
		vector<Vertex> v;
		vector<Index> ind;
		float r = 10;
		int count_usech = 25;
		int count_h = 25;
		Color cl = { 1.f, 0.952f, 0.04f, 1 };
		if (r < 3) r = 3;
		if (count_usech < 3) count_usech = 3;
		if (count_h < 3) count_h = 3;
		count_h++;
		int latitudeBands = count_usech; // количество широтных полос
		int longitudeBands = count_h; // количество полос долготы
		float radius = r; // радиус сферы
		for (float latNumber = 0; latNumber <= latitudeBands; latNumber++)
		{
			float theta = latNumber * GeneralMath::PI / latitudeBands;
			float sinTheta = sin(theta);
			float cosTheta = cos(theta);

			for (float longNumber = 0; longNumber <= longitudeBands; longNumber++)
			{
				float phi = longNumber * 2 * GeneralMath::PI / longitudeBands;
				float sinPhi = sin(phi);
				float cosPhi = cos(phi);

				float xn = cosPhi * sinTheta;   // x
				float yn = cosTheta;            // y
				float zn = sinPhi * sinTheta;   // z

				v.push_back({ Point(r * xn, r * yn, r * zn), Point(xn, yn, zn), cl });
			}
		}
		for (int latNumber(0); latNumber < latitudeBands; latNumber++)
		{
			for (int longNumber(0); longNumber < longitudeBands; longNumber++)
			{
				int first = (latNumber * (longitudeBands + 1)) + longNumber;
				int second = first + longitudeBands + 1;
				ind.push_back(first + 1);
				ind.push_back(second);
				ind.push_back(first);
				ind.push_back(first + 1);
				ind.push_back(second + 1);
				ind.push_back(second);
			}
		}
		BlankModel bm;
		wstring nameSun = L"sun";
		bm.nameModel = nameSun;
		bm.indexs = ind;
		bm.shiftIndexs.push_back(ind.size());
		bm.vertexs = v;
		bm.psoName = L"PSONoLight";
		wstring _isColor(1, ModelConstants::isColor);
		bm.texturesName.push_back(_isColor);
		bm.doubleSides.push_back(false);
		bm.materials.push_back(L"default");
		auto sunPos = sunLight.getStartPosition();
		if (!createModel(bm, Matrix4::CreateTranslationMatrixXYZ(sunPos[x], sunPos[y], sunPos[z])))
			return false;
		sunLight.setModel(nameSun);

		lightCenter = LightPoint({ 1.f, 0.549f, 0.f, 1.f }, { 0.f, 10.f, 0.f, 0.f });

		torch.init({ 1.f, 1.f, 1.f, 1.f }, {}, {}, 30.5f, 40.5f);
		keyStatus.insert({ 'L', false });

		return true;
	}

	void initGameParam()
	{
		gameParam.cBuffers = &cBuffers;
		gameParam.camera = camera.get();
		gameParam.lightCenter = &lightCenter;
		gameParam.sunLight = &sunLight;
		gameParam.torch = &torch;
		gameParam.materials = &materials;
	}




	void runWindowLoop()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		while (true)
		{
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
				{
					if (!drawFrame())
						break;
				}
				else
					Sleep(100);
			}
		}
	}

	bool drawFrame()
	{
		if (!updateCamera())
			return false;
		updateSound(); // update sound
		if (!updateScene()) // update the game logic
			return false;
		if (!render()) // execute the command queue (rendering the scene is the result of the gpu executing the command lists)
			return false;
		return true;
	}

	double GetFrameTime() // ПОМЕТКА НА УДАЛЕНИЕ
	{
		LARGE_INTEGER currentTime;
		__int64 tickCount;
		QueryPerformanceCounter(&currentTime);

		static long long frameTimeOld(0.l);
		tickCount = currentTime.QuadPart - frameTimeOld;
		frameTimeOld = currentTime.QuadPart;

		if (tickCount < 0.0f)
			tickCount = 0.0f;

		//OutputDebugString(std::to_wstring(tickCount).c_str());
		//OutputDebugString(L"\n");
		//OutputDebugString(std::to_wstring(countsPerSecond).c_str());
		//OutputDebugString(L"\n");

		return float(tickCount) / this->countsPerSecond;
	} 

	bool updateCamera()
	{
		RECT winRect;
		GetWindowRect(hwnd, &winRect);
		int _x = (widthClient + startWidth) / 2;
		int _y = (heightClient + startHeight) / 2;
		Point2 mDelta(camProp.mouseCurrState[x] - _x, camProp.mouseCurrState[y] - _y);
		camera->updateCamera(keyStatus, mDelta, 
			/*GetFrameTime()*/
			std::chrono::duration_cast<std::chrono::nanoseconds>(timeFrameAfter -timeFrameBefore).count() / countsPerSecond);


		return true;
	}

	void updateSound()
	{
		Vector3 pos = direct3DVector3ToOpenGLVector3(camera->getPosition());
		Vector3 tar = direct3DVector3ToOpenGLVector3(camera->getTarget());
		Vector3 up = direct3DVector3ToOpenGLVector3(camera->getUp());
		sndDevice->updateListener(pos, tar, up);
		sndDevice->update(AMBIENT_SOUND, L"engine_resource/sounds/ambient2.wav", pos);
		if (keyStatus['W'] || keyStatus['A'] || keyStatus['S'] || keyStatus['D'])
		{
			wstring nameSnd = keyStatus['X'] ? L"engine_resource/sounds/run.wav" : L"engine_resource/sounds/step.wav";
			sndDevice->update(ACTOR_SOUND, nameSnd.c_str(), pos);
			sndDevice->play(ACTOR_SOUND, nameSnd.c_str());
		}
		if (keyStatus['L'] && !torch.isTorchUsed())
		{
			wstring nameSnd;
			if (torch.isEnable())
			{
				nameSnd = L"engine_resource/sounds/torch-off.wav";
				torch.disable();
			}
			else
			{
				nameSnd = L"engine_resource/sounds/torch-on.wav";
				torch.enable();
			}
			torch.torchUsed(true);
			sndDevice->update(ACTOR_SOUND, nameSnd.c_str(), pos);
			sndDevice->play(ACTOR_SOUND, nameSnd.c_str());
		}
	}

	bool updateScene()
	{
		if (torch.isEnable())
		{
			auto targ = camera->getTarget();
			torch.update((Vector)camera->getPosition(), (Vector)(targ - camera->getPosition()));
		}

		bool status;
		Matrix newPosSun = sunLight.update(status);
		if(status)
			models[modelsIndex[sunLight.getNameModel()]]->editWorld(newPosSun, 0);
		gameParam.view = camera->getView().getGPUMatrix();
		gameParam.proj = camera->getProjection().getGPUMatrix();
		size_t countAllCb(0);
		size_t countMaterialsCb(0);
		gameParam.countAllCb = &countAllCb;
		gameParam.countMaterialsCb = &countMaterialsCb;
		gameParam.torchProperty = { torch.getLight().getCutOff(), torch.getLight().getOuterCutOff(), (torch.isEnable() ? 1.f : 0.f), 1 };
		gameParam.frameIndex = frameIndex;

		for (int i(0); i < models.size(); i++)
		{
			models[i]->updateConstantBuffers(gameParam);
		}

		return true;
	}

	bool updatePipeline()
	{
		HRESULT hr;

		// We have to wait for the gpu to finish with the command allocator before we reset it
		if (!waitForPreviousFrame())
			return false;

		// we can only reset an allocator once the gpu is done with it
		// resetting an allocator frees the memory that the command list was stored in
		hr = commandAllocator[frameIndex]->Reset();
		if (FAILED(hr))
			return false;

		// reset the command list. by resetting the command list we are putting it into
		// a recording state so we can start recording commands into the command allocator.
		// the command allocator that we reference here may have multiple command lists
		// associated with it, but only one can be recording at any time. Make sure
		// that any other command lists associated to this command allocator are in
		// the closed state (not recording).
		// Here you will pass an initial pipeline state object as the second parameter,
		// but in this tutorial we are only clearing the rtv, and do not actually need
		// anything but an initial default pipeline, which is what we get by setting
		// the second parameter to NULL

		hr = commandList->Reset(commandAllocator[frameIndex].Get(), nullptr);
		if (FAILED(hr))
			return false;

		// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

		// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		// here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
		// get a handle to the depth/stencil buffer
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		// set the render target for the output merger stage (the output of the pipeline)
		commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		// Clear the render target by using the ClearRenderTargetView command
		commandList->ClearRenderTargetView(rtvHandle, backBufferColor.toArray().data(), 0, nullptr);

		// clearing depth/stencil buffer
		commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// draw triangle
		commandList->RSSetViewports(1, &viewport); // set the viewports
		commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects

		// set the descriptor heap
		if (textures.getSize() != 0)
		{
			vector<ID3D12DescriptorHeap*> descriptorHeaps = { mainDescriptorHeap[frameIndex].Get() };
			//vector<ID3D12DescriptorHeap*> descriptorHeaps = { mainDescriptorHeap.Get() };
			commandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());
		}

		size_t countAllCb(0);
		size_t countMaterialsCb(0);
		for (int i(0); i < models.size(); i++)
		{
			auto pso = psos[models[i]->getPsoKey()];
			models[i]->draw(pso, commandList, cBuffers, textures, frameIndex, countAllCb, countMaterialsCb, mainDescriptorHeap[frameIndex], mCbvSrvDescriptorSize);
		}

		// transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a
		// warning if present is called on the render target when it's not in the present state
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		hr = commandList->Close();
		if (FAILED(hr))
			return false;

		return true;
	}

	bool render()
	{
		HRESULT hr;
		timeFrameBefore = std::chrono::high_resolution_clock::now();

		if (!updatePipeline()) // update the pipeline by sending commands to the commandqueue
			return false;

		// create an array of command lists (only one command list here)
		vector<ID3D12CommandList*> ppCommandLists = { commandList.Get() };
		commandQueue->ExecuteCommandLists(ppCommandLists.size(), ppCommandLists.data());	

		// this command goes in at the end of our command queue. we will know when our command queue 
		// has finished because the fence value will be set to "fenceValue" from the GPU since the command
		// queue is being executed on the GPU
		hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
		if (FAILED(hr))
			return false;

		// present the current backbuffer
		hr = swapChain->Present(vsync ? 1 : 0, 0);
		if (FAILED(hr))
			return false;

		++fpsCounter;
		fpsCounter.setAfterTime();
		if (fpsCounter.isReady())
		{
			updateFps();
			fpsCounter.setBeforeTime();
		}

		timeFrameAfter = std::chrono::high_resolution_clock::now();
		return true;
	}

	bool resize(int w, int h)
	{
		if (device && swapChain && commandAllocator[frameIndex])
		{
			HRESULT hr;
			flushGpu();
			for (int i(0); i < frameBufferCount; i++)
				renderTargets[i].Reset();
			depthStencilBuffer.Reset();
			hr = swapChain->ResizeBuffers(frameBufferCount, w, h, backBufferFormat, NULL);
			if (FAILED(hr))
				return false;
			createBackBuffer();
			if (!createDepthAndStencilBuffer())
				return false;
			createViewport();
			createScissor();
			camera->updateProjection(0.4f * 3.14f, widthClient / (FLOAT)heightClient, 1.f, 1000.f);
			return true;
		}
		return false;
	}

	bool changeModeDisplay()
	{		
		HRESULT hr;
		auto output = !isWarpAdapter ? adapters[indexCurrentAdapter].getAdapterOutputs().at(0).getOutput().Get() : warpAdapter.getAdapterOutputs().at(0).getOutput().Get();
		hr = swapChain->SetFullscreenState(modeWindow, modeWindow ? output: nullptr);
		if (FAILED(hr))
			return false;
		modeWindow = !modeWindow;
		return true;
	}

	void updateFps()
	{
		SetWindowText(hwnd, wstring(L"Current FPS: " + to_wstring(fpsCounter.getFps())).c_str());
	}

	bool waitForPreviousFrame()
	{
		HRESULT hr;

		// swap the current rtv buffer index so we draw on the correct buffer
		frameIndex = swapChain->GetCurrentBackBufferIndex();

		// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
		// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
		if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
		{
			// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
			hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
			if (FAILED(hr))
				return false;

			// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
			// has reached "fenceValue", we know the command queue has finished executing
			WaitForSingleObject(fenceEvent, INFINITE);
		}

		// increment fenceValue for next frame
		fenceValue[frameIndex]++;
		return true;
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

	~Window()
	{
		CloseHandle(fenceEvent);
	}

};

Window* Window::current(nullptr);

int WINAPI WinMain(HINSTANCE hInstance,  HINSTANCE hPrevInstance, LPSTR lpCmdLine,  int nShowCmd)
{
	// create the window
	shared_ptr<Window>  window(new Window(3, 0, 1, 900, 650, GameQualityConst::QUALITY_LOW, TypeCamera::GAME_CAMERA));
	if (!window->initializeWindow(hInstance))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return -1;
	}

	// initialize direct3d
	if (!window->initializeDirect3D())
	{
		MessageBox(0, L"Failed to initialize direct3d 12", L"Error", MB_OK);
		return -1;
	}

	//// init scene d3d 12
	//if (!window->initializeSceneDirect3D())
	//{
	//	MessageBox(0, L"Failed to initialize scene for direct3d 12", L"Error", MB_OK);
	//	return -1;
	//}

	// start the main loop
	window->runWindowLoop();

	// we want to wait for the gpu to finish executing the command list before we start releasing everything
	window->waitForPreviousFrame();

	return 0;
}


// разобраться, как менять разрешение экрана - это потом

// игровая камера:
// разобраться с синхронизацией по времени - движение и вращение немного зависит от фпс
// присед, полный присед - реализовать
// msaa:  разобраться, как включать




// звук - звуки статические - всегда делать СТЕРЕО
// для того, чтобы cb появился в шейдере, надо его использовать в шейдере обязательно


// TODO: камера
// input lag - изучить проблему
// прочитать стаью, реализовать очередь задержки?
// вынести опрос input в отдельный поток
// скачать пример из input lag и проверить, как там реализована очередь задержки