#pragma once

#include <vector>

#include "EditorWindow.h"
#include "Resource.h"

enum class ResourceType;

class ResourcesWindow : public EditorWindow
{
public:
	ResourcesWindow(const WindowType type, const std::string& name);
	~ResourcesWindow();

	void DrawWindow() override;

	void UpdateResources();

private:
	void DrawResourceUsageTable();
	std::string ResourceTypeToString(const ResourceType type);

private:
	std::vector<Resource*> resources;
};

