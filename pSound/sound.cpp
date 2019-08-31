#include "sound.h"

void SoundDevice::destroy()
{
	for_each(soundBuffer.begin(), soundBuffer.end(), [](auto& p)
		{
			alDeleteBuffers(1, &p.second);
		});
	ALFWShutdownOpenAL();
	ALFWShutdown();
}

bool SoundDevice::init(const Vector3& p, const Vector3& d, const Vector3& h)
{
	ALFWInit();
	if (!ALFWInitOpenAL())
	{
		//destroy();
		return false;
	}
	if (!Listener::getInstance()->init(p, d, h))
		return false;
	return true;
}

bool SoundDevice::addSound(const std::wstring& filename, const Vector3& p, const SoundType& type, bool looped, bool streamed, float v)
{
	// ѕровер€ем файл на наличие
	if (!fs::exists(filename))
		return false;
	ALuint mSourceID;
	fs::path soundFile(filename);
	std::string ext = soundFile.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), toupper);
	std::shared_ptr<SoundLoader> loader;
	if (ext == ".WAV")
		loader.reset(new WavLoader);
	if (!loader->loadFile(filename, mSourceID))
		return false;
	if (soundBuffer.find(filename) == soundBuffer.end())
		soundBuffer.insert({ filename, mSourceID });
	Sound snd(p, mSourceID, looped, streamed);
	snd.setVolume(v);
	switch (type)
	{
	case STATIC_SOUND:
		staticSound.insert({ mSourceID, snd });
		break;

	case ACTOR_SOUND:
		actorSound.insert({ mSourceID, snd });
		break;

	case AMBIENT_SOUND:
		ambientSound.insert({ mSourceID, snd });
		break;
	}
	return true;
}

void SoundDevice::updateListener(const Vector3& p, const Vector3& d, const Vector3& h)
{
	Listener::getInstance()->updatePosition(p, d, h);
}

void SoundDevice::update(const SoundType& type, const std::wstring& key, const Vector3& p)
{
	Sound snd;
	switch (type)
	{
	case ACTOR_SOUND:
		snd = actorSound[soundBuffer[key]];
		break;

	case STATIC_SOUND:
		snd = staticSound[soundBuffer[key]];
		break;

	case AMBIENT_SOUND:
		snd = ambientSound[soundBuffer[key]];
		break;
	}
	snd.move(p);
}

void SoundDevice::play(const SoundType& type, const std::wstring& key)
{
	Sound snd;
	switch (type)
	{
	case ACTOR_SOUND:
		snd = actorSound[soundBuffer[key]];
		break;

	case STATIC_SOUND:
		snd = staticSound[soundBuffer[key]];
		break;

	case AMBIENT_SOUND:
		snd = ambientSound[soundBuffer[key]];
		break;
	}
	if(snd.getStatus() != AL_PLAYING)
		snd.play();
}

void SoundDevice::start()
{
	for_each(staticSound.begin(), staticSound.end(), [](auto& snd)
		{
			snd.second.play();
		});
	for_each(ambientSound.begin(), ambientSound.end(), [](auto& snd)
		{
			snd.second.play();
		});
}

void SoundDevice::setVolumeDevice(float v)
{
	Listener::getInstance()->setVolumeDevice(v);
}




std::shared_ptr<Listener> Listener::instance(nullptr);

bool Listener::init(const Vector3& p, const Vector3& d, const Vector3& h)
{
	// устанавливаем позицию слушател€
	fill(camSpeed.begin(), camSpeed.end(), 0.f);
	updatePosition(p, d, h);
	setVolumeDevice(5.f); // default volume
	return true;
}

void Listener::updatePosition(const Vector3& p, const Vector3& d, const Vector3& h)
{
	camPos = { p[x], p[y], p[z] };
	camOrient = { d[x], d[y], d[z], h[x], h[y], h[z] };

	// позици€
	alListenerfv(AL_POSITION, camPos.data());
	// —корость
	alListenerfv(AL_VELOCITY, camSpeed.data());
	// ќриентаци€
	alListenerfv(AL_ORIENTATION, camOrient.data());
}

void Listener::setVolumeDevice(float v)
{
	alListenerf(AL_GAIN, v); // громкость устройства
}

std::shared_ptr<Listener> Listener::getInstance()
{
	if (!instance)
		instance.reset(new Listener);
	return instance;
}





bool WavLoader::loadFile(const std::wstring& filename, ALuint& buffer)
{
	alGenBuffers(1, &buffer);
	std::string cstrFilename(filename.begin(), filename.end());
	if (!ALFWLoadWaveToBuffer(cstrFilename.c_str(), buffer))
		return false;
	return true;
}



Sound::Sound(const Vector3& p, ALuint& buffer, bool looped, bool streamed)
{
	init(p, buffer, looped, streamed);
}

void Sound::init(const Vector3& p, ALuint & buffer, bool looped, bool streamed)
{
	alGenSources(1, &mSourceID);
	alSourcei(mSourceID, AL_BUFFER, buffer);
	fill(soundSpeed.begin(), soundSpeed.end(), 0.f);
	move(p);
	alSourcefv(mSourceID, AL_VELOCITY, soundSpeed.data()); // скорость звука?
	mLooped = looped;
	alSourcef(mSourceID, AL_PITCH, 1.0f); // тон звука
	alSourcef(mSourceID, AL_GAIN, 50.0f); // усиление звука по мере приближени€ к источнику
	alSourcei(mSourceID, AL_LOOPING, mLooped); // зациклен или нет
}

void Sound::move(const Vector3& p)
{
	soundPos = { p[x], p[y], p[z] };
	alSourcefv(mSourceID, AL_POSITION, soundPos.data()); // позици€ источника
}

void Sound::play()
{
	alSourcePlay(mSourceID);
}

ALint Sound::getStatus()
{
	ALint iState;
	alGetSourcei(mSourceID, AL_SOURCE_STATE, &iState);
	return iState;
}

void Sound::setVolume(float v)
{
	alSourcef(mSourceID, AL_GAIN, v);
}

void Sound::close()
{
	alSourceStop(mSourceID);
	if (alIsSource(mSourceID))
		alDeleteSources(1, &mSourceID);
}

void Sound::stop()
{
	alSourceStop(mSourceID);
}




bool OpenALCheckError::checkALCError(ALCdevice* device)
{
	ALenum ErrCode;
	std::string Err = "ALC error: ";
	if ((ErrCode = alcGetError(device)) != ALC_NO_ERROR)
	{
		Err += (char*)alcGetString(device, ErrCode);
		// error for Qt
		return false;
	}
	return true;
}

bool OpenALCheckError::checkALError()
{
	ALenum ErrCode;
	std::string Err = "OpenAL error: ";
	if ((ErrCode = alGetError()) != AL_NO_ERROR)
	{
		Err += (char*)alGetString(ErrCode);
		// error for Qt
		return false;
	}
	return true;
}


