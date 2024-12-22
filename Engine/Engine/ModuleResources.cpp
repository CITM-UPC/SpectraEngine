#include "ModuleResources.h"
#include "App.h"

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
		resource = new Resource(ResourceType::MODEL);
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

	for (const auto& resource : resources)
	{
		if (resource->GetLibraryFileDir() == libraryFileDir)
			return resource;
	}

	return nullptr;
}