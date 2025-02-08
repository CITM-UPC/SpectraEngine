#include "ModuleAudio.h"
#include "App.h"

#include <fstream>

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

	LoadAudioBanks();

	//AK::SoundEngine::RegisterGameObj(0);

	AkGameObjectID listenerID = 100;
	AK::SoundEngine::RegisterGameObj(listenerID);
	Set3DPosition(listenerID, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
	AK::SoundEngine::SetDefaultListeners(&listenerID, 1);

	return true;
}

bool ModuleAudio::LoadAudioBanks()
{
	std::ifstream file("Assets\\Audio\\WwiseProject\\GeneratedSoundBanks\\Windows\\SoundbanksInfo.json");
	if (!file.is_open())
	{
		LOG(LogType::LOG_ERROR, "Failed to open SoundbanksInfo.json");
		return false;
	}

	nlohmann::json banksInfo;
	try
	{
		file >> banksInfo;
		file.close();
	}
	catch (const std::exception& e)
	{
		LOG(LogType::LOG_ERROR, "Error parsing SoundbanksInfo.json: %s", e.what());
		return false;
	}

	auto soundBanks = banksInfo["SoundBanksInfo"]["SoundBanks"];

	for (const auto& bankData : soundBanks)
	{
		const std::string shortName = bankData["ShortName"];

		AudioBank* audioBank = new AudioBank();
		audioBank->bankName = shortName;
		audioBank->bankPath = "Assets\\Audio\\WwiseProject\\GeneratedSoundBanks\\Windows\\" + shortName;

		if (bankData.contains("Events"))
		{
			for (const auto& eventData : bankData["Events"])
			{
				std::string eventName = eventData["Name"];

				audioBank->events.push_back(eventName);
			}
		}

		audioBanks.push_back(audioBank);
	}

	AkBankID bankID;

	for (const auto& audioBank : audioBanks)
	{
		AK::SoundEngine::LoadBank(audioBank->bankName.c_str(), bankID);
	}

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

void ModuleAudio::PlayEvent(const char* eventName, AkGameObjectID gameObjectID)
{
	AK::SoundEngine::PostEvent(eventName, gameObjectID);
}

void ModuleAudio::AddAudioComponent(ComponentAudio* audioComponent, AkGameObjectID& gameObjectID)
{
	audioComponents.push_back(audioComponent);
	gameObjectID = audioComponents.size();

	AK::SoundEngine::RegisterGameObj(gameObjectID);
}

void ModuleAudio::Set3DPosition(AkGameObjectID gameObjectID, glm::vec3 position, glm::vec3 forward, glm::vec3 up)
{
	AkSoundPosition soundPos;
	soundPos.SetPosition(position.x, position.y, -position.z);
	soundPos.SetOrientation(forward.x, forward.y, forward.z, up.x, up.y, up.z);

	AK::SoundEngine::SetPosition(gameObjectID, soundPos);
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
