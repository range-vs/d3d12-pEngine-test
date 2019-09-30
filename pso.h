#pragma once

#include "shaders.h"
#include "textures.h"
#include "cBufferFactory.h"
#include "sBufferFactory.h"
#include "loaders.h"

class PipelineStateObject
{
	ComPtr<ID3D12PipelineState> pipelineStateObject; // pso containing a pipeline state
	ComPtr<ID3D12PipelineState> pipelineStateObjectDoubleSides;
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
			rp.InitAsShaderResourceView(i + 1);
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
		auto rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState = rasterizer; // a default rasterizer state.
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
		psoDesc.NumRenderTargets = 1; // we are only binding one render target
		psoDesc.SampleDesc = sDesc;
		/*psoDesc.RasterizerState.MultisampleEnable = msaaState;
		psoDesc.SampleDesc.Count = msaaState ? msaaQuality[0] : 1;
		psoDesc.SampleDesc.Quality = msaaState ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN : 0;*/
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state;

		// create the pso
		if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineStateObject.GetAddressOf()))))
			return false;

		rasterizer.CullMode = D3D12_CULL_MODE_NONE; // double sides
		psoDesc.RasterizerState = rasterizer; // a default rasterizer state.
		if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineStateObjectDoubleSides.GetAddressOf()))))
			return false;

		return true;
	}

	ComPtr<ID3D12PipelineState>& getPipelineStateObject() { return pipelineStateObject; }
	ComPtr<ID3D12PipelineState>& getPipelineStateObjectDoubleSides() { return pipelineStateObjectDoubleSides; }
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
