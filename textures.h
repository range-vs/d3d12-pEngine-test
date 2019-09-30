#pragma once

#include "types.h"

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
	ContainerTextures() :count(0) {}

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

