#include "ComponentTransform.h"

#include "App.h"
#include "GameObject.h"

ComponentTransform::ComponentTransform(GameObject* gameObject) : Component(gameObject, ComponentType::TRANSFORM)
{
	localTransform = glm::float4x4(1.0f);
	globalTransform = glm::float4x4(1.0f);

	Decompose(globalTransform, position, rotation, scale);

	eulerRotation = eulerAngles(rotation);
}

ComponentTransform::~ComponentTransform()
{
}

void ComponentTransform::Update()
{
}

void ComponentTransform::OnEditor()
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* labels[] = { "X", "Y", "Z" };

		// Position
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Position      ");
		ImGui::PushItemWidth(40);

		for (int i = 0; i < 3; ++i) {
			ImGui::SameLine();
			SetButtonColor(labels[i]);
			ImGui::SameLine();
			float floatValue = position[i];
			std::string formattedValue = std::to_string(floatValue);
			formattedValue.erase(formattedValue.find_last_not_of('0') + 1, std::string::npos);
			formattedValue.erase(formattedValue.find_last_not_of('.') + 1, std::string::npos);
			if (ImGui::DragFloat(std::string("##position_").append(labels[i]).c_str(), &position[i], 0.1f, 0.0f, 0.0f, formattedValue.c_str()))
			{
				updateTransform = true;
			}
		}
		ImGui::PopItemWidth();

		// Rotation
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Rotation      ");
		ImGui::PushItemWidth(40);

		for (int i = 0; i < 3; ++i) {
			ImGui::SameLine();
			SetButtonColor(labels[i]);
			ImGui::SameLine();
			float eulerAngle = eulerRotation[i];
			if (eulerAngle == -0.0f) {
				eulerAngle = 0.0f;
			}

			std::string formattedValue = std::to_string(eulerAngle);
			formattedValue.erase(formattedValue.find_last_not_of('0') + 1, std::string::npos);
			formattedValue.erase(formattedValue.find_last_not_of('.') + 1, std::string::npos);
			if (ImGui::DragFloat(std::string("##rotation_").append(labels[i]).c_str(), &eulerRotation[i], 0.1f, 0.0f, 0.0f, formattedValue.c_str()))
			{
				updateTransform = true;
			}
		}
		ImGui::PopItemWidth();

		// Scale
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Scale  ");
		ImGui::SameLine();

		ImGui::Checkbox("##Constrained", &constrainedProportions);
		scaleInfoTag.ShowInfoTag("Constrained Proportions");

		if (constrainedProportions) {
			for (int i = 0; i < 3; ++i) {
				initialScale[i] = scale[i];
			}
		}
		
		static float previousScales[3];
		previousScales[0] = scale.x;
		previousScales[1] = scale.y;
		previousScales[2] = scale.z;

		ImGui::PushItemWidth(40);

		bool scaleChanged = false;
		for (int i = 0; i < 3; ++i) {
			ImGui::SameLine();
			SetButtonColor(labels[i]);
			ImGui::SameLine();
			float floatValue = scale[i];
			std::string formattedValue = std::to_string(floatValue);
			formattedValue.erase(formattedValue.find_last_not_of('0') + 1, std::string::npos);
			formattedValue.erase(formattedValue.find_last_not_of('.') + 1, std::string::npos);
			if (ImGui::DragFloat(std::string("##scale_").append(labels[i]).c_str(), &scale[i], 0.1f, 0.0f, 0.0f, formattedValue.c_str()))
			{
				updateTransform = true;
			}
			scaleChanged |= ImGui::IsItemEdited();
		}

		if (scaleChanged && constrainedProportions) {
			for (int i = 0; i < 3; ++i) {
				if (previousScales[i] != scale[i]) {
					float ratio = scale[i] / previousScales[i];
					for (int j = 0; j < 3; ++j) {
						if (j != i) {
							scale[j] = previousScales[j] * ratio;
						}
					}
					break;
				}
			}
		}

		ImGui::PopItemWidth();

		// Reset
		if (ImGui::Button("Reset"))
		{
			position = glm::float3(0.f);
			eulerRotation = glm::float3(0.f);
			scale = glm::float3(1.f);
			updateTransform = true;
		}
		resetInfoTag.ShowInfoTag("Reset Transform Values");
	}

	if (updateTransform) UpdateTransform();
}

