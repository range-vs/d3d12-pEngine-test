#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "sBufferTypes.h"

using Microsoft::WRL::ComPtr;

class SBufferGeneral
{
	vector < ComPtr < ID3D12Resource>> structuredBufferUploadHeap; // this is the memory on the gpu where our constant buffer will be placed.
	size_t sizeBuffer;
	vector<BYTE*> mMappedData;

public:
	template<class TypeData>
	bool initStructuredBuffer(ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering, const wstring& debugName,
		vector< TypeData>& instancedData)
	{
		HRESULT hr;

		structuredBufferUploadHeap.resize(buffering);
		mMappedData.resize(buffering);
		sizeBuffer = sizeof(TypeData);
		for (int i(0); i < buffering; ++i)
		{
			hr = device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeBuffer * instancedData.size()),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&structuredBufferUploadHeap[i]));
			if (FAILED(hr))
				return false;
			hr = structuredBufferUploadHeap[i]->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData[i]));
			if (FAILED(hr))
				return false;
			updateData(instancedData, i);
		}
		return true;
	}

	template<class TypeData>
	void updateData(const vector<TypeData>& d, size_t buffering)
	{
		for(int i(0);i<d.size();i++)
			memcpy(&mMappedData[buffering][i * sizeBuffer], &d[i], sizeof(d[i]));
	}

	D3D12_GPU_VIRTUAL_ADDRESS getConstantBufferUploadHeap(size_t buffering)
	{
		return structuredBufferUploadHeap[buffering]->GetGPUVirtualAddress();
	}

	size_t& getShiftAdress() { return sizeBuffer; }
};

using SBufferGeneralPtr = shared_ptr< SBufferGeneral>;

