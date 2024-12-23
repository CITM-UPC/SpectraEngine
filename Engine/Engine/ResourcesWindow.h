#pragma once

#include "EditorWindow.h"

enum class ResourceType;

class ResourcesWindow : public EditorWindow
{
public:
	ResourcesWindow(const WindowType type, const std::string& name);
	~ResourcesWindow();

	void DrawWindow() override;

private:
	void DrawResourceUsageTable();
	std::string ResourceTypeToString(const ResourceType type);
};

