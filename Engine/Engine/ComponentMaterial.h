#pragma once

#include "Component.h"
#include "Texture.h"

class ComponentMaterial : public Component
{
public:
	ComponentMaterial(GameObject* gameObject);
	virtual ~ComponentMaterial();

	void Update() override;
	void OnEditor() override;

	void Serialize(nlohmann::json& json) const override;
	void Deserialize(const nlohmann::json& json) override;

	void AddTexture(Texture* texture);

public:
	Texture* materialTexture;
	GLuint textureId;

private:
	bool showCheckersTexture = false;
};