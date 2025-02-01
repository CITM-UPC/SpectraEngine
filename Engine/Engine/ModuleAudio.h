#pragma once

#include "Module.h"

#include <AK/SoundEngine/Common/AkTypes.h>

#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif // AK_OPTIMIZED

#if AK_WWISESDK_VERSION_MAJOR < 2024
#include <AK/SoundEngine/Common/AkModule.h>
#else
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#endif

#include <AkFilePackageLowLevelIODeferred.h>

class ModuleAudio : public Module
{
public:

	ModuleAudio(App* app);
	virtual ~ModuleAudio();

	bool Start() override;

	void Terminate() const;

	void ProcessAudio();

	void PlayEvent(const char* eventName, AkGameObjectID gameObjectID);

private:
	bool InitMemoryManager();
	bool InitStreamingManager();
	bool InitLowLevelIO();
	bool InitSoundEngine();
	bool InitMusicEngine();
	bool InitSpatialAudio();
	bool InitCommunication();

	CAkFilePackageLowLevelIODeferred* g_lowLevelIO = nullptr;
};
