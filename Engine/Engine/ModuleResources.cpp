#include "ModuleResources.h"

#include <iostream>

#include "App.h"
#include "Model.h"

ModuleResources::ModuleResources(App* app) : Module(app)
{
}

ModuleResources::~ModuleResources()
{
}

bool ModuleResources::Awake()
{
	return true;
}

bool ModuleResources::CleanUp()
{
	return true;
}

Resource* ModuleResources::CreateResource(const std::string& fileDir, ResourceType type)
{
	Resource* resource = nullptr;

	std::string fileName = app->fileSystem->GetFileNameWithoutExtension(fileDir);

	switch (type)
	{
	case ResourceType::MODEL:
		resource = new Model();
		break;
	case ResourceType::MESH:
		resource = new Mesh();
		break;
	case ResourceType::TEXTURE:
		resource = new Texture(0, 0, 0, fileDir.c_str());
		break;
	}

	if (resource)
	{
		resource->SetAssetFileDir(fileDir.c_str());
		std::string libraryFileDir = CreateLibraryFileDir(fileName, type);
		resource->SetLibraryFileDir(libraryFileDir);
		resources.push_back(resource);
		resourceUsageCount[resource] = 0;
	}

	return resource;
}

ResourceType ModuleResources::GetResourceTypeFromExtension(const std::string& extension)
{
	if (extension == "fbx")
		return ResourceType::MODEL;
	else if (extension == "png" || extension == "dds" || extension == "tga")
		return ResourceType::TEXTURE;
	else
		return ResourceType::UNKNOWN;
}

std::string ModuleResources::CreateLibraryFileDir(std::string name, ResourceType type)
{
	switch (type)
	{
	case ResourceType::MODEL:
		return "Library/Models/" + name + ".model";
		break;
	case ResourceType::MESH:
		return "Library/Meshes/" + name + ".mesh";
		break;
	case ResourceType::TEXTURE:
		return "Library/Textures/" + name + ".dds";
		break;
	}

	return std::string();
}

Resource* ModuleResources::FindResourceInLibrary(const std::string& fileDir, ResourceType type)
{
	std::string fileName = app->fileSystem->GetFileNameWithoutExtension(fileDir);
	std::string libraryFileDir = CreateLibraryFileDir(fileName, type);

	Resource* foundResource = nullptr;
	for (const auto& resource : resources)
	{
		if (resource->GetLibraryFileDir() == libraryFileDir)
		{
			foundResource = resource;
			break;
		}
	}

	return foundResource;
}

int ModuleResources::GetResourceUsageCount(Resource* resource) const
{
	auto it = resourceUsageCount.find(resource);
	if (it != resourceUsageCount.end())
	{
		return it->second;
	}
	return 0;
}

void ModuleResources::ModifyResourceUsageCount(Resource* resource, int delta)
{
	auto it = resourceUsageCount.find(resource);
	if (it != resourceUsageCount.end())
	{
		it->second += delta;
		if (it->second < 0)
		{
			it->second = 0;
		}
	}

	//RemoveUnusedResource(resource);

	app->editor->resourcesWindow->UpdateResources();
}

void ModuleResources::RemoveUnusedResource(Resource* resource)
{
	if (GetResourceUsageCount(resource) == 0)
	{
		auto it = std::find(resources.begin(), resources.end(), resource);
		if (it != resources.end())
		{
			resources.erase(it);
		}

		delete resource;

		resourceUsageCount.erase(resource);
	}
}
