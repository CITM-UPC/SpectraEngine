#include "InspectorWindow.h"
#include "GameObject.h"
#include "App.h"

#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"

InspectorWindow::InspectorWindow(const WindowType type, const std::string& name) : EditorWindow(type, name)
{
}

InspectorWindow::~InspectorWindow()
{
}

void InspectorWindow::DrawWindow()
{
	ImGui::Begin(name.c_str());

	UpdateMouseState();

	if (app->editor->selectedGameObject != nullptr && app->editor->selectedGameObject->parent != nullptr)
	{
		ImGui::Checkbox("##Active", &app->editor->selectedGameObject->isActive);
		ImGui::SameLine();

		strcpy_s(inputName, app->editor->selectedGameObject->name.c_str());

		if (ImGui::InputText("##InspectorName", inputName, sizeof(inputName), inputTextFlags)
			|| (isEditingInspector && !ImGui::IsItemActive() && !ImGui::IsAnyItemActive()))
		{
			if (inputName[0] != '\0') app->editor->selectedGameObject->name = inputName;
			isEditingInspector = false;
		}

		if (ImGui::IsItemClicked())
		{
			isEditingInspector = true;
			ImGui::SetKeyboardFocusHere(-1);
		}
		
		ImGui::SameLine();
		ImGui::Checkbox("Static", &app->editor->selectedGameObject->isStatic);

		for (auto i = 0; i < app->editor->selectedGameObject->components.size(); i++)
		{
			app->editor->selectedGameObject->components[i]->OnEditor();
		}

		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{
			if (ImGui::MenuItem("Audio Source"))
			{
				ComponentAudioSource* audioSource = new ComponentAudioSource(app->editor->selectedGameObject);
				app->editor->selectedGameObject->AddComponent(audioSource);
			}
			if (ImGui::MenuItem("Audio Listener"))
			{
				ComponentAudioListener* audioListener = new ComponentAudioListener(app->editor->selectedGameObject);
				app->editor->selectedGameObject->AddComponent(audioListener);
			}
			ImGui::EndPopup();
		}
	}

	ImGui::End();
}