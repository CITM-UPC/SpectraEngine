#pragma once

#include "EditorWindow.h"

class OctreeWindow : public EditorWindow
{
public:
	OctreeWindow(const WindowType type, const std::string& name);
	~OctreeWindow();

	void DrawWindow() override;

private:
	int currentView = 0;

	float baseWidth = 500.0f;
	float baseHeight = 500.0f;
	float basePadding = 200.0f;
};
