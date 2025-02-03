#include "ComponentAudio.h"

#include "App.h"

ComponentAudio::ComponentAudio(GameObject* gameObject) : Component(gameObject, ComponentType::AUDIO)
{
}

ComponentAudio::~ComponentAudio()
{
}

void ComponentAudio::OnEditor()
{
	
}