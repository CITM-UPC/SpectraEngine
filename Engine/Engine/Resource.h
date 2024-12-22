#pragma once

#include <string>

enum class ResourceType
{
	TEXTURE,
	MODEL,
	MESH,
	UNKNOWN = -1
};

class Resource
{
public:
	Resource(ResourceType type) : type(type) {}
	virtual ~Resource() {}

	ResourceType GetType() const { return type; }

	void SetAssetFileDir(const std::string& assetFileDir) { this->assetFileDir = assetFileDir; }
	void SetLibraryFileDir(const std::string& libraryFileDir) { this->libraryFileDir = libraryFileDir; }

	const std::string& GetAssetFileDir() const { return assetFileDir; }
	const std::string& GetLibraryFileDir() const { return libraryFileDir; }

private:
	ResourceType type;
	std::string assetFileDir;
	std::string libraryFileDir;
};