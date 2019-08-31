/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Framework.h"

#include <algorithm>
#include <memory>
#include <array>
#include <algorithm>
#include <memory>
#include <fstream>
#include <experimental/filesystem>
#include <map>

namespace fs = std::experimental::filesystem;

#include "../math_graphics/math_graphics/mathematics.h"
using namespace gmath;

#include "../pSound/sound.h"

using namespace std;

#define	TEST_WAVE_FILE		"rnd_dog3.wav"


//enum SoundType : int
//{
//	STATIC_SOUND = 0,
//	ACTOR_SOUND
//};
//
//class Sound;
//class Listener;
//
//class SoundDevice
//{
//	std::multimap< ALuint, Sound> staticSound;
//	std::multimap< ALuint, Sound> actorSound;
//	std::map< std::wstring, ALuint> soundBuffer;
//	std::shared_ptr<Listener> listener;
//
//	void destroy();
//public:
//	bool init(const Vector3& p, const Vector3& d, const Vector3& h);
//	bool addSound(const std::wstring& filename, const Vector3& p, const SoundType& type, bool looped, bool streamed);
//	void update(const Vector3& p, const Vector3& d, const Vector3& h);
//	void setVolume(float v);
//};
//
//
//class Listener
//{
//	std::vector<ALfloat> camPos;
//	std::vector<ALfloat> camSpeed;
//	std::vector<ALfloat> camOrient;
//public:
//
//	bool init(const Vector3& p, const Vector3& d, const Vector3& h);
//	void updatePosition(const Vector3& p, const Vector3& d, const Vector3& h);
//	void setVolume(float v);
//
//	~Listener()
//	{
//		//OutputDebugString("delete listener");
//	}
//};
//
//
//class SoundLoader
//{
//public:
//	virtual bool loadFile(const std::wstring& filename, ALuint& buffer) = 0;
//};
//
//class WavLoader : public SoundLoader
//{
//public:
//	bool loadFile(const std::wstring& filename, ALuint& buffer) override;
//};
//
//
//class Sound
//{
//	std::vector<ALfloat> soundPos; // позиция звука
//	std::vector<ALfloat> soundSpeed; // скорость звука
//	bool  mLooped;
//	ALuint mSourceID;	// Идентификатор источника
//	bool mStreamed; // Потоковый ли звук
//
//
//public:
//	Sound(const Vector3& p, ALuint& buffer, bool looped, bool streamed);
//	void init(const Vector3& p, ALuint& buffer, bool looped, bool streamed);
//
//	bool isStreamed();
//	void play();
//	void update();
//	void update_debug();
//	void close();
//	void move(const Vector3& p);
//	void stop();
//
//};
//



//int main()
//{
//	ALuint      uiBuffer;
//	ALuint      uiSource;
//	ALint       iState;
//
//	// Initialize Framework
//	ALFWInit();
//
//	ALFWprintf("PlayStatic Test Application\n");
//
//	if (!ALFWInitOpenAL())
//	{
//		ALFWprintf("Failed to initialize OpenAL\n");
//		ALFWShutdown();
//		return 0;
//	}
//
//	ALfloat p [] = { 0,0,0 };
//	alListenerfv(AL_POSITION, p);
//	// Скорость
//	ALfloat s[] = { 0,0,0 };
//	alListenerfv(AL_VELOCITY, s);
//	// Ориентация
//	ALfloat t[] = { 0,0,0,0,1,0 };
//	alListenerfv(AL_ORIENTATION, t);
//
//	// Generate an AL Buffer
//	alGenBuffers(1, &uiBuffer);
//
//	// Load Wave file into OpenAL Buffer
//	if (!ALFWLoadWaveToBuffer(TEST_WAVE_FILE, uiBuffer))
//	{
//		ALFWprintf("Failed to load %s\n", TEST_WAVE_FILE);
//	}
//
//	// Generate a Source to playback the Buffer
//	alGenSources(1, &uiSource);
//
//	// Attach Source to Buffer
//	alSourcei(uiSource, AL_BUFFER, uiBuffer);
//
//	alSourcef(uiSource, AL_GAIN, 50.f); // расстояние для затухания
//	alListenerf(AL_GAIN, 5.f); // громкость устройства
//	alSourcei(uiSource, AL_LOOPING, true); // зациклен или нет
//
//	// Play Source
//	alSourcePlay(uiSource);
//	ALFWprintf("Playing Source ");
//
//	ALfloat ps[] = { 0,0,0 };
//	alSourcefv(uiSource, AL_POSITION, ps); // позиция источника
//
//	do
//	{
//		Sleep(100);
//		//ps[2] --;
//		//alSourcefv(uiSource, AL_POSITION, ps); // позиция источника
//		alGetSourcei(uiSource, AL_SOURCE_STATE, &iState); // проигрываем часть звука
//	} while (iState == AL_PLAYING);
//
//	ALFWprintf("\n");
//
//	// Clean up by deleting Source(s) and Buffer(s)
//	alSourceStop(uiSource);
//	alDeleteSources(1, &uiSource);
//	alDeleteBuffers(1, &uiBuffer);
//
//	ALFWShutdownOpenAL();
//
//	ALFWShutdown();
//
//	return 0;
//}



