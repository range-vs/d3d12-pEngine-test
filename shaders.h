#pragma once

#include "types.h"

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
	PairShaders() {}
	PairShaders(const wstring& vs, const wstring& ps) :vShader(vs), pShader(ps) {}
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
		path += L'.' + type;
		if (!fs::exists(path))
		{
			// error
			OutputDebugString((L"Shader " + path + L" not found. Terminated.\n").c_str());
			return false;
		}
		HRESULT hr = D3DCompileFromFile(path.c_str(),
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