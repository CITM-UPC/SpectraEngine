#pragma once

#include "Module.h"

#include <string>
#include <filesystem>

class ModuleFileSystem : public Module
{
public:
	ModuleFileSystem(App* app);
	virtual ~ModuleFileSystem();

	bool Awake();
	bool CleanUp();

	std::string OpenFileDialog(const char* filter, const char* titleText = "Open File");
	std::string SaveFileDialog(const char* filter, const char* titleText = "Save File");
	std::string CopyFileIfNotExists(const std::string& source);
	std::string GetExtension(const std::string& filePath);
	std::string GetNameFromPath(const std::string& filePath);
	std::string GetFileNameWithoutExtension(const std::string& filePath);

	bool FileExists(const std::string& filePath);
};