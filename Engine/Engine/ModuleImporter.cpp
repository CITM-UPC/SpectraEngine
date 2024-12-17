#include "ModuleImporter.h"
#include "App.h"
#include "Logger.h"

#include <Windows.h>

#include <filesystem>
#include <fstream>

ModuleImporter::ModuleImporter(App* app) : Module(app)
{
	textureImporter = new TextureImporter();
	modelImporter = new ModelImporter();
}

ModuleImporter::~ModuleImporter()
{
}

bool ModuleImporter::Awake()
{
	// Project
	icons.folderIcon = textureImporter->LoadIconImage("Engine/Icons/folder.png");
	icons.openFolderIcon = textureImporter->LoadIconImage("Engine/Icons/open_folder.png");
	icons.fileIcon = textureImporter->LoadIconImage("Engine/Icons/file.png");
	icons.pngFileIcon = textureImporter->LoadIconImage("Engine/Icons/file_png.png");
	icons.ddsFileIcon = textureImporter->LoadIconImage("Engine/Icons/file_dds.png");
	icons.fbxFileIcon = textureImporter->LoadIconImage("Engine/Icons/file_fbx.png");
	icons.dotsIcon = textureImporter->LoadIconImage("Engine/Icons/dots.png");

	// Console
	icons.infoIcon = textureImporter->LoadIconImage("Engine/Icons/info.png");
	icons.warningIcon = textureImporter->LoadIconImage("Engine/Icons/warning.png");
	icons.errorIcon = textureImporter->LoadIconImage("Engine/Icons/error.png");

	return true;
}

bool ModuleImporter::CleanUp()
{
	glDeleteTextures(1, &icons.folderIcon);
	glDeleteTextures(1, &icons.openFolderIcon);
	glDeleteTextures(1, &icons.fileIcon);
	glDeleteTextures(1, &icons.pngFileIcon);
	glDeleteTextures(1, &icons.ddsFileIcon);
	glDeleteTextures(1, &icons.fbxFileIcon);
	glDeleteTextures(1, &icons.dotsIcon);
	glDeleteTextures(1, &icons.infoIcon);
	glDeleteTextures(1, &icons.warningIcon);
	glDeleteTextures(1, &icons.errorIcon);

	return true;
}

void ModuleImporter::TryImportFile()
{
	if (!draggedFile.empty())
	{
		if (app->editor->sceneWindow->IsMouseInside() || app->editor->hierarchyWindow->IsMouseInside())
			app->importer->ImportFile(draggedFile, true);
		else if (app->editor->projectWindow->IsMouseInside())
			app->importer->ImportFile(draggedFile, false);

		draggedFile.clear();
	}
	isDraggingFile = false;
}

void ModuleImporter::ImportFile(const std::string& fileDir, bool addToScene)
{
	std::string extension = app->fileSystem->GetExtension(fileDir);

	bool isValidFile = extension == "fbx" || extension == "png" || extension == "dds" || extension == "tga";

	if (!isValidFile)
	{
		LOG(LogType::LOG_WARNING, "File format not supported");
		return;
	}

	std::string newDir = app->fileSystem->CopyFileIfNotExists(fileDir);

	ResourceType resourceType = app->resources->GetResourceTypeFromExtension(extension);

	Resource* newResource = app->resources->FindResourceInLibrary(newDir, resourceType);

	if (!newResource)
		newResource = ImportFileToLibrary(newDir, resourceType);

	if (addToScene)
		LoadToScene(newResource, resourceType);
}

void ModuleImporter::LoadToScene(Resource* newResource, ResourceType resourceType)
{
	switch (resourceType)
	{
	case ResourceType::MODEL:
		modelImporter->LoadModel(newResource, app->scene->root);
		break;
	case ResourceType::TEXTURE:
		Texture* newTexture = textureImporter->LoadTextureImage(newResource);
		if (newTexture && app->editor->selectedGameObject)
		{
			app->editor->selectedGameObject->material->AddTexture(newTexture);
		}
		break;
	}
}

void ModuleImporter::SetDraggedFile(const std::string& filePath)
{
	draggedFile = filePath;
	isDraggingFile = true;
}

Resource* ModuleImporter::ImportFileToLibrary(const std::string& fileDir, ResourceType type)
{
	Resource* resource = app->resources->CreateResource(fileDir, type);

	switch (type)
	{
	case ResourceType::MODEL:
		modelImporter->SaveModel(resource);
		break;
	case ResourceType::TEXTURE:
		textureImporter->SaveTextureFile(resource);
		break;
	}

	return resource;
}