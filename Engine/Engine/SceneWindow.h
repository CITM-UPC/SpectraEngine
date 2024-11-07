#pragma once

#include "EditorWindow.h"

#include "ImGuizmo.h"

class SceneWindow : public EditorWindow
{
public:
	SceneWindow(const WindowType type, const std::string& name);
	~SceneWindow();

	void DrawWindow() override;

private:
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
};