#pragma once

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
#include <experimental/filesystem>

#include "sBufferTypes.h"

#include "DDSTextureLoader.h"

#include "pSound/sound.h"

using Microsoft::WRL::ComPtr;
using namespace std;
namespace fs = experimental::filesystem;

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "D3Dcompiler.lib")

using uint = unsigned int;
using uint8 = unsigned char;
using uint64 = unsigned long long;

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

