#pragma once

#include "Module.h"
#include "Resource.h"

#include <string>
#include <vector>
#include <unordered_map>

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

	const std::vector<Resource*>& GetResources() const { return resources; }
	int GetResourceUsageCount(Resource* resource) const;
	void ModifyResourceUsageCount(Resource* resource, int delta);

private:
	std::vector<Resource*> resources;
	std::unordered_map<Resource*, int> resourceUsageCount;
};
