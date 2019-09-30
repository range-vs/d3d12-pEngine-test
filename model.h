#pragma once

#include "pso.h"
#include "game_parameters.h"

class ModelPosition
{
	MatrixCPUAndGPU world;
	MatrixCPUAndGPU normal;

public:
	ModelPosition(const MatrixCPUAndGPU& w) : world(w) {}
	void generateNormal()
	{
		normal = Matrix4::Transponse(Matrix4::Inverse(world.getCPUMatrix()));
	}
	MatrixCPUAndGPU& getWorld() { return world; }
	MatrixCPUAndGPU& getNormals() { return normal; }

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
	vector<bool> dSides;


public:
	virtual bool createModel(ComPtr<ID3D12Device>& device, BlankModel& data) = 0;
	virtual void draw(PipelineStateObjectPtr& pso, ComPtr < ID3D12GraphicsCommandList>& commandList, map<wstring, CBufferGeneralPtr>& cBuffers, ContainerTextures& textures,
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

};


class ModelIdentity : public Model
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
		dSides = data.doubleSides;
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

	void draw(PipelineStateObjectPtr& pso, ComPtr < ID3D12GraphicsCommandList>& commandList, map<wstring, CBufferGeneralPtr>& cBuffers, ContainerTextures& textures,
		int frameIndex, size_t& countAllCb, size_t& countMaterialsCb, ComPtr<ID3D12DescriptorHeap>& mainDescriptorHeap, size_t& mCbvSrvDescriptorSize) override
	{
		wstring _isColor(1, ModelConstants::isColor);
		commandList->SetGraphicsRootSignature(pso->getRootSignature().Get());
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
				if (texturesKey[i] != _isColor && textures.getTexture(texturesKey[i], &texture))
				{
					CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
					//CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
					auto textureSrvIndex = texture->getIndexSrvHeap();
					tex.Offset(textureSrvIndex, mCbvSrvDescriptorSize);
					commandList->SetGraphicsRootDescriptorTable(0, tex);
				}
				commandList->SetGraphicsRootConstantBufferView(3, cBuffers[L"cbMaterial"]->getConstantBufferUploadHeap(frameIndex, countMaterialsCb));
				countMaterialsCb++;
				if (dSides[i])
					commandList->SetPipelineState(pso->getPipelineStateObjectDoubleSides().Get());
				else
					commandList->SetPipelineState(pso->getPipelineStateObject().Get());

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
	bool addStructuredBuffer(ComPtr<ID3D12Device>& device, vector<ComPtr<ID3D12DescriptorHeap>>& mainDescriptorHeap, size_t buffering,
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
		dSides = data.doubleSides;
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

	void draw(PipelineStateObjectPtr& pso, ComPtr < ID3D12GraphicsCommandList>& commandList, map<wstring, CBufferGeneralPtr>& cBuffers, ContainerTextures& textures,
		int frameIndex, size_t& countAllCb, size_t& countMaterialsCb, ComPtr<ID3D12DescriptorHeap>& mainDescriptorHeap, size_t& mCbvSrvDescriptorSize) override
	{
		wstring _isColor(1, ModelConstants::isColor);
		commandList->SetGraphicsRootSignature(pso->getRootSignature().Get());
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
			if (texturesKey[i] != _isColor && textures.getTexture(texturesKey[i], &texture))
			{
				CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
				//CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
				auto textureSrvIndex = texture->getIndexSrvHeap();
				tex.Offset(textureSrvIndex, mCbvSrvDescriptorSize);
				commandList->SetGraphicsRootDescriptorTable(0, tex);
			}
			commandList->SetGraphicsRootConstantBufferView(3, cBuffers[L"cbMaterial"]->getConstantBufferUploadHeap(frameIndex, countMaterialsCb));
			countMaterialsCb++;
			if (dSides[i])
				commandList->SetPipelineState(pso->getPipelineStateObjectDoubleSides().Get());
			else
				commandList->SetPipelineState(pso->getPipelineStateObject().Get());
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
