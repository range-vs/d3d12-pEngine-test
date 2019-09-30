#pragma once

#include "materials.h"
#include "camera.h"
#include "sun.h"
#include "torch.h"
// #include "light.h"

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

enum GameQualityConst : int
{
	QUALITY_VERY_LOW = 0,
	QUALITY_LOW
};

class GameQuality
{

private:
	map< GameQualityConst, wstring> foldersShader;
	GameQualityConst currentQuality;

public:
	GameQuality(const GameQualityConst& cq) : currentQuality(cq)
	{
		foldersShader.insert({ QUALITY_VERY_LOW , L"very-low" });
		foldersShader.insert({ QUALITY_LOW , L"low" });
	}

	wstring& getFolder()
	{
		return foldersShader[currentQuality];
	}
};

using GameQualityPtr = shared_ptr< GameQuality>;
