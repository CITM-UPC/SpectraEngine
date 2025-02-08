#include "ComponentAudioListener.h"

#include "App.h"

ComponentAudioListener::ComponentAudioListener(GameObject* gameObject) : Component(gameObject, ComponentType::AUDIO_LISTENER)
{
	app->audio->AddAudioListenerComponent(this, gameObjectID);
	UpdatePosition();
}

ComponentAudioListener::~ComponentAudioListener()
{
}

void ComponentAudioListener::OnEditor()
{
	ImGui::CollapsingHeader("Audio Listener", ImGuiTreeNodeFlags_DefaultOpen);
}

void ComponentAudioListener::UpdatePosition()
{
	app->audio->Set3DPosition(gameObjectID, gameObject->transform->GetGlobalPosition(), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
}