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
#include <chrono>

#include "cBufferFactory.h"
#include "sBufferFactory.h"
#include "loaders.h"
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


extern "C"
{
	__declspec(dllexport) unsigned int NvOptimusEnablement;
}


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

class ModelPosition
{
	MatrixCPUAndGPU world;
	MatrixCPUAndGPU normal;

public:
	ModelPosition(const MatrixCPUAndGPU& w): world(w){}
	void generateNormal()
	{
		normal = Matrix4::Transponse(Matrix4::Inverse(world.getCPUMatrix()));
	}
	MatrixCPUAndGPU& getWorld() { return world; }
	MatrixCPUAndGPU& getNormals() { return normal; }

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
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
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
	vector<wstring> cBufferKeys;
	vector<wstring> sBufferKeys;

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
		//vector<D3D12_ROOT_DESCRIPTOR> rootCBVDescriptor(cBufferKeys.size());
		//for (int i(0); i < cBufferKeys.size(); i++)
		//{
		//	rootCBVDescriptor[i].RegisterSpace = i;
		//	rootCBVDescriptor[i].ShaderRegister = i; // 0
		//}


		//  PSO держит одну текстуру
		CD3DX12_DESCRIPTOR_RANGE range;
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // texture

		// создаем массив всех буферов
		vector<CD3DX12_ROOT_PARAMETER> slotRootParameter;
		CD3DX12_ROOT_PARAMETER rpt;
		rpt.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter.push_back(rpt);

		for (int i(0); i < cBufferKeys.size(); i++)
		{
			CD3DX12_ROOT_PARAMETER rp;
			rp.InitAsConstantBufferView(i);
			slotRootParameter.push_back(rp);
		}
		for (int i(0); i < sBufferKeys.size(); i++)
		{
			CD3DX12_ROOT_PARAMETER rp;
			rp.InitAsShaderResourceView(i+1);
			slotRootParameter.push_back(rp);
		}

		// create a static sampler
		auto staticSamplers = ContainerTextures::getStaticSamplers();

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(slotRootParameter.size(), // we have 1 root parameter
			slotRootParameter.data(), // a Point2er to the beginning of our root parameters array
			staticSamplers.size(),
			staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> errorBlob;
		ComPtr<ID3DBlob> signature;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, errorBlob.GetAddressOf());
		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			return false;
		}

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
		this->cBufferKeys.push_back(keyCB);
	}
	void addSBufferKey(const wstring& keySB)
	{
		this->sBufferKeys.push_back(keySB);
	}

	vector<wstring>& getCBufferKeys() { return cBufferKeys; }
	vector<wstring>& getSBufferKeys() { return sBufferKeys; }
};

