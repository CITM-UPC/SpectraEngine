#pragma once

#include "Module.h"
#include "GameObject.h"
#include "EditorWindow.h"

#include "ConsoleWindow.h"
#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "ProjectWindow.h"
#include "SceneWindow.h"
#include "GameWindow.h"
#include "PerformanceWindow.h"
#include "PreferencesWindow.h"
#include "AboutWindow.h"
#include "OctreeWindow.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <list>

#include "ResourcesWindow.h"

class ModuleEditor : public Module
{
public:
	ModuleEditor(App* app);
	~ModuleEditor();

	bool Awake();
	bool CleanUp();

	void DrawEditor();
	void Docking();
	void MainMenuBar();
	void ApplyStyle();

public:
	GameObject* selectedGameObject = nullptr;

	ConsoleWindow* consoleWindow = nullptr;
	HierarchyWindow* hierarchyWindow = nullptr;
	InspectorWindow* inspectorWindow = nullptr;
	ProjectWindow* projectWindow = nullptr;
	SceneWindow* sceneWindow = nullptr;
	GameWindow* gameWindow = nullptr;
	PerformanceWindow* performanceWindow = nullptr;
	PreferencesWindow* preferencesWindow = nullptr;
	AboutWindow* aboutWindow = nullptr;
	OctreeWindow* octreeWindow = nullptr;
	ResourcesWindow* resourcesWindow = nullptr;

	ImVec4 dataTextColor = ImVec4(0.45f, 0.55f, 0.75f, 1.0f);

private:

	std::list<EditorWindow*> editorWindows;
};
