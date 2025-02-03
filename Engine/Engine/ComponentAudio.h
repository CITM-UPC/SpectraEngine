#pragma once

#include "Component.h"

class ComponentAudio : public Component
{
public:
	ComponentAudio(GameObject* gameObject);
	~ComponentAudio();

	void OnEditor() override;
};
