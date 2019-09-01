#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"
#include <wrl.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <array>
#include <memory>
#include <ctime>
#include <fstream>
#include <functional>
#include <sstream>
#include <regex>
#include <set>

#include "cBufferFactory.h"
#include "PSOLoader.h"
#include "types.h"

#include "DDSTextureLoader.h"

#include "pSound/sound.h"

using namespace std;
using Microsoft::WRL::ComPtr;

//#include <DirectXMath.h>
//using namespace DirectX;

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "D3Dcompiler.lib")
#pragma comment(lib, "d3d12.lib")


//extern "C"
//{
//	__declspec(dllexport) unsigned int NvOptimusEnablement;
//}


enum Sizes
{
	featureLevelsSize = 4
};

class MatrixCPUAndGPU
{
	Matrix4 _m;
	Matrix4 _mT;

	void createTransponse()
	{
		_mT = Matrix4::Transponse(_m);
	}

public:
	MatrixCPUAndGPU() :_m(Matrix::Identity()), _mT(Matrix::Identity()) {}

	MatrixCPUAndGPU(const Matrix4& m) :_m(m)
	{
		createTransponse();
	}

	Matrix4& getCPUMatrix()
	{
		return _m;
	}

	Matrix4& getGPUMatrix()
	{
		return _mT;
	}

	void updateCPUMatrix(const Matrix4& m)
	{
		_m = m;
		createTransponse();
	}

};


enum TypeCamera: int
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
	Camera(){}
	Camera(const Vector3& p, const Vector3& d)
	{
		updateView(p, d, hCamStandart);
	}
	Camera(const Matrix4& v, const Matrix4& p):view(v), projection(p){}
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
			currentProperties.camYaw += (mDelta[x] * 0.001f);
			currentProperties.camPitch += (mDelta[y] * 0.001f);
			currentProperties.lastPosMouse = currentProperties.mouseCurrState;
		}

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




class TypeShaders
{
public:
	static wstring vs;
	static wstring ps;
};

wstring TypeShaders::vs(L"vs");
wstring TypeShaders::ps(L"ps");

class PairShaders
{
	wstring pShader;

protected:
	wstring vShader;

public:
	PairShaders(){}
	PairShaders(const wstring& vs, const wstring& ps):vShader(vs), pShader(ps){}
	wstring& getVertexShaderPath() { return vShader; }
	virtual wstring& getPixelShaderPath() { return pShader; }
};

class CombinedPairShaders : public PairShaders
{
public:
	CombinedPairShaders(const wstring& s)
	{
		vShader = s;
	}
	wstring& getPixelShaderPath() override { return vShader; }

};

class Shader
{
	ComPtr<ID3DBlob> _shader;
	D3D12_SHADER_BYTECODE _shaderBytecode;
	
public:
	bool initShader(const wstring& path, const wstring& type)
	{
		if (!createVertexShader(path, type))
			return false;
		createVertexShaderByteCode();
		return true;
	}

	bool createVertexShader(wstring path, wstring type)
	{
		ComPtr<ID3DBlob> errorBuff; // a buffer holding the error data if any
		string verison = string(type.begin(), type.end()) + "_5_0";
		HRESULT hr = D3DCompileFromFile((path + L'.' + type).c_str(),
			nullptr,
			nullptr,
			"main",
			verison.c_str(),
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&_shader,
			&errorBuff);
		if (FAILED(hr))
		{
			OutputDebugStringA((char*)errorBuff->GetBufferPointer());
			return false;
		}
		return true;
	}

	void createVertexShaderByteCode()
	{
		_shaderBytecode.BytecodeLength = _shader->GetBufferSize();
		_shaderBytecode.pShaderBytecode = _shader->GetBufferPointer();
	}

	D3D12_SHADER_BYTECODE& getShaderByteCode() { return _shaderBytecode; }

};

class ContainerShader // container shaders of auto loading shaders
{
	map<pair<wstring, wstring>, Shader> shaders;

	void loadShader(const wstring& path, const wstring& type)
	{
		Shader newShader;
		if (!newShader.initShader(path, type))
		{
			MessageBox(NULL, L"Error load shader", L"Error!", MB_OK);
			exit(1);
		}
		shaders.insert({ {path, type}, newShader });
	}

public:
	Shader& getVertexShader(const wstring& path)
	{
		if (shaders.find({ path, TypeShaders::vs }) != shaders.end())
			return shaders[{path, TypeShaders::vs}];
		// error!
	}

	Shader& getPixelShader(const wstring& path)
	{
		if (shaders.find({ path, TypeShaders::ps }) != shaders.end())
			return shaders[{path, TypeShaders::ps}];
		// error!
	}

