#pragma once

#include "ModelImporter.h"
#include "Module.h"
#include "Resource.h"
#include "Texture.h"
#include "TextureImporter.h"

#include <GL/glew.h>
#include <string>

struct Icons
{
	GLuint folderIcon = 0;
	GLuint openFolderIcon = 0;
	GLuint fileIcon = 0;
	GLuint pngFileIcon = 0;
	GLuint tgaFileIcon = 0;
	GLuint ddsFileIcon = 0;
	GLuint fbxFileIcon = 0;
	GLuint sceneFileIcon = 0;
	GLuint dotsIcon = 0;
	GLuint infoIcon = 0;
	GLuint warningIcon = 0;
	GLuint errorIcon = 0;
	GLuint playIcon = 0;
	GLuint pauseIcon = 0;
	GLuint stepIcon = 0;
};

class ModuleImporter : public Module
{
public:
	ModuleImporter(App* app);
	virtual ~ModuleImporter();

	bool Awake();
	bool CleanUp();

	void TryImportFile();
	void ImportFile(const std::string& fileDir, bool addToScene = false);
	void SetDraggedFile(const std::string& filePath);

	Resource* ImportFileToLibrary(const std::string& fileDir, ResourceType type);
	void LoadToScene(Resource* newResource, ResourceType resourceType);

public:
	Icons icons;
	bool isDraggingFile = false;

	TextureImporter* textureImporter;
	ModelImporter* modelImporter;

private:
	std::string draggedFile;
};