using PipelineStateObjectPtr = shared_ptr< PipelineStateObject>;



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

	bool isSecond_1_1000() const
	{
		return allTime >= 1.f;
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



struct Material
{
	Vector ambient;
	Vector diffuse;
	Vector specular;
	float shininess;

	Material() :ambient(), diffuse(), specular(), shininess(0.f) {}
	Material(const Vector& a, const Vector& d, const Vector& s, const float& sn):ambient(a), diffuse(d), specular(s), shininess(sn){}
};

class ContainerMaterials
{
	map<wstring, Material> materials;

public:
	ContainerMaterials(){}
	bool addMaterial(const wstring& name, const Material& m) 
	{
		if (materials.find(name) != materials.end())
			return false;
		materials.insert({ name, m });
		return true;
	}
	bool getMaterial(const wstring& name, Material** m)
	{
		if (materials.find(name) == materials.end())
		{
			// error
			return false;
		}
		*m = &materials[name];
		return true;
	}
};



class Light
{
protected:
	Color color;

public:
	Light() :color() {}
	Light(const Color& c) :color(c) {}
	Color& getColor() { return color; }
	void setColor(const Color& c) { color = c; }
};

class LightDirection : public Light
{
	Vector dir;

public:
	LightDirection() :Light(), dir() {}
	LightDirection(const Color& c, const Vector& d) :Light(c), dir(d) {}
	Vector& getDirection() { return dir; }
	void setDirection(const Vector& d) { dir = d; }
};

class LightPoint : public Light
{
	Vector pos;

public:
	LightPoint() : Light(), pos() {}
	LightPoint(const Color& c, const Vector& p) :Light(c), pos(p) {}
	Vector& getPosition() { return pos; }
	void setPosition(const Vector& p) { pos = p; }
};

class LightSpot : public Light
{
	Vector pos;
	Vector dir;
	float cutOff;
	float outerCutOff;

public:
	LightSpot() : Light(), pos(), dir(), cutOff(0.f), outerCutOff(0.f) {}
	LightSpot(const Color& c, const Vector& p, const Vector& d, float coff, float outerCutOff) :Light(c), pos(p), dir(d), cutOff(coff), outerCutOff(outerCutOff) {}
	Vector& getPosition() { return pos; }
	void setPosition(const Vector& p) { pos = p; }
	Vector& getDirection() { return dir; }
	void setDirection(const Vector& d) { dir = d; }
	float& getCutOff() { return cutOff; }
	void setCutOff(float coff) { cutOff = coff; }
	float& getOuterCutOff() { return outerCutOff; }
	void setOuterCutOff(float outerCutOff) { this->outerCutOff = outerCutOff; }
};




class Sun
{
	LightDirection light;
	Vector position;
	SystemTimer updater;
	Matrix rotate;
	float angle;
	int wait;
	int currentWait;
	wstring modelSun;
	Vector center;

	bool isStop()
	{
		if (currentWait == 0)
			return true;
		if (updater.isSecond())
		{
			--currentWait;
			updater.setBeforeTime();
		}
		return false;
	}
public:
	void init(const Color& c, const Vector& p, int w)
	{
		position = p;
		center = { 0,0,0,0 };
		light = LightDirection(c, Vector::createVectorForDoublePoints(position, center));
		rotate = Matrix::Identity();
		updater.start();
		updater.setBeforeTime();
		wait = w;
		currentWait = 0;
	}

	Matrix update(bool& status)
	{
		status = false;
		Matrix newPosSun = Matrix::Identity();
		updater.setAfterTime();
		if (!isStop())
		{
			return Matrix::Identity();
		}
		if (updater.isSecond_1_100())
		{
			updater.setBeforeTime();
			angle += 0.2f;
			if (angle > 180)
			{
				angle = 0;
				currentWait = wait;
				//return; // расскомментировать, чтобы солнце замирало в конце
			}
			rotate = Matrix::CreateRotateMatrixZ(GeneralMath::degreesToRadians(-angle));
			newPosSun = Matrix::CreateTranslationMatrixXYZ(position[x], position[y], position[z]) * rotate;
			Vector v = position * rotate;
			light.setDirection(Vector::createVectorForDoublePoints(v, center));
			status = true;
		}
		return newPosSun;
	}

	LightDirection& getLightSun() { return light; }

	void setModel(const wstring& s) { modelSun = s; }

	wstring& getNameModel() { return modelSun; }

	Vector& getStartPosition() { return position; }
};

class Torch
{
	bool _enable;
	bool _used;
	LightSpot light;
public:
	void init(const Color& c, const Vector& p, const Vector& d, float coff, float outerCutOff)
	{
		light = LightSpot(c, p, d, cos(GeneralMath::degreesToRadians(coff)), cos(GeneralMath::degreesToRadians(outerCutOff)));
	}

	void update(const Vector& camPos, const Vector& camDir)
	{
		light.setPosition(camPos);
		light.setDirection(Vector::Normalize(camDir));
	}

	void enable()
	{
		_enable = true;
	}
	void disable()
	{
		_enable = false;
	}
	bool isEnable() { return _enable; }
	LightSpot& getLight() { return light; }
	void torchUsed(bool use) { _used = use; }
	bool isTorchUsed() { return _used; }
};

struct GameParameters
{
	map<wstring, CBufferGeneralPtr>* cBuffers;
	ContainerMaterials* materials;
	Camera* camera;
	Sun* sunLight;
	LightPoint* lightCenter;
	Torch* torch; 
	size_t* countAllCb;
	size_t* countMaterialsCb;
	Matrix view;
	Matrix proj;
	size_t frameIndex;
	Vector torchProperty;

	GameParameters() {}
};


class Model;

using ModelPtr = shared_ptr<Model>;

class Model
{
protected:
	ComPtr <ID3D12Resource> vertexBuffer;
	ComPtr < ID3D12Resource> indexBuffer;
	ComPtr<ID3D12Resource> vBufferUploadHeap;
	ComPtr<ID3D12Resource> iBufferUploadHeap;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	size_t vBufferSize;
	size_t iBufferSize;
	D3D12_SUBRESOURCE_DATA vertexData;
	D3D12_SUBRESOURCE_DATA indexData;

	wstring psoKey;

	vector< size_t> countIndex;
	vector<wstring> texturesKey;
	vector<wstring> materialsKey;

	static wstring _color;
public:
	virtual bool createModel(ComPtr<ID3D12Device>& device, BlankModel& data) = 0;
	virtual void draw(ComPtr < ID3D12GraphicsCommandList>& commandList, map<wstring, CBufferGeneralPtr>& cBuffers, ContainerTextures& textures,
		int frameIndex, size_t& countAllCb, size_t& countMaterialsCb, ComPtr<ID3D12DescriptorHeap>& mainDescriptorHeap, size_t& mCbvSrvDescriptorSize) = 0;

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

	virtual void createVertexBufferView() = 0;
	virtual void createIndexBufferView() = 0;
	virtual void translationBufferForHeapGPU(ComPtr < ID3D12GraphicsCommandList>& commandList) = 0;
	virtual wstring& getPsoKey() = 0;
	virtual void updateConstantBuffers(const GameParameters& gameParameters) = 0;

	virtual void addWorld(Matrix m) {}
	virtual void addWorld(MatrixCPUAndGPU m) {}
	virtual void editWorld(Matrix m, size_t index) {}
	virtual void editWorld(MatrixCPUAndGPU m, size_t index) {}

	static wstring isColor() { return _color; }
};

wstring Model::_color(L"--");

class ModelIdentity: public Model
{
	vector<ModelPosition> worlds;

public:
	ModelIdentity()
	{
		vBufferSize = 0;
		vertexBufferView = {};
		vertexData = {};
	}

	bool createModel(ComPtr<ID3D12Device>& device, BlankModel& data) override
	{
		if (!createBuffer(device, data.vertexs, vertexBuffer, vBufferSize, L"reserved name vb - todo"))
			return false;
		if (!createUploadBuffer(device, vBufferUploadHeap, vBufferSize, vertexData, data.vertexs, L"reserved name vb upload - todo"))
			return false;
		if (!createBuffer(device, data.indexs, indexBuffer, iBufferSize, L"reserved name ib - todo"))
			return false;
		if (!createUploadBuffer(device, iBufferUploadHeap, iBufferSize, indexData, data.indexs, L"reserved name ib upload - todo"))
			return false;
		countIndex = data.shiftIndexs;
		psoKey = data.psoName;
		texturesKey = data.texturesName;
		materialsKey = data.materials;
		return true;
	}

	void createVertexBufferView() override
	{
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = vBufferSize;
	}

	void createIndexBufferView() override
	{
		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = iBufferSize;
	}

	void translationBufferForHeapGPU(ComPtr < ID3D12GraphicsCommandList>& commandList) override
	{
		UpdateSubresources(commandList.Get(), vertexBuffer.Get(), vBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		UpdateSubresources(commandList.Get(), indexBuffer.Get(), iBufferUploadHeap.Get(), 0, 0, 1, &indexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	}

	void draw(ComPtr < ID3D12GraphicsCommandList>& commandList, map<wstring, CBufferGeneralPtr>& cBuffers, ContainerTextures& textures,
		int frameIndex, size_t& countAllCb, size_t& countMaterialsCb, ComPtr<ID3D12DescriptorHeap>& mainDescriptorHeap, size_t& mCbvSrvDescriptorSize) override
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&indexBufferView);
		for (size_t j(0); j < worlds.size(); j++)
		{
			commandList->SetGraphicsRootConstantBufferView(1, cBuffers[L"cbCamera"]->getConstantBufferUploadHeap(frameIndex, countAllCb));
			commandList->SetGraphicsRootConstantBufferView(2, cBuffers[L"cbLight"]->getConstantBufferUploadHeap(frameIndex, countAllCb));

			countAllCb++;
			size_t startIndex(0);
			for (int i(0); i < texturesKey.size(); i++)
			{
				Texture* texture(nullptr);
				if(texturesKey[i] != isColor() && textures.getTexture(texturesKey[i], &texture))
				{
					CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
					//CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
					auto textureSrvIndex = texture->getIndexSrvHeap();
					tex.Offset(textureSrvIndex, mCbvSrvDescriptorSize);
					commandList->SetGraphicsRootDescriptorTable(0, tex);
				}
				commandList->SetGraphicsRootConstantBufferView(3, cBuffers[L"cbMaterial"]->getConstantBufferUploadHeap(frameIndex, countMaterialsCb));
				countMaterialsCb++;
				commandList->DrawIndexedInstanced(countIndex[i], 1, startIndex, 0, 0);
				startIndex += countIndex[i];
			}
		}
	}

	wstring& getPsoKey() override { return psoKey; }

	void updateConstantBuffers(const GameParameters& gameParameters)override
	{
		size_t countWorlds = worlds.size();
		for (size_t j(0); j < countWorlds; j++)
		{
			cbufferCamera cbCam(worlds.at(j).getWorld().getGPUMatrix(), gameParameters.view, gameParameters.proj);
			gameParameters.cBuffers->at(L"cbCamera")->updateData(cbCam, gameParameters.frameIndex, *gameParameters.countAllCb);

			cbufferLight cbDirLight(gameParameters.sunLight->getLightSun().getColor(), gameParameters.sunLight->getLightSun().getDirection(),
				gameParameters.lightCenter->getColor(), gameParameters.lightCenter->getPosition(), (Vector)gameParameters.camera->getPosition(),
				worlds.at(j).getNormals().getGPUMatrix(),
				gameParameters.torch->getLight().getColor(), gameParameters.torch->getLight().getDirection(), gameParameters.torch->getLight().getPosition(),
				gameParameters.torchProperty);

			gameParameters.cBuffers->at(L"cbLight")->updateData(cbDirLight, gameParameters.frameIndex, *gameParameters.countAllCb);

			for (size_t k(0); k < materialsKey.size(); k++)
			{
				Material* m(nullptr);
				if (gameParameters.materials->getMaterial(materialsKey[k], &m))
				{
					cbufferMaterial mat(m->ambient, m->diffuse, m->specular, m->shininess);
					gameParameters.cBuffers->at(L"cbMaterial")->updateData(mat, gameParameters.frameIndex, *gameParameters.countMaterialsCb);
					(*gameParameters.countMaterialsCb)++;
				}
			}

			(*gameParameters.countAllCb)++;
		}
	}

	void addWorld(Matrix m)override { ModelPosition _m(m);  worlds.push_back(_m); }
	void addWorld(MatrixCPUAndGPU m)override { ModelPosition _m(m);  worlds.push_back(_m); }
	void editWorld(Matrix m, size_t index)override { ModelPosition _m(m);  worlds[index] = _m; }
	void editWorld(MatrixCPUAndGPU m, size_t index)override { ModelPosition _m(m);  worlds[index] = _m; }
};

class ModelInstancing : public Model
{
	map<wstring, SBufferGeneralPtr> sBuffers;

	size_t countInstance;

public: 
	ModelInstancing()
	{
		vBufferSize = 0;
		vertexBufferView = {};
		vertexData = {};
	}

	template<class TypeData>
	bool addStructuredBuffer(ComPtr<ID3D12Device> & device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering, 
		const wstring& name, TypeData& strBufferData)
	{
		SBufferGeneralPtr buffer(new SBufferGeneral);
		if (!buffer->initStructuredBuffer(device, mainDescriptorHeap, buffering, L"structured buffer " + name, strBufferData))
			return false;
		sBuffers.insert({ name, buffer });
	}

	void setCountInstanced(size_t inst)
	{
		countInstance = inst;
	}

	bool createModel(ComPtr<ID3D12Device>& device, BlankModel& data) override
	{
		if (!createBuffer(device, data.vertexs, vertexBuffer, vBufferSize, L"reserved name vb - todo"))
			return false;
		if (!createUploadBuffer(device, vBufferUploadHeap, vBufferSize, vertexData, data.vertexs, L"reserved name vb upload - todo"))
			return false;
		if (!createBuffer(device, data.indexs, indexBuffer, iBufferSize, L"reserved name ib - todo"))
			return false;
		if (!createUploadBuffer(device, iBufferUploadHeap, iBufferSize, indexData, data.indexs, L"reserved name ib upload - todo"))
			return false;
		countIndex = data.shiftIndexs;
		psoKey = data.psoName;
		texturesKey = data.texturesName;
		materialsKey = data.materials;
		return true;
	}

	void createVertexBufferView() override
	{
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = vBufferSize;
	}

	void createIndexBufferView() override
	{
		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = iBufferSize;
	}

	void translationBufferForHeapGPU(ComPtr < ID3D12GraphicsCommandList>& commandList) override
	{
		UpdateSubresources(commandList.Get(), vertexBuffer.Get(), vBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		UpdateSubresources(commandList.Get(), indexBuffer.Get(), iBufferUploadHeap.Get(), 0, 0, 1, &indexData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	}

	void draw(ComPtr < ID3D12GraphicsCommandList>& commandList, map<wstring, CBufferGeneralPtr>& cBuffers, ContainerTextures& textures,
		int frameIndex, size_t& countAllCb, size_t& countMaterialsCb, ComPtr<ID3D12DescriptorHeap>& mainDescriptorHeap, size_t& mCbvSrvDescriptorSize) override
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // set the vertex buffer (using the vertex buffer view)
		commandList->IASetIndexBuffer(&indexBufferView);

		commandList->SetGraphicsRootConstantBufferView(1, cBuffers[L"cbCamera"]->getConstantBufferUploadHeap(frameIndex, countAllCb));
		commandList->SetGraphicsRootConstantBufferView(2, cBuffers[L"cbLight"]->getConstantBufferUploadHeap(frameIndex, countAllCb));
		
		commandList->SetGraphicsRootShaderResourceView(4, sBuffers[L"sbInstancedPosition"]->getConstantBufferUploadHeap(frameIndex)); // продумать структуру соответствия

		countAllCb++;
		size_t startIndex(0);
		for (int i(0); i < texturesKey.size(); i++)
		{
			Texture* texture(nullptr);
			if (texturesKey[i] != isColor() && textures.getTexture(texturesKey[i], &texture))
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
				//CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
				auto textureSrvIndex = texture->getIndexSrvHeap();
				tex.Offset(textureSrvIndex, mCbvSrvDescriptorSize);
				commandList->SetGraphicsRootDescriptorTable(0, tex);
			}
			commandList->SetGraphicsRootConstantBufferView(3, cBuffers[L"cbMaterial"]->getConstantBufferUploadHeap(frameIndex, countMaterialsCb));
			countMaterialsCb++;
			commandList->DrawIndexedInstanced(countIndex[i], countInstance, startIndex, 0, 0);
			startIndex += countIndex[i];
		}
	}

	void updateConstantBuffers(const GameParameters& gameParameters)override
	{
		cbufferCamera cbCam(Matrix::Identity(), gameParameters.view, gameParameters.proj);
		gameParameters.cBuffers->at(L"cbCamera")->updateData(cbCam, gameParameters.frameIndex, *gameParameters.countAllCb);

		cbufferLight cbDirLight(gameParameters.sunLight->getLightSun().getColor(), gameParameters.sunLight->getLightSun().getDirection(),
			gameParameters.lightCenter->getColor(), gameParameters.lightCenter->getPosition(), (Vector)gameParameters.camera->getPosition(),
			Matrix::Identity(),
			gameParameters.torch->getLight().getColor(), gameParameters.torch->getLight().getDirection(), gameParameters.torch->getLight().getPosition(),
			gameParameters.torchProperty);

		gameParameters.cBuffers->at(L"cbLight")->updateData(cbDirLight, gameParameters.frameIndex, *gameParameters.countAllCb);

		for (size_t k(0); k < materialsKey.size(); k++)
		{
			Material* m(nullptr);
			if (gameParameters.materials->getMaterial(materialsKey[k], &m))
			{
				cbufferMaterial mat(m->ambient, m->diffuse, m->specular, m->shininess);
				gameParameters.cBuffers->at(L"cbMaterial")->updateData(mat, gameParameters.frameIndex, *gameParameters.countMaterialsCb);
				(*gameParameters.countMaterialsCb)++;
			}
		}

		(*gameParameters.countAllCb)++;
	}


	wstring& getPsoKey() override { return psoKey; }

	// продумать обновление?
	/*void addWorld(Matrix m)override { ModelPosition _m(m);  worlds.push_back(_m); }
	void addWorld(MatrixCPUAndGPU m)override { ModelPosition _m(m);  worlds.push_back(_m); }
	void editWorld(Matrix m, size_t index)override { ModelPosition _m(m);  worlds[index] = _m; }
	void editWorld(MatrixCPUAndGPU m, size_t index)override { ModelPosition _m(m);  worlds[index] = _m; }*/
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


// debug //

struct VertexNormal
{
	Point Pos;
	Point Texture;
	Point Normal;

	VertexNormal() : Texture(), Normal(), Pos(){  }
	VertexNormal(float x, float y, float z) :Pos(Point(x, y, z)), Texture(), Normal() {  }
	VertexNormal(const Point& p, const Point& t, const Point& n) : Pos(p), Texture(t), Normal(n) {  }
};

struct BoundingBoxData // данные для создания BBox
{
	float xl, yd, zf; // лево, низ и перед
	float xr, yu, zr; // право, верх и зад
	Vector center; // центр модели

	BoundingBoxData(float xl, float yd, float zf, float xr, float yu, float zr) :xl(xl), yd(yd), zf(zf), xr(xr), yu(yu), zr(zr) {}
	BoundingBoxData() :xl(0), yd(0), zf(0), xr(0), yu(0), zr(0) {}
};

struct DataFromFile // структура, возвращаемая при загрузке файла
{
	std::vector<wstring> texturesListOut; // выходной список текстур для модели
	std::vector< VertexNormal> vertex; // массив вершин
	size_t vertexCount; // количество вершин
	std::vector<WORD> index; // массив индексов
	size_t indexCount; // количество индексов
	std::vector<wstring> shaders;
	BoundingBoxData bboxData; // крайние точки BBox
	bool isBBox; // есть ли данные ббокса
	bool isInstansing; // инстансинговая ли модель

	DataFromFile() :vertexCount(0), isBBox(false) {}
	DataFromFile(const std::vector<wstring>& tLO, const std::vector< VertexNormal>& v, size_t vC, const std::vector<WORD>& i, size_t iC, const std::vector<wstring>& shaders = std::vector<wstring>(), const BoundingBoxData& bbd = BoundingBoxData());
};

struct DateTime // время и дата
{
	int seconds;
	int minutes;
	int hours;
	int days;
	int mounts;
	int years;

	DateTime(int s, int m, int h, int d, int mo, int y) :seconds(s), minutes(m), hours(h), days(d), mounts(mo), years(1900 + y) {}
	DateTime() :seconds(0), minutes(0), hours(0), days(0), mounts(0), years(1900) {}
};

// debug //


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

	// test camera timing
	chrono::time_point<std::chrono::high_resolution_clock> timeFrameBefore;
	chrono::time_point<std::chrono::high_resolution_clock> timeFrameAfter;
	double countsPerSecond;

public:

	Window(int buffering, int vsync, bool msaaState, int w, int h, TypeCamera tc = TypeCamera::STATIC_CAMERA, bool md = false)
	{
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

	bool loadFromFileProObject(const string& path, DataFromFile* data)
	{
		std::ifstream file(path, std::ios_base::binary);
		if (!file)
		{
			// TODO: print error message
			return false;
		}
		// переменные для модели
		int countMesh(0); // количество мешей в модели
		Vector center; // центр BBox
		BoundingBoxData bBoxData; // данные для конструирования BBox
		DataFromFile _data; // выходной буфер
		_data.isBBox = true; // ббокс будет загружаться из файла

		while (true)
		{
			int block(0);
			file.read((char*)&block, 4);
			if (file.eof())
				break;
			if (!file)
			{
				// TODO: print error message
				return false;
			}
			switch (block)
			{
			case 0x10:// блок служебной информации
			{
				string nameAutor;
				if (!loadLineFromFile(nameAutor, file)) // имя автора
				{
					// TODO: print error message
					return false;
				}
				DateTime dateCreate;
				file.read((char*)&dateCreate, sizeof DateTime); // дата создания
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				DateTime dateModify;
				file.read((char*)&dateModify, sizeof DateTime); // дата модификации
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				file.read((char*)&countMesh, 4); // количество мешей в модели
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				// читаем центр модели
				float arr[6];
				for (int i(0); i < 3; i++)
				{
					file.read((char*)&arr[i], sizeof(float));
					if (!file)
					{
						// TODO: print error message
						return false;
					}
				}
				center = Vector(arr[0], arr[1], arr[2], 1);
				// читаем BBox модели(ограничивающие точки)
				for (int i(0); i < 6; i++)
				{
					file.read((char*)&arr[i], sizeof(float));
					if (!file)
					{
						// TODO: print error message
						return false;
					}
				}
				bBoxData = BoundingBoxData(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
				_data.bboxData = bBoxData;
				_data.bboxData.center = center; // записываем центр
				string unit;
				if (!loadLineFromFile(unit, file)) // читаем единицы измерения
				{
					// TODO: print error message
					return false;
				}
				break;
			}

			case 0x20: // блок информации о каждом меше в модели
			{
				for (int i(0); i < countMesh; i++) // перебираем все меши
				{
					file.read((char*)&block, 4); // id подблока
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					string nameMesh;
					if (!loadLineFromFile(nameMesh, file))  // читаем имя меша
					{
						// TODO: print error message
						return false;
					}
					int ci(0); file.read((char*)&ci, 4); // читаем количество индексов
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					_data.indexCount += ci; // приращиваем количество индексов
					for (int j(0); j < ci; j++) // перебираем все индексы
					{
						int tmpIndex(-1); // временная переменная
						file.read((char*)&tmpIndex, 4); // читаем индекс
						if (!file)
						{
							// TODO: print error message
							return false;
						}
						_data.index.push_back((WORD)tmpIndex); // пишем считанный индекс в буфер
					}
				}
				break;
			}

			case 0x30: // блок информации о вершинах
			{
				file.read((char*)&_data.vertexCount, 4); // количество вершин
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				for (int i(0); i < _data.vertexCount; i++) // перебираем все вершины
				{
					VertexNormal vn;// временная переменная
					file.read((char*)&vn, sizeof(VertexNormal)); // читаем вершину
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					_data.vertex.push_back(vn); // пишем считанную вершину в буфер
				}
				break;
			}

			case 0x40: // блок информации о текстурах
			{
				int count(0);
				file.read((char*)&count, 4); // количество текстур
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				_data.texturesListOut.resize(count); // создаём буфер текстур
				for (int i(0); i < count; i++)
				{
					int index(0);
					file.read((char*)&index, 4); // индекс текстуры
					if (!file)
					{
						// TODO: print error message
						return false;
					}
					string path;
					if (!loadLineFromFile(path, file)) // читаем путь к текстуре
					{
						// TODO: print error message
						return false;
					}
					_data.texturesListOut[index] = wstring(path.begin(), path.end()); // помещаем текстуру в буфер на позицию
				}
				break;
			}

			case 0x50: // блок настроек(пока только инстансинг или нет)
			{
				file.read((char*)&_data.isInstansing, sizeof(bool)); // количество текстур
				if (!file)
				{
					// TODO: print error message
					return false;
				}
				if (_data.isInstansing)
					_data.shaders = { L"textures_instansing", L"textures_instansing", L"textures" };
				else
					_data.shaders = { L"textures" };
				break;
			}

			default:
			{
				file.close();
				return false;
			}

			}
		}
		file.close();
		*data = _data;
		return true;
	}
	
	bool  loadLineFromFile(string& line, std::ifstream& file)
	{
		char s(' ');
		int size(0);
		while (true)
		{
			file.read((char*)&s, 1);
			if (!file)
			{
				// TODO: print error message
				return false;
			}
			if (s == '\0')
				break;
			line += s;
		}
		return true;
	}


	bool initializeSceneDirect3D() 
	{
		array<Color, 2> colors =
		{
			Color(0.f, .49f, .05f, 1.f),
			Color(0.41f, 0.24f, 0.49f, 1.f)
		};
		Matrix4 trans;

		{
			// cube color
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
			vector<Point> norm = {
		generateNormal(_v[0], _v[1], _v[4]), // near 0
		generateNormal(_v[5], _v[1], _v[2]), // right 1
		generateNormal(_v[6], _v[2], _v[3]), // far 2
		generateNormal(_v[7], _v[3], _v[0]), // left 3
		generateNormal(_v[0], _v[3], _v[2]), // top 4 
		generateNormal(_v[7], _v[4], _v[5]), // bottom 5
			};
			vector<Point> real_norm = {
		genereateAverageNormal(3, norm[0], norm[3], norm[4]),  // top ( против часовой, с самой нижней левой)
		genereateAverageNormal(3, norm[0], norm[1], norm[4]),
		genereateAverageNormal(3, norm[2], norm[1], norm[4]),
		genereateAverageNormal(3, norm[2], norm[3], norm[4]),
		genereateAverageNormal(3, norm[0], norm[3], norm[5]),  // bottom ( против часовой, с самой нижней левой)
		genereateAverageNormal(3, norm[0], norm[1], norm[5]),
		genereateAverageNormal(3, norm[2], norm[1], norm[5]),
		genereateAverageNormal(3, norm[2], norm[3], norm[5]),
			};
			bm.vertexs = {
				Vertex(_v[0],real_norm[0],  colors[1]),
				Vertex(_v[1],real_norm[1],  colors[1]),
				Vertex(_v[2],real_norm[2],  colors[1]),
				Vertex(_v[3],real_norm[3],  colors[1]),
				Vertex(_v[4],real_norm[4],  colors[1]),
				Vertex(_v[5],real_norm[5],  colors[1]),
				Vertex(_v[6] ,real_norm[6],  colors[1]),
				Vertex(_v[7],real_norm[7],  colors[1])
			};
			bm.indexs = {
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
			bm.shiftIndexs.push_back(bm.indexs.size());
			bm.texturesName.push_back(Model::isColor()); // color
			bm.materials.push_back(L"default");
			bm.psoName = L"PSOLightPhong";

			array<pair<float, float>, 4> coord = { pair<float, float>{-15.f, -15.f}, pair<float, float>{-15.f, 15.f}, pair<float, float>{15.f, -15.f}, pair<float, float>{15.f, 15.f} };
			for (int i(0), y(10); i < 3; i++, y += 20)
			{
				for (int j(0); j < coord.size(); j++)
				{
					auto world1(Matrix4::CreateTranslationMatrixXYZ(coord[j].first, (float)y, coord[j].second) * Matrix::CreateTranslationMatrixX(-80.f));
					if (!createModel(bm, world1))
						return false;
				}
			}
		}

		{ 
			// plane grass
			BlankModel bm;
			bm.nameModel = L"plane grass";
			bm.texturesName.push_back(L"engine_resource/textures/pGrass.dds");
			bm.indexs =
			{
				0, 3, 2,
				0, 2, 1
			};
			bm.shiftIndexs.push_back(bm.indexs.size());
			bm.vertexs =
			{
				Vertex({-10000.f,  0.f,  -10000.f}, {0,1,0},  {-129,130,0,-1}),
				Vertex({10000.f,  0.f,  -10000.f},{0,1,0}, {130,130,0,-1}),
				Vertex({10000.f,  0.f, 10000.f},{0,1,0}, {130,-129,0,-1}),
				Vertex({-10000.f,  0.f,  10000.f},{0,1,0}, {-129,-129,0,-1})
			};
			bm.psoName = L"PSOLightBlinnPhong";
			bm.materials.push_back(L"default");
			if (!createModel(bm))
				return false;
		}

		{
			// cube wood
			BlankModel bm;
			float value = 10.f;
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
			vector<Point> norm = {
		generateNormal(_v[0], _v[1], _v[4]), // near
		generateNormal(_v[5], _v[1], _v[2]), // right
		generateNormal(_v[6], _v[2], _v[3]), // far
		generateNormal(_v[7], _v[3], _v[0]), // left
		generateNormal(_v[0], _v[3], _v[2]), // top
		generateNormal(_v[7], _v[4], _v[5]), // bottom
			};
			vector<Point> real_norm = {
				genereateAverageNormal(3, norm[0], norm[3], norm[4]),  // top ( против часовой, с самой нижней левой)
				genereateAverageNormal(3, norm[0], norm[1], norm[4]),
				genereateAverageNormal(3, norm[2], norm[1], norm[4]),
				genereateAverageNormal(3, norm[2], norm[3], norm[4]),
				genereateAverageNormal(3, norm[0], norm[3], norm[5]),  // bottom ( против часовой, с самой нижней левой)
				genereateAverageNormal(3, norm[0], norm[1], norm[5]),
				genereateAverageNormal(3, norm[2], norm[1], norm[5]),
				genereateAverageNormal(3, norm[2], norm[3], norm[5]),
			};
			bm.vertexs = {
				Vertex(_v[0], real_norm[0], {0.35f, 1.f, 0, -1}),
				Vertex(_v[1], real_norm[1],{0.63f, 1.f, 0, -1}),
				Vertex(_v[2],real_norm[2], {0.63f, 0.75f, 0, -1}),
				Vertex(_v[3], real_norm[3],{0.35f, 0.75f, 0, -1}),
				Vertex(_v[7], real_norm[7],{0.12f, 0.75f, 0, -1}),
				Vertex(_v[4], real_norm[4],{0.12f, 1.f, 0, -1}),
				Vertex(_v[5], real_norm[5],{0.85f, 1.f, 0, -1}),
				Vertex(_v[6], real_norm[6],{0.85f, 0.75f, 0, -1}),
				Vertex(_v[7], real_norm[7],{0.35f, 0.5f, 0, -1}),
				Vertex(_v[6], real_norm[6],{0.63f, 0.5f, 0, -1}),
				Vertex(_v[4],real_norm[4],{0.35f, 0.25f, 0, -1}),
				Vertex(_v[5], real_norm[5],{0.63f, 0.25f, 0, -1}),
				Vertex(_v[0],real_norm[0], {0.35f, 0.f, 0, -1}),
				Vertex(_v[1], real_norm[1],{0.63f, 0.f, 0, -1})
			};
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
			bm.shiftIndexs.push_back(bm.indexs.size());
			bm.nameModel = L"cube wood";
			bm.texturesName.push_back(L"engine_resource/textures/pWoodDoski.dds");
			bm.psoName = L"PSOLightBlinnPhong";
			bm.materials.push_back(L"default");
			float val(20);
			array<pair<float, float>, 4> coord = { pair<float, float>{-val, -val}, pair<float, float>{-val, val}, pair<float, float>{val, -val}, pair<float, float>{val, val} };
			for (int i(0), y(10); i < 3; i++, y += 30)
			{
				for (int j(0); j < coord.size(); j++)
				{
					auto world1(Matrix4::CreateTranslationMatrixXYZ(coord[j].first, (float)y, coord[j].second));
					if (!createModel(bm, world1))
						return false;
				}
			}
		}

		{
			// plane grass, wood and cirpic
			BlankModel bm;
			bm.nameModel = L"plane grass, cirpic and wood";
			bm.texturesName.push_back(L"engine_resource/textures/pHouse.dds");
			bm.texturesName.push_back(L"engine_resource/textures/pWoodDesc.dds");
			bm.materials.push_back(L"default");
			bm.materials.push_back(L"default");
			bm.indexs =
			{
				5, 7, 6,
				5, 6, 4,

				9, 11, 10,
				9, 10, 8,
			};
			bm.shiftIndexs.push_back(6);
			bm.shiftIndexs.push_back(6);
			bm.vertexs =
			{
				Vertex({-100.f,  0.f,  -250.f}, {0,1,0},  {-2,3,0,-1}),
				Vertex({50.f,  0.f,  -250.f},{0,1,0}, {3,3,0,-1}),
				Vertex({50.f,  0.f, -50.f},{0,1,0}, {3,-2,0,-1}),
				Vertex({-100.f,  0.f,  -50.f},{0,1,0}, {-2,-2,0,-1}),

				Vertex({50.f,  0.f,  -150.f},{-1,0,0}, {2, 2 ,0,-1}),
				Vertex({50.f,  0.f, -50.f},{-1,0,0}, {-2, 2,0,-1}),
				Vertex({50.f,  100.f,  -150.f},{-1,0,0}, {2,-2,0,-1}),
				Vertex({50.f,  100.f, -50.f},{-1,0,0}, {-2,-2,0,-1}),

				Vertex({50.f,  100.f,  -150.f},{-1,0,0}, {1, 1 ,0,-1}), // 8
				Vertex({50.f,  100.f, -50.f},{-1,0,0}, {0, 1,0,-1}), // 9
				Vertex({50.f,  200.f,  -150.f},{-1,0,0}, {1,0,0,-1}), // 10
				Vertex({50.f,  200.f, -50.f},{-1,0,0}, {0,0,0,-1}) // 11
			};
			bm.psoName = L"PSOLightBlinnPhong";
			if (!createModel(bm))
				return false;
		}

		{
			// dimes roft
			BlankModel bm;
			bm.nameModel = L"dimes-test-rofl";
			bm.psoName = L"PSOLightBlinnPhong";
			if (!loadModelFromFile("model_dimes/dimes.pro_object", bm))
				return false;
			auto transform = Matrix::CreateTranslationMatrixXYZ(-50.f, 0.f, -200.f);
			if (!createModel(bm, transform))
				return false;
		}

		{
			// houses
			vector<BlankModel> bm(3);
			vector<string> namesModel = { "model_houses/house1.pro_object", "model_houses/house2.pro_object", "model_houses/house3.pro_object" };
			for (int i(0); i< bm.size(); i++)
			{
				bm[i].nameModel = L"house " + to_wstring(i) + L" instansing";
				bm[i].psoName = L"PSOLightBlinnPhongInstansed";
				if (!loadModelFromFile(namesModel[i], bm[i]))
					return false;
			}
			srand(time(nullptr));
			vector<vector<sbufferInstancing>> worlds(3);
			for (int x(-5000); x < 5000; x+=250)
			{
				for (int z(-5000); z < 5000; z += 250)
				{
					int indexModel = 0 + rand() % 3;
					if ((x >= -150 && x <= 150) || (z >= -150 && z <= 150))
						continue;
					array<float, 7> degrees = { 0.f, 90.f, 180.f, 270.f, -90.f, -180.f, -270.f };
					int indexRotate = 0 + rand() % degrees.size();
					float rotate = degrees[indexRotate];
					ModelPosition world(Matrix::CreateRotateMatrixY(GeneralMath::degreesToRadians(rotate)) * Matrix4::CreateTranslationMatrixXYZ(x, 0, z));
					worlds[indexModel].push_back({ world.getWorld().getGPUMatrix(), world.getNormals().getGPUMatrix() });
				}
			}
			for (int i(0); i < bm.size(); i++)
			{
				bm[i].instansing = true;
				bm[i].instansingBuffer = std::move(worlds[i]);
				if (!createModel(bm[i]))
					return false;
			}
		}

		return true;
	}

	bool loadModelFromFile(const string& path, BlankModel& bm)
	{
		DataFromFile data;
		if (!loadFromFileProObject(path, &data))
			return false;
		vector<vector<int>> shifts(data.texturesListOut.size());
		for (VertexNormal& vn : data.vertex)
			bm.vertexs.push_back({ vn.Pos, vn.Normal, {vn.Texture[x], vn.Texture[y], 0, -1} });
		for (WORD& index : data.index)
		{
			int indNewIndexs = data.vertex[index].Texture[z];
			shifts[indNewIndexs].push_back(index);
		}
		for (auto& shift : shifts)
		{
			bm.shiftIndexs.push_back(shift.size());
			copy(shift.begin(), shift.end(), back_inserter(bm.indexs));
		}
		for (auto t : data.texturesListOut)
		{
			t = t.erase(0, t.find(L'/') + 1);
			t = t.insert(0, L"engine_resource/textures/");
			bm.texturesName.push_back(t);
			bm.materials.push_back(L"default");
		}
		return true;
	}

	bool createModel(BlankModel& data, Matrix4 world = Matrix4::Identity())
	{
		// todo: потом прикрутить фабрику для создания моделей
		ModelPtr model;
		if (modelsIndex.find(data.nameModel) == modelsIndex.end())
		{
			for (auto&& t : data.texturesName)
			{
				if (t == L"--")
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
		bm.texturesName.push_back(Model::isColor());
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
		gameParam.torchProperty = { torch.getLight().getCutOff(), torch.getLight().getOuterCutOff(), (torch.isEnable() ? 1.f : 0.f) };
		gameParam.frameIndex = frameIndex;

		for (auto&& pso : psos)
		{
			for (int i(0); i < models.size(); i++)
			{
				// updates cb
				if (models[i]->getPsoKey() == pso.first)
					models[i]->updateConstantBuffers(gameParam);
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
		for (auto&& pso : psos)
		{
			commandList->SetPipelineState(pso.second->getPipelineStateObject().Get());
			commandList->SetGraphicsRootSignature(pso.second->getRootSignature().Get());
			for (int i(0); i < models.size(); i++)
			{
				if (models[i]->getPsoKey() == pso.first)
					models[i]->draw(commandList, cBuffers, textures, frameIndex, countAllCb, countMaterialsCb, mainDescriptorHeap[frameIndex], mCbvSrvDescriptorSize);
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
	shared_ptr<Window>  window(new Window(3, 0, 1, 900, 650, TypeCamera::GAME_CAMERA));
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