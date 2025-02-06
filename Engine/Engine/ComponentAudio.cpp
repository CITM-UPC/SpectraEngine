#include "ComponentAudio.h"

#include "App.h"

ComponentAudio::ComponentAudio(GameObject* gameObject) : Component(gameObject, ComponentType::AUDIO)
{
	app->audio->AddAudioComponent(this, gameObjectID);
}

ComponentAudio::~ComponentAudio()
{
}

void ComponentAudio::OnEditor()
{
	if (ImGui::CollapsingHeader("Audio Source", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (!app->audio->audioBanks.empty() && ImGui::BeginCombo("Audio Bank", audioBankName.c_str()))
		{
			std::vector<AudioBank*>::const_iterator it;
			for (it = app->audio->audioBanks.begin(); it != app->audio->audioBanks.end(); ++it)
			{
				bool isSelected = audioBankName == (*it)->bankName;

				if (ImGui::Selectable((*it)->bankName.c_str()))
				{
					audioBank = (*it);
					audioBankName = (*it)->bankName;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (audioBank != nullptr && ImGui::BeginCombo("Audio Clip", eventName.c_str()))
		{
			for (const std::string& event : audioBank->events)
			{
				bool isSelected = eventName == event;

				if (ImGui::Selectable(event.c_str()))
					eventName = event;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.f))
			SetVolume(volume);
	}
}

void ComponentAudio::Play()
{
	AK::SoundEngine::PostEvent(eventName.c_str(), gameObjectID);
}

void ComponentAudio::SetVolume(float newVolume)
{
	AK::SoundEngine::SetGameObjectOutputBusVolume(gameObjectID, AK_INVALID_GAME_OBJECT, newVolume);
}