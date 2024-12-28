#pragma once

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

class GameObject;

enum class ComponentType
{
	NONE,
	TRANSFORM,
	MESH,
	MATERIAL,
	CAMERA
};

class Component
{
public:
	Component(GameObject* owner, ComponentType type);
	virtual ~Component();

	virtual void Update();
	virtual void OnEditor();

	void Enable();
	void Disable();

public:
	bool active;
	GameObject* gameObject;
	ComponentType type;
};