	void addVertexShader(const wstring& path)
	{
		if (shaders.find({ path, TypeShaders::vs }) == shaders.end())
			loadShader(path, TypeShaders::vs);
	}

	void addPixelShader(const wstring& path)
	{
		if (shaders.find({ path, TypeShaders::ps }) == shaders.end())
			loadShader(path, TypeShaders::ps);
	}

};

class Texture
{
	ComPtr<ID3D12Resource> texture;
	ComPtr<ID3D12Resource> textureUploadHeap;
	size_t indexSrvHeap;
public:
	bool loadTexture(ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& cmdList, const wstring& filename)
	{
		if (FAILED(DirectX::CreateDDSTextureFromFile12(device.Get(),
			cmdList.Get(), filename.c_str(),
			texture, textureUploadHeap)))
			return false;
		return true;
	}
	ComPtr<ID3D12Resource>& getTexture() { return texture; }
	ComPtr<ID3D12Resource>& getTextureUploadHeap() { return textureUploadHeap; }
	void setIndexSrvHeap(size_t index) { indexSrvHeap = index; }
	size_t& getIndexSrvHeap() { return indexSrvHeap; }
};

class ContainerTextures
{
	map<wstring, Texture> textures;
	size_t count;

public:
	ContainerTextures():count(0){}

	static array<const CD3DX12_STATIC_SAMPLER_DESC, 6> getStaticSamplers()
	{
		// Applications usually only need a handful of samplers.  So just define them all up front
		// and keep them available as part of the root signature.  

		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
			0.0f,                             // mipLODBias
			8);                               // maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
			0.0f,                              // mipLODBias
			8);                                // maxAnisotropy

		return {
			pointWrap, pointClamp,
			linearWrap, linearClamp,
			anisotropicWrap, anisotropicClamp };
	}


	bool addTexture(ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& cmdList, const wstring& filename)
	{
		if (textures.find(filename) == textures.end())
		{
			Texture t; 
			if (!t.loadTexture(device, cmdList, filename))
				return false;
			textures.insert({ filename, t });
			count++;
		}
		return true;
	}

	bool getTexture(const wstring& filename, Texture** texture)
	{
		if (textures.find(filename) == textures.end())
		{
			// error
			return false;
		}
		*texture = &textures[filename];
		return true;
	}

	size_t& getSize() { return count; }

	map<wstring, Texture>& getTexturesBuffer() { return textures; }
};

class PipelineStateObject
{
	ComPtr<ID3D12PipelineState> pipelineStateObject; // pso containing a pipeline state
	ComPtr<ID3D12RootSignature> rootSignature; // root signature defines data shaders will access
	vector< D3D12_INPUT_ELEMENT_DESC> inputLayout;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	set<wstring> cBufferKeys;

public:

	bool initPipelineStateObject(ComPtr<ID3D12Device>& device, bool msaaState, vector<uint>& msaaQuality, ContainerShader& shaders, const PiplineStateObjectProperties& psoProp)
	{
		if (!createInputAssembler(device, psoProp))
			return false;
		if (!createPipelineStateObject(device, msaaState, msaaQuality, shaders, psoProp))
			return false;
		return true;
	}

	bool createInputAssembler(ComPtr<ID3D12Device>& device, const PiplineStateObjectProperties& psoProp)
	{
		if (!createRootSignature(device))
			return false;
		createInputLayout(psoProp);
		return true;
	}

