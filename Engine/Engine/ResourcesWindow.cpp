#include "ResourcesWindow.h"
#include "ModuleResources.h"
#include "App.h"
#include "imgui.h"

ResourcesWindow::ResourcesWindow(const WindowType type, const std::string& name) : EditorWindow(type, name)
{
}

ResourcesWindow::~ResourcesWindow()
{
}

void ResourcesWindow::DrawWindow()
{
	ImGui::Begin(name.c_str());

	DrawResourceUsageTable();

	ImGui::End();
}

void ResourcesWindow::DrawResourceUsageTable()
{
	if (ImGui::BeginTable("Resource Usage", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("Resource");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("Usage Count");
		ImGui::TableHeadersRow();

		for (const auto& resource : app->resources->GetResources())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(resource->GetLibraryFileDir().c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(ResourceTypeToString(resource->GetType()).c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%d", app->resources->GetResourceUsageCount(resource));
		}

		ImGui::EndTable();
	}
}

std::string ResourcesWindow::ResourceTypeToString(const ResourceType type)
{
	switch (type)
	{
	case ResourceType::TEXTURE: return "Texture";
	case ResourceType::MODEL: return "Model";
	case ResourceType::MESH: return "Mesh";
	default: return "Unknown";
	}
}