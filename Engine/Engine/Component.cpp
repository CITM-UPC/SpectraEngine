#include "Component.h"

Component::Component(GameObject* owner, ComponentType type) : active(true), gameObject(owner), type(type)
{
}

Component::~Component()
{
}

void Component::Update()
{
}

void Component::OnEditor()
{
}

void Component::Serialize(nlohmann::json& json) const
{
	json["type"] = type;
}

void Component::Deserialize(const nlohmann::json& json)
{
	type = json["type"];
}