	bool createRootSignature(ComPtr<ID3D12Device>& device)
	{
		//vector<D3D12_DESCRIPTOR_RANGE>  descriptorTableRanges(cBufferKeys.size()); // only one range right now
		//for (int i(0); i < cBufferKeys.size();i++)
		//{
		//	descriptorTableRanges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
		//	descriptorTableRanges[i].NumDescriptors = i + 1; // we only have one constant buffer, so the range is only 1
		//	descriptorTableRanges[i].BaseShaderRegister = 0; // start index of the shader registers in the range
		//	descriptorTableRanges[i].RegisterSpace = 0; // space 0. can usually be zero
		//	descriptorTableRanges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables
		//}
		//D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
		//descriptorTable.NumDescriptorRanges = descriptorTableRanges.size(); // we only have one range
		//descriptorTable.pDescriptorRanges = descriptorTableRanges.data(); // the Point2er to the beginning of our ranges array
		//
		//vector<D3D12_ROOT_PARAMETER>  rootParameters(1); // only one parameter right now
		//rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
		//rootParameters[0].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
		//rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now


		// набор константных буферов
		vector<D3D12_ROOT_DESCRIPTOR> rootCBVDescriptor(cBufferKeys.size());
		for (int i(0); i < cBufferKeys.size(); i++)
		{
			rootCBVDescriptor[i].RegisterSpace = i;
			rootCBVDescriptor[i].ShaderRegister = 0;
		}

		// пока PSO держит одну текстуру
		CD3DX12_DESCRIPTOR_RANGE texTable;
		texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		// создаем массив всех буферов
		vector<CD3DX12_ROOT_PARAMETER> slotRootParameter;
		CD3DX12_ROOT_PARAMETER rpt;
		rpt.InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter.push_back(rpt);
		for (int i(0); i < cBufferKeys.size(); i++)
		{
			CD3DX12_ROOT_PARAMETER rp;
			rp.InitAsConstantBufferView(i);
			slotRootParameter.push_back(rp);
		}

		// create a static sampler
		/*D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;*/

		auto staticSamplers = ContainerTextures::getStaticSamplers();

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(slotRootParameter.size(), // we have 1 root parameter
			slotRootParameter.data(), // a Point2er to the beginning of our root parameters array
			staticSamplers.size(),
			staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr)))
			return false;

		if (FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()))))
			return false;

		return true;
	}

	void createInputLayout(const PiplineStateObjectProperties& psoProp)
	{
		for (auto&& ia : psoProp.inputLayouts)
		{
			D3D12_INPUT_ELEMENT_DESC desc = { ia.semantics.c_str(), 0, static_cast<DXGI_FORMAT>(ia.countBytes), 0, ia.shiftBytes, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
			inputLayout.push_back(desc);
		}

		// we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
		inputLayoutDesc.NumElements = inputLayout.size();
		inputLayoutDesc.pInputElementDescs = inputLayout.data();
	}

	bool createPipelineStateObject(ComPtr<ID3D12Device>& device, bool msaaState, vector<uint>& msaaQuality, ContainerShader& shaders, const PiplineStateObjectProperties& psoProp)
	{
		// TODO: no msaa!
		DXGI_SAMPLE_DESC sDesc = {};
		sDesc.Count = 1;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
		psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
		psoDesc.pRootSignature = rootSignature.Get(); // the root signature that describes the input data this pso needs
		psoDesc.VS = shaders.getVertexShader(psoProp.vsPath).getShaderByteCode(); // structure describing where to find the vertex shader bytecode and how large it is
		psoDesc.PS = shaders.getPixelShader(psoProp.psPath).getShaderByteCode(); // same as VS but for pixel shader
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
		psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means Point2 sampling is done
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
		psoDesc.NumRenderTargets = 1; // we are only binding one render target
		psoDesc.SampleDesc = sDesc;
		/*psoDesc.RasterizerState.MultisampleEnable = msaaState;
		psoDesc.SampleDesc.Count = msaaState ? msaaQuality[0] : 1;
		psoDesc.SampleDesc.Quality = msaaState ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN : 0;*/
		psoDesc.DepthStencilState  = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state;

		// create the pso
		if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineStateObject.GetAddressOf()))))
			return false;
		return true;
	}

	ComPtr<ID3D12PipelineState>& getPipelineStateObject() { return pipelineStateObject; }
	ComPtr<ID3D12RootSignature>& getRootSignature() { return rootSignature; }
	void addCBufferKey(const wstring& keyCB)
	{
		this->cBufferKeys.insert(keyCB);
	}
	set<wstring>& getCBufferKeys() { return cBufferKeys; }
};

using PipelineStateObjectPtr = shared_ptr< PipelineStateObject>;

class Model
{
	ComPtr <ID3D12Resource> vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
	ComPtr < ID3D12Resource> indexBuffer; // ind buffer
	ComPtr<ID3D12Resource> vBufferUploadHeap;
	ComPtr<ID3D12Resource> iBufferUploadHeap;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	size_t vBufferSize;
	size_t iBufferSize;
	D3D12_SUBRESOURCE_DATA vertexData;
	D3D12_SUBRESOURCE_DATA indexData;
	size_t countIndex;
	size_t countParts;

	wstring psoKey;
	wstring textureName;
	vector<MatrixCPUAndGPU> worlds;

public:
	Model():vBufferSize(0)
	{
		vertexBufferView = {};
		vertexData = {};
	}

	bool createModel(ComPtr<ID3D12Device>& device, BlankModel& data)
	{
		//initVariables(data.size());
		textureName = data.textureName;
		if (!createBuffer(device, data.vertexs, vertexBuffer, vBufferSize, L"reserved name vb - todo"))
			return false;
		if (!createUploadBuffer(device, vBufferUploadHeap, vBufferSize, vertexData, data.vertexs, L"reserved name vb upload - todo"))
			return false;
		countIndex = data.indexs.size();
		if (!createBuffer(device, data.indexs, indexBuffer, iBufferSize, L"reserved name ib - todo"))
			return false;
		if (!createUploadBuffer(device, iBufferUploadHeap, iBufferSize, indexData, data.indexs, L"reserved name ib upload - todo"))
			return false;
		psoKey = data.psoName;
		return true;
	}

