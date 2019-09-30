#pragma once

#include "types.h"

struct Material
{
	Vector ambient;
	Vector diffuse;
	Vector specular;
	float shininess;

	Material() :ambient(), diffuse(), specular(), shininess(0.f) {}
	Material(const Vector& a, const Vector& d, const Vector& s, const float& sn) :ambient(a), diffuse(d), specular(s), shininess(sn) {}
};

class ContainerMaterials
{
	map<wstring, Material> materials;

public:
	ContainerMaterials() {}
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
	bool getFirst(wstring& name)
	{
		if (materials.size() > 0)
		{
			name = materials.begin()->first;
			return true;
		}
		return false;
	}
};