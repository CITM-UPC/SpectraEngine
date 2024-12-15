#pragma once

#include <string>

#include "imgui.h"

enum class WindowType
{
	HIERARCHY,
	INSPECTOR,
	CONSOLE,
	PROJECT,
	SCENE,
	PERFORMANCE,
	PREFERENCES,
	ABOUT,
	OCTREE
};

class EditorWindow
{
public:
	EditorWindow(const WindowType type, const std::string& name) : type(type), name(name) {}
	virtual ~EditorWindow() {}

	virtual void DrawWindow() = 0;
	WindowType GetType() const { return type; }
	std::string GetName() const { return name; }
	virtual bool IsEnabled() const { return enabled; }
	virtual void SetEnabled(bool enabled) { this->enabled = enabled; }
	virtual void UpdateMouseState() { isMouseInside = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) ? true : false; }
	bool IsMouseInside() const { return isMouseInside; }

protected:
	WindowType type;
	std::string name;
	bool enabled = true;
	bool isMouseInside = false;
};