	/*void initVariables(size_t s)
	{
		countParts = s;
		vertexBuffer.resize(s);
		indexBuffer.resize(s);
		vBufferUploadHeap.resize(s);
		iBufferUploadHeap.resize(s);
		vertexBufferView.resize(s);
		indexBufferView.resize(s);
		vBufferSize.resize(s);
		iBufferSize.resize(s);
		vertexData.resize(s);
		indexData.resize(s);
		countIndex.resize(s);
	}*/

	template<class TypeBuffer>
	bool createBuffer(ComPtr<ID3D12Device>& device, vector< TypeBuffer>& buff, ComPtr < ID3D12Resource>& buffGpu, size_t& buffSize, const wstring& name)
	{
		buffSize = sizeof(TypeBuffer) * buff.size();
		if (FAILED(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(buffSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
											// from the upload heap to this heap
			nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(buffGpu.GetAddressOf()))))
			return false;
		// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
		buffGpu->SetName(name.c_str());
		return true;
	}

	template<class TypeBuffer>
	bool createUploadBuffer(ComPtr<ID3D12Device>& device, ComPtr<ID3D12Resource>& uploadBuffer, size_t& sizeBuffer, D3D12_SUBRESOURCE_DATA& subData, 
		vector<TypeBuffer>& buff, const wstring& name)
	{
		if (FAILED(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(sizeBuffer), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf()))))
			return false;
		uploadBuffer->SetName(name.c_str());

		// store vertex buffer in upload heap
		subData.pData = reinterpret_cast<BYTE*>(buff.data()); // Point2er to our vertex array
		subData.RowPitch = sizeBuffer; // size of all our triangle vertex data
		subData.SlicePitch = sizeBuffer; // also the size of our triangle vertex data
		return true;
	}

	void createVertexBufferView()
	{
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = vBufferSize;
	}

	void createIndexBufferView()
	{
		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = iBufferSize;
	}

	ComPtr < ID3D12Resource>& getVertexBuffer() { return vertexBuffer; }
	ComPtr < ID3D12Resource>& getUploadVertexBuffer() { return vBufferUploadHeap; }
	ComPtr < ID3D12Resource>& getIndexBuffer() { return indexBuffer; }
	ComPtr < ID3D12Resource>& getUploadIndexBuffer() { return iBufferUploadHeap; }
	D3D12_SUBRESOURCE_DATA& getSubresourceVertexData() { return vertexData; }
	D3D12_SUBRESOURCE_DATA& getSubresourceIndexData() { return indexData; }
	D3D12_VERTEX_BUFFER_VIEW& getVertexBufferView() { return vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW& getIndexBufferView() { return indexBufferView; }
	size_t& getCountIndex() {return countIndex;}
	void addWorld(const Matrix4& m) { MatrixCPUAndGPU _m(m); worlds.push_back(_m); }
	void addWorld(const MatrixCPUAndGPU& m) { worlds.push_back(m); }
	vector<MatrixCPUAndGPU>& getWorlds() { return worlds; }
	size_t& getCountParts() { return countParts; }
	wstring& getPsoKey() { return psoKey; }
	wstring& getTextureName() { return textureName; }
};

using ModelPtr = shared_ptr<Model>;

class FPSCounter;

class SystemTimer
{
	clock_t oldTime;
	clock_t newTime;
	clock_t allTime;

	void clear()
	{
		oldTime = newTime = allTime = 0;
	}

	friend class FPSCounter;
public:
	SystemTimer()
	{
		clear();
	}

	void start()
	{
		clear();
	}

	void setBeforeTime()
	{
		oldTime = clock();
	}

	void setAfterTime()
	{
		newTime = clock();
		allTime = newTime - oldTime;
	}

	bool isSecond() const
	{
		return allTime >= 1000.f;
	}

	bool isSecond_1_10() const
	{
		return allTime >= 100.f;
	}

	bool isSecond_1_100() const
	{
		return allTime >= 10.f;
	}

	float getCurrentTime()
	{
		return allTime;
	}

};

class FPSCounter
{
	SystemTimer timerFps;
	size_t frameCount;

	void clear()
	{
		timerFps.clear();
		frameCount = 0;
	}
public:
	FPSCounter()
	{
		clear();
	}

	void start()
	{
		clear();
	}

	void setBeforeTime()
	{
		timerFps.setBeforeTime();
	}

