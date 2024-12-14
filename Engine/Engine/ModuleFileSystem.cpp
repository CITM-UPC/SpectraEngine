#include "ModuleFileSystem.h"
#include "App.h"

#include <fstream>

ModuleFileSystem::ModuleFileSystem(App* app) : Module(app)
{
	std::filesystem::create_directories("Library");
	std::filesystem::create_directories("Library/Textures");
	std::filesystem::create_directories("Library/Meshes");
	std::filesystem::create_directories("Library/Models");
}

ModuleFileSystem::~ModuleFileSystem()
{
}

bool ModuleFileSystem::Awake()
{
	return true;
}

bool ModuleFileSystem::CleanUp()
{
	return true;
}

std::string ModuleFileSystem::OpenFileDialog(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR sizeFile[260] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;

	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = sizeFile;
	ofn.nMaxFile = sizeof(sizeFile);
	ofn.lpstrTitle = "Open File";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return std::string(ofn.lpstrFile);
	}
	return "";
}

std::string ModuleFileSystem::CopyFileIfNotExists(const std::string& source)
{
	if (source.find("Engine/") != std::string::npos || source.find("Engine\\") != std::string::npos)
		return source;

	std::string extension = GetExtension(source);
	std::string assetsDir;

	if (extension == "fbx")
		assetsDir = "Assets/Models/";
	else if (extension == "png" || extension == "dds" || extension == "tga")
		assetsDir = "Assets/Textures/";

	std::string destination = assetsDir + GetNameFromPath(source);

	if (!std::filesystem::exists(destination))
	{
		std::ifstream src(source, std::ios::binary);
		std::ofstream dst(destination, std::ios::binary);
		dst << src.rdbuf();
		app->editor->projectWindow->UpdateDirectoryContent();
		return destination;
	}
	return source;
}

std::string ModuleFileSystem::GetExtension(const std::string& filePath)
{
	std::string extension = std::filesystem::path(filePath).extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if (!extension.empty() && extension[0] == '.')
		extension.erase(0, 1);

	return extension;
}

std::string ModuleFileSystem::GetNameFromPath(const std::string& filePath)
{
	return std::filesystem::path(filePath).filename().string();
}

std::string ModuleFileSystem::GetFileNameWithoutExtension(const std::string& filePath)
{
	return std::filesystem::path(filePath).stem().string();
}

bool ModuleFileSystem::FileExists(const std::string& filePath)
{
	return std::filesystem::exists(filePath);
}