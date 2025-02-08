#pragma once

#include "Component.h"
#include "ModuleAudio.h"

class ComponentAudioSource : public Component
{
public:
	ComponentAudioSource(GameObject* gameObject);
	~ComponentAudioSource();

	void Update() override;
	void OnEditor() override;

	void Play();

	void UpdatePosition();

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
