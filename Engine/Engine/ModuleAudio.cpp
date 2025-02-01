#include "ModuleAudio.h"

#include "App.h"

ModuleAudio::ModuleAudio(App* app) : Module(app)
{
}

ModuleAudio::~ModuleAudio()
{
	Terminate();
}

bool ModuleAudio::Start()
{
	if (!InitMemoryManager())
		return false;

	if (!InitStreamingManager())
		return false;

	if (!InitLowLevelIO())
		return false;

	if (!InitSoundEngine())
		return false;

	if (!InitMusicEngine())
		return false;

	if (!InitSpatialAudio())
		return false;

	if (!InitCommunication())
		return false;

	g_lowLevelIO->SetBasePath(AKTEXT("Assets\\Audio\\WwiseProject\\GeneratedSoundBanks\\Windows"));

	AkBankID bankID;
	AK::SoundEngine::LoadBank("Init.bnk", bankID);
	AK::SoundEngine::LoadBank("SpectraEngine.bnk", bankID);

	return true;
}

void ModuleAudio::Terminate() const
{
#ifndef AK_OPTIMIZED
	AK::Comm::Term();
#endif

	AK::SoundEngine::ClearBanks();

	AK::MusicEngine::Term();

	AK::SoundEngine::Term();

	g_lowLevelIO->Term();
	delete g_lowLevelIO;

	if (AK::IAkStreamMgr::Get())
		AK::IAkStreamMgr::Get()->Destroy();

	AK::MemoryMgr::Term();
}

void ModuleAudio::ProcessAudio()
{
	AK::SoundEngine::RenderAudio();
}

bool ModuleAudio::InitMemoryManager()
{
	AkMemSettings memSettings;
	AK::MemoryMgr::GetDefaultSettings(memSettings);

	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		LOG(LogType::LOG_ERROR, "Could not create the memory manager.");
		return false;
	}
	return true;
}

bool ModuleAudio::InitStreamingManager()
{
	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);

	if (!AK::StreamMgr::Create(stmSettings))
	{
		LOG(LogType::LOG_ERROR, "Could not create the Streaming Manager.");
		return false;
	}
	return true;
}

bool ModuleAudio::InitLowLevelIO()
{
	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

	g_lowLevelIO = new CAkFilePackageLowLevelIODeferred();
	if (g_lowLevelIO->Init(deviceSettings) != AK_Success)
	{
		LOG(LogType::LOG_ERROR, "Could not create the streaming device and Low-Level I/O system");
		return false;
	}

	return true;
}

bool ModuleAudio::InitSoundEngine()
{
	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

	if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
	{
		LOG(LogType::LOG_ERROR, "Could not initialize the Sound Engine.");
		return false;
	}
	return true;
}

bool ModuleAudio::InitMusicEngine()
{
	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);

	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		LOG(LogType::LOG_ERROR, "Could not initialize the Music Engine.");
		return false;
	}
	return true;
}

bool ModuleAudio::InitSpatialAudio()
{
	AkSpatialAudioInitSettings settings;
	if (AK::SpatialAudio::Init(settings) != AK_Success)
	{
		LOG(LogType::LOG_ERROR, "Could not initialize the Spatial Audio.");
		return false;
	}
	return true;
}

bool ModuleAudio::InitCommunication()
{
#ifndef AK_OPTIMIZED
	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);

	AKPLATFORM::SafeStrCpy(commSettings.szAppNetworkName, "SpectraEngine", AK_COMM_SETTINGS_MAX_STRING_SIZE);

	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		LOG(LogType::LOG_ERROR, "Could not initialize communication.");
		return false;
	}
#endif // AK_OPTIMIZED

	return true;
}
