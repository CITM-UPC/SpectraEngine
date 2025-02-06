#pragma once

#include "Component.h"
#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "InfoTag.h"

class ComponentTransform : public Component
{
public:
	ComponentTransform(GameObject* gameObject);
	virtual ~ComponentTransform();

	void Update() override;
	void OnEditor() override;

	void Serialize(nlohmann::json& json) const override;
	void Deserialize(const nlohmann::json& json) override;

	void SetTransformMatrix(glm::float3 position, glm::quat rotation, glm::float3 scale, ComponentTransform* parent);
	void UpdateTransform();

	bool Decompose(const glm::float4x4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale);

	void SetPosition(const glm::vec3& position);

private:
	void SetButtonColor(const char* label);

public:
	glm::float4x4 localTransform;
	glm::float4x4 globalTransform;

	glm::float3 position;
	glm::quat rotation;
	glm::float3 eulerRotation;
	glm::float3 scale;

	bool constrainedProportions = false;
	float initialScale[3] = { 1.0f, 1.0f, 1.0f };
	bool updateTransform = false;
	
private:
	InfoTag resetInfoTag;
	InfoTag scaleInfoTag;
};
