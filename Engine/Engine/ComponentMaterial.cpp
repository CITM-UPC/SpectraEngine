#include "ComponentMaterial.h"
#include "GameObject.h"
#include "App.h"
#include "ModuleRenderer3D.h"

#include <windows.h>
#include <shellapi.h>
#include <algorithm>

ComponentMaterial::ComponentMaterial(GameObject* gameObject) : Component(gameObject, ComponentType::MATERIAL), materialTexture(nullptr), textureId(-1)
{
}

ComponentMaterial::~ComponentMaterial()
{
	app->resources->ModifyResourceUsageCount(materialTexture, -1);
	materialTexture = nullptr;
}

void ComponentMaterial::Update()
{
}

void ComponentMaterial::OnEditor()
{
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (materialTexture->textureId != -1)
		{
			ImGui::Text("Path: %s", materialTexture->texturePath);
			ImGui::Text("Texture Size: %i x %i", materialTexture->textureWidth, materialTexture->textureHeight);
			ImGui::Image((ImTextureID)(uintptr_t)materialTexture->textureId, ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));

			if (ImGui::MenuItem("Show in Explorer"))
			{
				std::string path = std::string(materialTexture->texturePath);
				std::replace(path.begin(), path.end(), '/', '\\');
				std::string command = "explorer /select,\"" + path + "\"";
				system(command.c_str());
			}

			if (ImGui::MenuItem("Open Image"))
			{
				std::string path = std::string(materialTexture->texturePath);
				std::replace(path.begin(), path.end(), '/', '\\');
				std::string command = "explorer \"" + path + "\"";
				system(command.c_str());
			}

			if (ImGui::Checkbox("Show Checkers Texture", &showCheckersTexture))
			{
				textureId = showCheckersTexture ? app->renderer3D->checkerTextureId : materialTexture->textureId;
			}

			ImGui::ColorEdit4("Material Color", &gameObject->mesh->mesh->diffuseColor[0]);
		}
	}
}

void ComponentMaterial::AddTexture(Texture* texture)
{
	if (gameObject->GetComponent(ComponentType::MESH))
	{
		if (!gameObject->GetComponent(ComponentType::MATERIAL))
		{
			gameObject->AddComponent(gameObject->material);
		}

		app->resources->ModifyResourceUsageCount(materialTexture, -1);
		app->resources->ModifyResourceUsageCount(texture, 1);
		materialTexture = texture;
		textureId = materialTexture->textureId;
	}

	for (auto& child : gameObject->children)
	{
		if (!child->GetComponent(ComponentType::MATERIAL))
		{
			child->AddComponent(child->material);
		}

		app->resources->ModifyResourceUsageCount(child->material->materialTexture, -1);
		app->resources->ModifyResourceUsageCount(texture, 1);
		child->material->materialTexture = texture;
		child->material->textureId = texture->textureId;
	}
}

void ComponentMaterial::Serialize(nlohmann::json& json) const
{
	Component::Serialize(json);
	json["texture"] = materialTexture ? materialTexture->GetLibraryFileDir() : "";
}

void ComponentMaterial::Deserialize(const nlohmann::json& json)
{
	Component::Deserialize(json);
	std::string texturePath = json["texture"].get<std::string>();
	if (!texturePath.empty())
	{
		AddTexture(dynamic_cast<Texture*>(app->resources->FindResourceInLibrary(texturePath, ResourceType::TEXTURE)));
	}
}