	void setAfterTime()
	{
		timerFps.setAfterTime();
	}

	bool isReady() const
	{
		return timerFps.allTime >= 500.f;
	}

	size_t getFps()
	{
		size_t fps = 1000 * frameCount / timerFps.allTime;
		clear();
		return fps;
	}

	FPSCounter& operator++()
	{
		frameCount++;
		return *this;
	}
};

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
	int width;
	int height;

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
	float r;
	float g;
	float b;
	float a;
	bool modeWindow;
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

public:

	Window(int buffering, int vsync, bool msaaState, int w, int h, TypeCamera tc = TypeCamera::STATIC_CAMERA, bool md = false)
	{
		r = g = b = 0.53f;
		a = 1.f;
		hwnd = nullptr;
		windowName = L"dx12_className";
		windowTitle = L"DX12 Render Triangles";
		width = w;
		height = h;
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
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH); // (HBRUSH)(CreateSolidBrush(RGB(r * 255, g * 255, b * 255)));
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
			width, height,
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
			break;
		}

		case WM_SIZE:
		{
			RECT rect;
			if (GetClientRect(hwnd, &rect))
			{
				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				if (!Window::current->resize(width, height))
				{
					DestroyWindow(hwnd);
					return 0;
				}
			}
			else
			{
				DestroyWindow(hwnd);
				return 0;
			}
			break;
		}
		
		case WM_MOUSEMOVE: 
		{
			SetCursorPos(Window::current->width / 2, Window::current->height / 2);
			LONG xPos = LOWORD(lParam);
			LONG yPos = HIWORD(lParam);
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

		/*if (!enableDebugLayer())
			return false;*/
		if (!createFactory())
			return false;
		getAllGraphicAdapters();
		if (!tryCreateRealDevice())
			return false;
		if (!checkSupportMSAAQuality())
			return false;
		if (!createCommandQueue())
			return false;

		createCamera();
		if (!createSwapChain())
			return false;
		// replace mode screen
		ShowWindow(hwnd, SW_SHOW);
		if (modeWindow)
		{
			ShowCursor(0);
			SetCapture(hwnd);
		}
		UpdateWindow(hwnd);
		if (!changeModeDisplay())
			return false;

		if (!createBackBuffer())
			return false;
		if (!createCommandAllocators())
			return false;
		if (!createCommandList())
			return false;
		if (!createFence())
			return false;
		createViewport();
		createScissor();
		if (!createDepthAndStencilBuffer())
			return false;
		if (!setProperties())
			return false;

		// create PSOs
		if (!loadPSO(L"pso.pCfg"))
			return false;

		// create model
		mCbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		if (!initializeSceneDirect3D())
			return false;
		if (!createBuffersDescriptorHeap())
			return false;

		// close command list
		commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// create views buffer for all model (vb/ib)
		createBuffersViews();

		fpsCounter.start();
		fpsCounter.setBeforeTime();

		// init snd
		if (!initSound())
			return false;
		// start snd
		startSound();

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

		/*DXGI_RGBA swapChainBkColor = { r, g, b, a };
		if(FAILED(swapChain->SetBackgroundColor(&swapChainBkColor)))
			return false;*/

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
			if (displayModeList[i].Width == (uint)width)
			{
				if (displayModeList[i].Height == (uint)height)
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
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
	}

	void createScissor()
	{
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = width;
		scissorRect.bottom = height;
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
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
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
		//Ss load PSO
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
					_pso->getCBufferKeys().insert(key);
				}
			);
			if (!_pso->initPipelineStateObject(device, msaaState, msaaQuality, shaders, pP))
				return false;
			psos.insert({ pP.name, _pso });
		}

		//CBufferCreator cBCreator;
		//shaders.addVertexShader(L"shader");
		//shaders.addPixelShader(L"shader");
		//cBuffers.insert({ L"cbCamera" , cBCreator.createCBuffer(L"cbCamera", device, mainDescriptorHeap, frameBufferCount) });
		//PiplineStateObjectProperties pP;
		//pP.inputLayouts.push_back({ "POSITION", 6, 0 });
		//pP.inputLayouts.push_back({ "NORMAL", 6, 12 });
		//pP.inputLayouts.push_back({ "COLOR", 6, 24 });
		//pP.name = L"PSOModelColorNoLight";
		//pP.number = 0;
		//pP.vsPath = L"shader";
		//pP.psPath = L"shader";
		//pP.cBuffNames.push_back(L"cbCamera");
		//if (!createPso(pP))
		//	return false;

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
		Vector3 pos(0.f, 5.f, -8.f);
		Vector3 dir(0.0f, 0.0f, 0.0f);
		switch (typeCamera)
		{
		case STATIC_CAMERA:
			camera.reset(new StaticCamera(pos, dir));
			break;

		case GAME_CAMERA:
			float step(15.f);
			float run(30.f);
			camera.reset(new GameCamera(step, run, pos, dir));
			break;
		}
		camera->updateProjection(3.14159f / 4.f, width / (FLOAT)height, 0.01f, 1000.f);
	}


	bool initializeSceneDirect3D() 
	{
		// cube color
		array<Color, 2> colors =
		{
			Color(0.f, .49f, .05f, 1.f),
			Color(0.41f, 0.24f, 0.49f, 1.f)
		};
		BlankModel bm;
		bm.nameModel = L"cube color";
		float value(5.f);
		vector<Vector3> _v = {
			 Vector3(-value, value, -value), // top ( против часовой, с самой нижней левой)
			 Vector3(value, value, -value) ,
			 Vector3(value, value, value),
			 Vector3(-value, value, value),
			 Vector3(-value, -value, -value), // bottom ( против часовой, с самой нижней левой)
			 Vector3(value, -value, -value),
			 Vector3(value, -value, value),
			 Vector3(-value, -value, value)
		};
		bm.vertexs = {
			Vertex(_v[0],{0,0,0},  colors[1]),
			Vertex(_v[1],{0,0,0},  colors[1]),
			Vertex(_v[2],{0,0,0},  colors[1]),
			Vertex(_v[3],{0,0,0},  colors[1]),
			Vertex(_v[4],{0,0,0},  colors[1]),
			Vertex(_v[5],{0,0,0},  colors[1]),
			Vertex(_v[6] ,{0,0,0},  colors[1]),
			Vertex(_v[7], {0,0,0},  colors[1])
		};
		bm.indexs= {
			3,1,0,
		2,1,3,
		0,5,4,
		1,5,0,
		3,4,7,
		0,4,3,
		1,6,5,
		2,6,1,
		2,7,6,
		3,7,2,
		6,4,5,
		7,4,6
		};
		bm.psoName = L"PSOModelColorNoLight";
		auto world1(Matrix4::CreateTranslationMatrixXYZ(-14, 10, 0));
		if (!createModel(bm, world1))
			return false;

		world1 = Matrix4::CreateTranslationMatrixXYZ(-28, 8, 0);
		if (!createModel(bm, world1))
			return false;

		// plane grass
		bm.nameModel = L"plane grass";
		bm.textureName = L"pGrass.dds";
		bm.indexs =
		{
			0, 3, 2,
			0, 2, 1
		};
		bm.vertexs =
		{
			Vertex({-50.f,  0.f,  -50.f}, {0,0,0},  {-1,2,0,0}),
			Vertex({50.f,  0.f,  -50.f},{0,0,0}, {2,2,0,0}),
			Vertex({50.f,  0.f, 50.f},{0,0,0}, {2,-1,0,0}),
			Vertex({-50.f,  0.f,  50.f},{0,0,0}, {-1,-1,0,0})
		};
		bm.psoName = L"PSOModelSingleTextureNoLight";
		if (!createModel(bm))
			return false;

		// plane desc
		bm.nameModel = L"plane wood";
		bm.textureName = L"pWoodDesc.dds";
		Matrix4 trans = Matrix4::CreateTranslationMatrixX(-100);
		if (!createModel(bm, trans))
			return false;


		// cube desc
		value = 2.f;
		_v = {
			 Vector3(-value, value, -value), // top ( против часовой, с самой нижней левой)
			 Vector3(value, value, -value) ,
			 Vector3(value, value, value),
			 Vector3(-value, value, value),
			 Vector3(-value, -value, -value), // bottom ( против часовой, с самой нижней левой)
			 Vector3(value, -value, -value),
			 Vector3(value, -value, value),
			 Vector3(-value, -value, value)
		};
		bm.vertexs = {
			Vertex(_v[0], {0,0,0,0}, {0.35f, 1.f, 0}),
			Vertex(_v[1], {0,0,0,0},{0.63f, 1.f, 0}),
			Vertex(_v[2],{0,0,0,0}, {0.63f, 0.75f, 0}),
			Vertex(_v[3], {0,0,0,0},{0.35f, 0.75f, 0}),
			Vertex(_v[7], {0,0,0,0},{0.12f, 0.75f, 0}),
			Vertex(_v[4], {0,0,0,0},{0.12f, 1.f, 0}),
			Vertex(_v[5], {0,0,0,0},{0.85f, 1.f, 0}),
			Vertex(_v[6], {0,0,0,0},{0.85f, 0.75f, 0}),
			Vertex(_v[7], {0,0,0,0},{0.35f, 0.5f, 0}),
			Vertex(_v[6], {0,0,0,0},{0.63f, 0.5f, 0}),
			Vertex(_v[4], {0,0,0,0},{0.35f, 0.25f, 0}),
			Vertex(_v[5], {0,0,0,0},{0.63f, 0.25f, 0}),
			Vertex(_v[0],{0,0,0,0}, {0.35f, 0.f, 0}),
			Vertex(_v[1], {0,0,0,0},{0.63f, 0.f, 0})
		};
		// generate index
		bm.indexs = {
			10,12,11,
			11,12,13,
			8,10,9,
			9,10,11,
			3,8,2,
			2,8,9,
			5,4,0,
			0,4,3,
			0,3,1,
			1,3,2,
			1,2,6,
			6,2,7
		};
		bm.nameModel = L"cube wood";
		bm.textureName = L"pWoodDoski.dds";
		trans = Matrix4::CreateTranslationMatrixXYZ(25, 5, -13);
		if (!createModel(bm, trans))
			return false;
		trans = Matrix4::CreateTranslationMatrixXYZ(25, 20, -13);
		if (!createModel(bm, trans))
			return false;
		trans = Matrix4::CreateTranslationMatrixXYZ(25, 40, -13);
		if (!createModel(bm, trans))
			return false;

		return true;
	}

	bool createModel(BlankModel& data, Matrix4 world = Matrix4::Identity())
	{
		ModelPtr model(new Model());
		if (modelsIndex.find(data.nameModel) == modelsIndex.end())
		{
			if(!data.textureName.empty())
				if (!textures.addTexture(device, commandList, data.textureName))
					return false;
			if (!model->createModel(device, data))
				return false;
			translationVertexBufferForHeapGPU(model->getVertexBuffer(), model->getUploadVertexBuffer(), model->getSubresourceVertexData());
			translationVertexBufferForHeapGPU(model->getIndexBuffer(), model->getUploadIndexBuffer(), model->getSubresourceIndexData());
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

	void translationVertexBufferForHeapGPU(ComPtr<ID3D12Resource>& buffer, ComPtr<ID3D12Resource>& bufferUpload, D3D12_SUBRESOURCE_DATA& subData)
	{
		UpdateSubresources(commandList.Get(), buffer.Get(), bufferUpload.Get(), 0, 0, 1, &subData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
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
		if (!sndDevice->init(pos, tar, up))
			return false;
		if (!sndDevice->addSound( L"rnd_night_1.wav", { 0.f, 0.f, 0.f }, STATIC_SOUND, true, false, 50.f))
			return false;
		if (!sndDevice->addSound(L"step.wav", { 0.f, 0.f, 0.f }, ACTOR_SOUND, false, false, 0.5f))
			return false;
		if (!sndDevice->addSound(L"run.wav", { 0.f, 0.f, 0.f }, ACTOR_SOUND, false, false, 0.5f))
			return false;
		if (!sndDevice->addSound(L"ambient2.wav", { 0.f, 0.f, 0.f }, AMBIENT_SOUND, true, false, 0.1f))
			return false;
		if (!sndDevice->addSound(L"heavy_wind_2.wav", direct3DVector3ToOpenGLVector3(Vector3(-90.f, 0.f, 90.f)), STATIC_SOUND, true, false, 40.f))
			return false;
		return true;
	}

	void startSound()
	{
		sndDevice->start();
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
				// run game code
				if (!updateCamera())
					break;
				updateSound(); // update sound
				if (!updateScene()) // update the game logic
					break;
				if (!render()) // execute the command queue (rendering the scene is the result of the gpu executing the command lists)
					break;
			}
		}
	}

	double GetFrameTime()
	{
		LARGE_INTEGER currentTime;
		__int64 tickCount;
		QueryPerformanceCounter(&currentTime);

		static long long frameTimeOld(0.l);
		tickCount = currentTime.QuadPart - frameTimeOld;
		frameTimeOld = currentTime.QuadPart;

		static long long countsPerSecond = 10000000l;

		if (tickCount < 0.0f)
			tickCount = 0.0f;

		//OutputDebugString(std::to_wstring(tickCount).c_str());
		//OutputDebugString(L"\n");
		//OutputDebugString(std::to_wstring(countsPerSecond).c_str());
		//OutputDebugString(L"\n");

		return float(tickCount) / countsPerSecond;
	}

	bool updateCamera()
	{
		Point2 mDelta(camProp.mouseCurrState[x] - (width / 2), camProp.mouseCurrState[y] - (height / 2));
		camera->updateCamera(keyStatus, mDelta, GetFrameTime());

		return true;
	}

	void updateSound()
	{
		Vector3 pos = direct3DVector3ToOpenGLVector3(camera->getPosition());
		Vector3 tar = direct3DVector3ToOpenGLVector3(camera->getTarget());
		Vector3 up = direct3DVector3ToOpenGLVector3(camera->getUp());
		sndDevice->updateListener(pos, tar, up);
		if (keyStatus['W'] || keyStatus['A'] || keyStatus['S'] || keyStatus['D'])
		{
			wstring nameSnd = keyStatus['X'] ? L"run.wav" : L"step.wav";
			sndDevice->update(ACTOR_SOUND, nameSnd.c_str(), pos);
			sndDevice->play(ACTOR_SOUND, nameSnd.c_str());
		}
		sndDevice->update(AMBIENT_SOUND, L"ambient2.wav", pos);
	}

	bool updateScene()
	{
		auto view = camera->getView().getGPUMatrix();
		auto proj = camera->getProjection().getGPUMatrix();
		size_t countAllCb(0);
		for (size_t i(0); i < models.size(); i++)
		{
			// updates cb
			size_t countWorlds = models[i]->getWorlds().size();
			for (size_t j(0); j < countWorlds; j++)
			{
				cbufferCamera cbCam(models[i]->getWorlds().at(j).getGPUMatrix(), view, proj);
				cBuffers[L"cbCamera"]->updateData(cbCam, frameIndex, countAllCb++);
			}
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
		const float clearColor[] = { r, g, b, a };
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		// clearing depth/stencil buffer
		commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// draw triangle
		commandList->RSSetViewports(1, &viewport); // set the viewports
		commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects

		// set the descriptor heap
		vector<ID3D12DescriptorHeap*> descriptorHeaps = { mainDescriptorHeap[frameIndex].Get() };
		//vector<ID3D12DescriptorHeap*> descriptorHeaps = { mainDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());

		size_t countAllCb(0);
		for (auto&& pso : psos)
		{
			commandList->SetPipelineState(pso.second->getPipelineStateObject().Get());
			commandList->SetGraphicsRootSignature(pso.second->getRootSignature().Get());
			for (int i(0); i < models.size(); i++)
			{
				if (models[i]->getPsoKey() == pso.first)
				{
					Texture* texture(nullptr);
					if (textures.getTexture(models[i]->getTextureName(), &texture))
					{
						CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap[frameIndex]->GetGPUDescriptorHandleForHeapStart());
						//CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
						auto textureSrvIndex = texture->getIndexSrvHeap();
						tex.Offset(textureSrvIndex, mCbvSrvDescriptorSize);
						commandList->SetGraphicsRootDescriptorTable(0, tex);
					}
					size_t countWorlds = models[i]->getWorlds().size();
					commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
					commandList->IASetVertexBuffers(0, 1, &models[i]->getVertexBufferView()); // set the vertex buffer (using the vertex buffer view)
					commandList->IASetIndexBuffer(&models[i]->getIndexBufferView());
					for (size_t j(0); j < countWorlds; j++)
					{
						commandList->SetGraphicsRootConstantBufferView(1, cBuffers[L"cbCamera"]->getConstantBufferUploadHeap(frameIndex, countAllCb++));
						commandList->DrawIndexedInstanced(models[i]->getCountIndex(), 1, 0, 0, 0);
					}
				}
			}
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

		return true;
	}

	bool resize(int w, int h)
	{
		width = w;
		height = h;
		if (swapChain)
		{
			HRESULT hr;
			for (int i(0); i < frameBufferCount; i++)
				renderTargets[i].Reset();
			hr = swapChain->ResizeBuffers(frameBufferCount, width, height, backBufferFormat, NULL);
			if (FAILED(hr))
				return false;
			createBackBuffer();
			createViewport();
			createScissor();
			if (!createDepthAndStencilBuffer())
				return false;
			camera->updateProjection(0.4f * 3.14f, width / (FLOAT)height, 1.f, 1000.f);
		}
		return true;
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

	~Window()
	{
		CloseHandle(fenceEvent);
	}

};

Window* Window::current(nullptr);

int WINAPI WinMain(HINSTANCE hInstance,  HINSTANCE hPrevInstance, LPSTR lpCmdLine,  int nShowCmd)
{
	// create the window
	shared_ptr<Window>  window(new Window(3, 0, 1, 900, 650, TypeCamera::GAME_CAMERA, true));
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

// игровая камера
// разобраться с синхронизацией по времени
// присед, полный присед - реализовать

// msaa - разобраться, как включать
// переход из полного экрана и обратно - реализовать по своему алгоритму

// звук - звуки статические - всегда делать СТЕРЕО