int main()
{
	shared_ptr<SoundDevice> sndDevice;
	sndDevice.reset(new SoundDevice);
	if (!sndDevice->init({ 0,0,0 }, {0, 0, 0}, { 0,1,0 }))
		return 1;
	if (!sndDevice->addSound(L"rnd_night_1.wav", { 0.f, 0.f, 0.f }, STATIC_SOUND, true, false, 3))
		return 1;

	while (true)
	{
		//Sleep(10);
	}


	return 0;
}


//void SoundDevice::destroy()
//{
//	for_each(soundBuffer.begin(), soundBuffer.end(), [](auto& p)
//		{
//			alDeleteBuffers(1, &p.second);
//		});
//	ALFWShutdownOpenAL();
//	ALFWShutdown();
//}
//
//bool SoundDevice::init(const Vector3& p, const Vector3& d, const Vector3& h)
//{
//	ALFWInit();
//	if (!ALFWInitOpenAL())
//	{
//		//destroy();
//		return false;
//	}	
//	listener.reset(new Listener);
//	if (!listener->init(p, d, h))
//		return false;
//
//	//std::vector<ALfloat> camPos = { p[x], p[y], p[z] };
//	//std::vector<ALfloat> camSpeed = { 0,0,0 };
//	//std::vector<ALfloat> camOrient = { d[x], d[y], d[z], h[x], h[y], h[z] };
//
//	//camPos = { p[x], p[y], p[z] };
//
//	//// позиция
//	//alListenerfv(AL_POSITION, camPos.data());
//	//// Скорость
//	//alListenerfv(AL_VELOCITY, camSpeed.data());
//	//// Ориентация
//	//alListenerfv(AL_ORIENTATION, camOrient.data());
//
//	//ALfloat p1[] = { 0,0,0 };
//	//alListenerfv(AL_POSITION, p1);
//	//// Скорость
//	//ALfloat s[] = { 0,0,0 };
//	//alListenerfv(AL_VELOCITY, s);
//	//// Ориентация
//	//ALfloat t[] = { 0,0,0,0,1,0 };
//	//alListenerfv(AL_ORIENTATION, t);
//	return true;
//}
//
//bool SoundDevice::addSound(const std::wstring& filename, const Vector3& p, const SoundType& type, bool looped, bool streamed)
//{
//	// Проверяем файл на наличие
//	if (!fs::exists(filename))
//		return false;
//	ALuint mSourceID;
//	fs::path soundFile(filename);
//	std::string ext = soundFile.extension().string();
//	std::transform(ext.begin(), ext.end(), ext.begin(), toupper);
//	std::shared_ptr<SoundLoader> loader;
//	if (ext == ".WAV")
//		loader.reset(new WavLoader);
//	if (!loader->loadFile(filename, mSourceID))
//		return false;
//	if (soundBuffer.find(filename) == soundBuffer.end())
//		soundBuffer.insert({ filename, mSourceID });
//	switch (type)
//	{
//	case STATIC_SOUND:
//		staticSound.insert({ mSourceID , Sound(p, mSourceID , looped, streamed) });
//		break;
//
//	case ACTOR_SOUND:
//		actorSound.insert({ mSourceID , Sound(p, mSourceID , looped, streamed) });
//		break;
//	}
//	return true;
//}
//
//void SoundDevice::update(const Vector3& p, const Vector3& d, const Vector3& h)
//{
//	//listener->updatePosition(p, d, h);
//	staticSound.begin()->second.update_debug();
//	/*for_each(staticSound.begin(), staticSound.end(), [](auto& snd)
//		{
//			snd.second.update();
//		});
//	for_each(actorSound.begin(), actorSound.end(), [&p](auto& snd)
//		{
//			snd.second.move(p);
//			snd.second.update();
//		});*/
//}
//
//void SoundDevice::setVolume(float v)
//{
//	listener->setVolume(v);
//}
//
//
//bool Listener::init(const Vector3& p, const Vector3& d, const Vector3& h)
//{
//	// устанавливаем позицию слушателя
//	fill(camSpeed.begin(), camSpeed.end(), 0.f);
//	updatePosition(p, d, h);
//	setVolume(5.f); // default volume
//	return true;
//}
//
//void Listener::updatePosition(const Vector3& p, const Vector3& d, const Vector3& h)
//{
//	camPos = { p[x], p[y], p[z] };
//	camOrient = { d[x], d[y], d[z], h[x], h[y], h[z] };
//
//	// позиция
//	alListenerfv(AL_POSITION, camPos.data());
//	// Скорость
//	alListenerfv(AL_VELOCITY, camSpeed.data());
//	// Ориентация
//	alListenerfv(AL_ORIENTATION, camOrient.data());
//}
//
//void Listener::setVolume(float v)
//{
//	alListenerf(AL_GAIN, v); // громкость устройства
//}
//
//bool WavLoader::loadFile(const std::wstring& filename, ALuint& buffer)
//{
//	alGenBuffers(1, &buffer);
//	std::string cstrFilename(filename.begin(), filename.end());
//	if (!ALFWLoadWaveToBuffer(cstrFilename.c_str(), buffer))
//		return false;
//	return true;
//}
//
//
//
//Sound::Sound(const Vector3& p, ALuint& buffer, bool looped, bool streamed)
//{
//	init(p, buffer, looped, streamed);
//}
//
//void Sound::init(const Vector3& p, ALuint& buffer, bool looped, bool streamed)
//{
//	alGenSources(1, &mSourceID);
//	alSourcei(mSourceID, AL_BUFFER, buffer);
//	fill(soundSpeed.begin(), soundSpeed.end(), 0.f);
//	move(p);
//	alSourcefv(mSourceID, AL_VELOCITY, soundSpeed.data()); // скорость звука?
//	mLooped = looped;
//	alSourcef(mSourceID, AL_PITCH, 1.0f); // тон звука
//	alSourcef(mSourceID, AL_GAIN, 50.0f); // усиление звука по мере приближения к источнику
//	alSourcei(mSourceID, AL_LOOPING, mLooped); // зациклен или нет
//	play();
//}
//
//void Sound::move(const Vector3& p)
//{
//	soundPos = { p[x], p[y], p[z] };
//	alSourcefv(mSourceID, AL_POSITION, soundPos.data()); // позиция источника
//}
//
//void Sound::play()
//{
//	alSourcePlay(mSourceID);
//}
//
//void Sound::update()
//{
//	ALint iState;
//	alGetSourcei(mSourceID, AL_SOURCE_STATE, &iState);
//	//if (iState != AL_PLAYING) // todo: переделать для циклических звуков
//	//	stop();
//}
//
//void Sound::update_debug()
//{
//	ALint iState;
//	do
//	{
//		Sleep(100);
//		alGetSourcei(mSourceID, AL_SOURCE_STATE, &iState); // проигрываем часть звука
//	} while (iState == AL_PLAYING);
//}
//
//void Sound::close()
//{
//	alSourceStop(mSourceID);
//	if (alIsSource(mSourceID))
//		alDeleteSources(1, &mSourceID);
//}
//
//void Sound::stop()
//{
//	alSourceStop(mSourceID);
//}
