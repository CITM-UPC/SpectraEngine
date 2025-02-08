#pragma once

#include "Component.h"
#include "ModuleAudio.h"

class ComponentAudioListener : public Component
{
public:
	ComponentAudioListener(GameObject* gameObject);
	~ComponentAudioListener();

	void OnEditor() override;

	void UpdatePosition();

private:
	AkGameObjectID gameObjectID = 0;
};
