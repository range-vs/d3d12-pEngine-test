#pragma once

#include <al.h>
#include <alc.h>

#include "Framework.h"

#include "../math_graphics/math_graphics/mathematics.h"
using namespace gmath;

#include <array>
#include <algorithm>
#include <memory>
#include <fstream>
#include <experimental/filesystem>
#include <map>

#pragma comment(lib, "OpenAL32.lib")

namespace fs = std::experimental::filesystem;


#if defined(DYNAMIC_LIBRARY_PSOUND)
#  define DYNLIB_PSOUND __declspec(dllexport)
#else
#  define DYNLIB_PSOUND __declspec(dllimport)
#endif



enum SoundType: int
{
	STATIC_SOUND = 0,
	ACTOR_SOUND,
	AMBIENT_SOUND
};

class Sound;
class Listener;

class DYNLIB_PSOUND SoundDevice
{
	std::map< ALuint, Sound> staticSound;
	std::map< ALuint, Sound> actorSound;
	std::map< ALuint, Sound> ambientSound;
	std::map< std::wstring, ALuint> soundBuffer;

	void destroy();
public:
	bool init(const Vector3& p, const Vector3& d, const Vector3& h, float volumeDevice = 5.f);
	bool addSound(const std::wstring& filename, const Vector3& p, const SoundType& type, bool looped, bool streamed, float v);
	void updateListener(const Vector3& p, const Vector3& d, const Vector3& h);
	void update(const SoundType& type, const std::wstring& key, const Vector3& p);
	void play(const SoundType& type, const std::wstring& key);
	void start();
	void setVolumeDevice(float v);
};


class Listener
{
	std::vector<ALfloat> camPos;
	std::vector<ALfloat> camSpeed;
	std::vector<ALfloat> camOrient;
	//ALCdevice* device;
	//ALCcontext* deviceContext;

	static std::shared_ptr<Listener> instance;

	Listener() {}
	Listener(const Listener&);
	Listener& operator=(const Listener&);
public:

	bool init(const Vector3& p, const Vector3& d, const Vector3& h, float volumeDevice);
	void updatePosition(const Vector3& p, const Vector3& d, const Vector3& h);
	void setVolumeDevice(float v);

	static std::shared_ptr<Listener> getInstance();

	~Listener()
	{
		//OutputDebugString("delete listener");
	}
};


class SoundLoader
{
public:
	virtual bool loadFile(const std::wstring& filename, ALuint& buffer) = 0;
};

class WavLoader: public SoundLoader
{
public:
	bool loadFile(const std::wstring& filename, ALuint& buffer) override;
};


class DYNLIB_PSOUND Sound
{
	std::vector<ALfloat> soundPos; // позиция звука
	std::vector<ALfloat> soundSpeed; // скорость звука
	bool  mLooped;
	ALuint mSourceID;	// Идентификатор источника
	bool mStreamed; // Потоковый ли звук


public:
	Sound(){}
	Sound(const Vector3& p, ALuint& buffer, bool looped, bool streamed);
	void init(const Vector3& p, ALuint& buffer, bool looped, bool streamed);

	bool isStreamed();
	void play();
	ALint getStatus();
	void setVolume(float v);
	void close();
	void move(const Vector3& p);
	void stop();

};

// todo - rewrite
class DYNLIB_PSOUND OpenALCheckError
{
public:
	static bool checkALCError(ALCdevice* device);
	static bool checkALError();
};

