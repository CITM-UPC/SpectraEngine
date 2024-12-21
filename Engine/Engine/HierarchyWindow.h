#pragma once

#include "EditorWindow.h"
#include "GameObject.h"

class HierarchyWindow : public EditorWindow
{
public:
	HierarchyWindow(const WindowType type, const std::string& name);
	~HierarchyWindow();

	void DrawWindow() override;

	void HierarchyTree(GameObject* node, bool isRoot = false, const char* searchText = "");
	bool FilterNode(GameObject* node, const char* searchText);
	void RemoveNodeFromParent(const GameObject* node) const;

private:
	char searchInput[256] = "";
	char inputName[256] = "GameObject";
	ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

	GameObject* selectedNode = nullptr;
	bool showNodePopup = false;
};