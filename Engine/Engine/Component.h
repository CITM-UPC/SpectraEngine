#pragma once

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <nlohmann/json.hpp>

class GameObject;

enum class ComponentType
{
	NONE,
	TRANSFORM,
	MESH,
	MATERIAL,
	CAMERA,
	SCRIPT,
	AUDIO
};

class Component
{
public:
	Component(GameObject* owner, ComponentType type);
	virtual ~Component();

	virtual void Update();
	virtual void OnEditor();

	virtual void Serialize(nlohmann::json& json) const;
	virtual void Deserialize(const nlohmann::json& json);

public:
	bool active;
	GameObject* gameObject;
	ComponentType type;
};