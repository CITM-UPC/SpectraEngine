#include "ComponentAudio.h"

#include "App.h"

#include <fstream>

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
				{
					eventName = event;
					duration = GetEventDurationInfo();
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (ImGui::Checkbox("Mute", &mute))
			SetVolume(mute ? 0.f : volume);

		if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.f))
			SetVolume(volume);

		if (duration != "")
			ImGui::Text("Duration: %s", duration.c_str());
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

std::string ComponentAudio::GetEventDurationInfo()
{
	std::ifstream file(audioBank->bankPath + ".json");

	if (!file.is_open())
		return "Error: Could not open JSON file";

	nlohmann::json banksInfo;

	file >> banksInfo;
	file.close();

	auto soundBanks = banksInfo["SoundBanksInfo"]["SoundBanks"];
	for (const auto& bank : soundBanks) {
		if (bank.contains("Events")) {
			for (const auto& event : bank["Events"]) {
				if (event["Name"] == eventName) {
					if (event.contains("DurationType")) {
						std::string durationType = event["DurationType"];
						if (durationType == "Infinite") {
							return "Loop Infinite";
						}
						else if (durationType == "OneShot") {
							if (event.contains("DurationMin")) {
								return std::to_string(std::stoi(event["DurationMin"].get<std::string>())) + "s";
							}

						}
					}
				}
			}
		}
	}
	
	return "";
}

