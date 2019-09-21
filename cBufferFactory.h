#pragma once

#include <wrl.h>
#include <d3d12.h>

#include <vector>
#include <map>
#include <memory>

#include "cBufferTypes.h"

using namespace std;
using Microsoft::WRL::ComPtr;

// cBuffer

class CBufferGeneral
{
	vector < ComPtr < ID3D12Resource>> constantBufferUploadHeap; // this is the memory on the gpu where our constant buffer will be placed.
	vector<uint8*> bufferGpuAddress; // this is a pointer to the memory location we get when we map our constant buffer
	size_t shift;
public:
	template<class TypeData>
	bool initConstantBuffer(ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering, const wstring& debugName)
	{
		constantBufferUploadHeap.resize(buffering);
		bufferGpuAddress.resize(buffering);
		HRESULT hr;
		shift = (sizeof(TypeData) + 255) & ~255;
		for (int i(0); i < buffering; ++i)
		{
			hr = device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
				D3D12_HEAP_FLAG_NONE, // no flags
				&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
				D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
				nullptr, // we do not have use an optimized clear value for constant buffers
				IID_PPV_ARGS(constantBufferUploadHeap[i].GetAddressOf()));
			if (FAILED(hr))
				return false;
			hr = constantBufferUploadHeap[i]->SetName(debugName.c_str());
			if (FAILED(hr))
				return false;

			//D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			//cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress();
			//cbvDesc.SizeInBytes = shift;    // CB size is required to be 256-byte aligned.
			//device->CreateConstantBufferView(&cbvDesc, mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

			CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
			hr = constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&bufferGpuAddress[i]));
			if (FAILED(hr))
				return false;
		}
		return true;
	}

	template<class TypeData>
	void updateData(const TypeData& d, size_t frameIndex, size_t indexShift)
	{
		memcpy(bufferGpuAddress[frameIndex] + (shift * indexShift), &d, sizeof(d));
	}

	D3D12_GPU_VIRTUAL_ADDRESS getConstantBufferUploadHeap(size_t buffering, size_t indexShift)
	{
		return constantBufferUploadHeap[buffering]->GetGPUVirtualAddress() + (shift * indexShift);
	}

	size_t& getShiftAdress() { return shift; }
};

using CBufferGeneralPtr = shared_ptr< CBufferGeneral>;

// Factory

class CBufferFactory
{
public:
	virtual CBufferGeneralPtr create(ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering) = 0;
};

class CBufferCamera : public CBufferFactory
{
public:
	CBufferGeneralPtr create(ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering)override
	{
		CBufferGeneralPtr cameraBuffer(new CBufferGeneral);
		if (!cameraBuffer->initConstantBuffer<cbufferCamera>(device, mainDescriptorHeap, buffering, L"camera cb"))
		{
			// error
			exit(1);
		}
		return cameraBuffer;
	}
};

class CBufferLight : public CBufferFactory
{
public:
	CBufferGeneralPtr create(ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering)override
	{
		CBufferGeneralPtr lightBuffer(new CBufferGeneral);
		if (!lightBuffer->initConstantBuffer<cbufferLight>(device, mainDescriptorHeap, buffering, L"light cb"))
		{
			// error
			exit(1);
		}
		return lightBuffer;
	}
};

class CBufferMaterial: public CBufferFactory
{
public:
	CBufferGeneralPtr create(ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering)override
	{
		CBufferGeneralPtr matBuffer(new CBufferGeneral);
		if (!matBuffer->initConstantBuffer<cbufferMaterial>(device, mainDescriptorHeap, buffering, L"material cb"))
		{
			// error
			exit(1);
		}
		return matBuffer;
	}
};


// Factory creator

class CBufferCreator
{
	static map<wstring, shared_ptr<CBufferFactory>> cBufferCreators;
	
	static map<wstring, shared_ptr<CBufferFactory>> initCBufferCreators()
	{
		return
		{
			{L"cbCamera", shared_ptr<CBufferFactory>(new CBufferCamera)},
			{L"cbLight", shared_ptr<CBufferFactory>(new CBufferLight)},
			{L"cbMaterial", shared_ptr<CBufferFactory>(new CBufferMaterial)}
		};
	}
public:
	CBufferGeneralPtr createCBuffer(const wstring& key, ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering)
	{
		if (cBufferCreators.find(key) == cBufferCreators.end())
		{
			// error
			exit(1);
		}
		return cBufferCreators[key]->create(device, mainDescriptorHeap, buffering);
	}
};

map<wstring, shared_ptr<CBufferFactory>> CBufferCreator::cBufferCreators(CBufferCreator::initCBufferCreators());
