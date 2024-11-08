#include "ModuleImporter.h"
#include "App.h"
#include "Logger.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"
#include <Windows.h>

#include <filesystem>
#include <fstream>

ModuleImporter::ModuleImporter(App* app) : Module(app)
{

    ilInit();
    iluInit();
    ilutInit();
}

ModuleImporter::~ModuleImporter()
{
}

bool ModuleImporter::Awake()
{
    // Project
	icons.folderIcon = LoadTexture("Assets/Icons/folder.png");
	icons.openFolderIcon = LoadTexture("Assets/Icons/open_folder.png");
	icons.fileIcon = LoadTexture("Assets/Icons/file.png");
    icons.dotsIcon = LoadTexture("Assets/Icons/dots.png");

    // Console
	icons.infoIcon = LoadTexture("Assets/Icons/info.png");
	icons.warningIcon = LoadTexture("Assets/Icons/warning.png");
	icons.errorIcon = LoadTexture("Assets/Icons/error.png");

	return true;
}

bool ModuleImporter::CleanUp()
{
	glDeleteTextures(1, &icons.folderIcon);
	glDeleteTextures(1, &icons.openFolderIcon);
	glDeleteTextures(1, &icons.fileIcon);
	glDeleteTextures(1, &icons.dotsIcon);
	glDeleteTextures(1, &icons.infoIcon);
	glDeleteTextures(1, &icons.warningIcon);
	glDeleteTextures(1, &icons.errorIcon);

	return true;
}

GLuint ModuleImporter::LoadTexture(const std::string& filePath)
{
    ilClearColour(255, 255, 255, 255);

    ILuint imageID;
    ilGenImages(1, &imageID);
    ilBindImage(imageID);

    if (ilLoadImage(filePath.c_str()) == IL_FALSE)
    {
        ilDeleteImages(1, &imageID);
        return 0;
    }

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
        ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    ilDeleteImages(1, &imageID);

    return textureID;
}

std::string ModuleImporter::OpenFileDialog(const char* filter)
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

void ModuleImporter::ImportFile(const std::string& fileDir, bool addToScene)
{
    const std::string modelsDir = "Assets/Models/";
    const std::string texturesDir = "Assets/Textures/";

    std::string extension = fileDir.substr(fileDir.find(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    auto copyFileIfNotExists = [](const std::string& source, const std::string& destination) 
    {
        if (!std::filesystem::exists(destination)) 
        {
            std::ifstream src(source, std::ios::binary);
            std::ofstream dst(destination, std::ios::binary);
            dst << src.rdbuf();
        }
    };

    if (extension == "fbx") 
    {
        std::string modelFilePath = modelsDir + std::filesystem::path(fileDir).filename().string();
        copyFileIfNotExists(fileDir, modelFilePath);

        if (!addToScene)
			return;

        app->renderer3D->meshLoader.ImportFBX(fileDir.c_str(), app->scene->root);
    }
    else if (extension == "png" || extension == "dds")
    {
        std::string textureFilePath = texturesDir + std::filesystem::path(fileDir).filename().string();
        copyFileIfNotExists(fileDir, textureFilePath);

        if (! addToScene)
			return;

        Texture* newTexture = app->renderer3D->LoadTextureImage(textureFilePath.c_str());
        if (newTexture && app->editor->selectedGameObject) 
        {
            app->editor->selectedGameObject->material->AddTexture(newTexture);
        }
    }
    else 
    {
        LOG(LogType::LOG_WARNING, "File format not supported");
    }
}