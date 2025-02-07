#pragma once

#include "Component.h"
#include "ModuleAudio.h"

class ComponentAudio : public Component
{
public:
	ComponentAudio(GameObject* gameObject);
	~ComponentAudio();

	void OnEditor() override;

	void Play();

private:
	void SetVolume(float newVolume);
	std::string GetEventDurationInfo();

	AkGameObjectID gameObjectID = 0;

	std::string audioBankName;
	std::string eventName;
	AudioBank* audioBank = nullptr;
	float volume = 1.f;
	bool mute = false;
	std::string duration;
};
