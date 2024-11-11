#include "InspectorWindow.h"
#include "GameObject.h"
#include "App.h"

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
	}

	ImGui::End();
}