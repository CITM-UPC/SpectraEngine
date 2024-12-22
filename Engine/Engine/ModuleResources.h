#pragma once

#include "Module.h"
#include "Resource.h"

#include <string>
#include <vector>

class ModuleResources : public Module
{
public:
	ModuleResources(App* app);
	virtual ~ModuleResources();

	bool Awake();
	bool CleanUp();

	Resource* CreateResource(const std::string& fileDir, ResourceType type);

	ResourceType GetResourceTypeFromExtension(const std::string& extension);

	std::string CreateLibraryFileDir(std::string name, ResourceType type);

	Resource* FindResourceInLibrary(const std::string& fileDir, ResourceType type);

private:
	std::vector<Resource*> resources;
};