void ComponentTransform::SetTransformMatrix(glm::float3 position, glm::quat rotation, glm::float3 scale, ComponentTransform* parent)
{
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;

	eulerRotation = degrees(glm::eulerAngles(rotation));

	localTransform = glm::float4x4(1.0f);
	localTransform = glm::translate(localTransform, position);
	localTransform *= glm::mat4_cast(rotation);
	localTransform = glm::scale(localTransform, scale);

	if (parent != nullptr)
	{
		globalTransform = parent->globalTransform * localTransform;
	}
}

void ComponentTransform::UpdateTransform()
{
	rotation = glm::quat(glm::vec3(glm::radians(eulerRotation.x), glm::radians(eulerRotation.y), glm::radians(eulerRotation.z)));

	localTransform = glm::float4x4(1.0f);
	localTransform = glm::translate(localTransform, position);
	localTransform *= glm::mat4_cast(rotation);
	localTransform = glm::scale(localTransform, scale);

	if (gameObject->parent != nullptr)
	{
		ComponentTransform* parentTransform = gameObject->parent->transform;
		globalTransform = parentTransform->globalTransform * localTransform;
	}
	else
	{
		globalTransform = localTransform;
	}

	for (auto child : gameObject->children)
	{
		child->transform->UpdateTransform();
	}

	updateTransform = false;
	app->scene->octreeNeedsUpdate = true;
}

bool ComponentTransform::Decompose(const glm::float4x4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale)
{
	glm::float4x4 localMatrix(transform);

	translation = glm::vec3(localMatrix[3]);

	glm::vec3 row[3];

	for (int i = 0; i < 3; i++)
	{
		row[i] = glm::vec3(localMatrix[i]);
	}

	scale.x = glm::length(row[0]);
	scale.y = glm::length(row[1]);
	scale.z = glm::length(row[2]);

	if (scale.x != 0) row[0] /= scale.x;
	if (scale.y != 0) row[1] /= scale.y;
	if (scale.z != 0) row[2] /= scale.z;

	glm::mat3 rotationMatrix(row[0], row[1], row[2]);

	rotation = glm::quat_cast(rotationMatrix);

	return true;
}

void ComponentTransform::SetButtonColor(const char* label)
{
	ImVec4 buttonColor;
	if (strcmp(label, "X") == 0)
	{
		buttonColor = ImVec4(0.8f, 0.4f, 0.4f, 0.7f); // Red
	}
	else if (strcmp(label, "Y") == 0)
	{
		buttonColor = ImVec4(0.4f, 0.8f, 0.4f, 0.7f); // Green
	}
	else if (strcmp(label, "Z") == 0)
	{
		buttonColor = ImVec4(0.4f, 0.4f, 0.8f, 0.7f); // Blue
	}

	ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonColor.x * 0.9f, buttonColor.y * 0.9f, buttonColor.z * 0.9f, 1.0f));

	ImGui::Button(label);

	ImGui::PopStyleColor(3);
}

void ComponentTransform::Serialize(nlohmann::json& json) const
{
	Component::Serialize(json);
	json["position"] = { position.x, position.y, position.z };
	json["rotation"] = { rotation.x, rotation.y, rotation.z, rotation.w };
	json["scale"] = { scale.x, scale.y, scale.z };
}

void ComponentTransform::Deserialize(const nlohmann::json& json)
{
	Component::Deserialize(json);
	position = { json["position"][0], json["position"][1], json["position"][2] };
	rotation = { json["rotation"][3], json["rotation"][0], json["rotation"][1], json["rotation"][2] };
	scale = { json["scale"][0], json["scale"][1], json["scale"][2] };

	SetTransformMatrix(position, rotation, scale, gameObject->parent->transform);
	UpdateTransform();
}