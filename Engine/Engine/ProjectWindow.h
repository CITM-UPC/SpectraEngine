#pragma once

#include "EditorWindow.h"

#include <filesystem>
#include <vector>
#include <string>

#include <GL/glew.h>

struct IconSizes
{
	float smallIcon = 16.0f;
	float mediumIcon = 32.0f;
	float largeIcon = 64.0f;
};

struct ColumnSettings
{
	float width = 100.0f;
	float height = 95.0f;
	float maxTextWidth = 80.0f;
};

class ProjectWindow : public EditorWindow
{
public:
	ProjectWindow(const WindowType type, const std::string& name);
	~ProjectWindow();

	void DrawWindow() override;

	void UpdateDirectoryContent();

	std::vector<std::string> GetPathParts() const;

private:
	GLuint GetFileIcon(const std::filesystem::path& entry) const;
	void DrawFoldersTree(const std::filesystem::path& directoryPath);
	void DrawFileItem(const std::filesystem::directory_entry& entry);

	void DrawDirectoryContents();
	void DrawMenuBar();
	void DrawSelectionBar();

	void SetupInitialColumnWidth();
	void ConfigureColumns() const;

	void DrawTile(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop);
	void DrawListItem(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop);
	void DrawColumnItem(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop);

	std::string GetTruncatedFilename(const std::string& filename, float maxTextWidth);

	void HandleItemClick(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop);
	void HandleDragDrop(const std::filesystem::directory_entry& entry);

private:
	std::filesystem::path currentPath;
	std::filesystem::path selectedPath;
	std::vector<std::filesystem::directory_entry> directoryContents;

	bool showPathBar = false;

	bool isItemSelected = false;

	bool listSelected = false;
	bool tilesSelected = true;
	bool columnSelected = false;
	bool oneColumnSelected = false;
	bool twoColumnsSelected = true;
	bool showEngineContent = false;

	IconSizes iconSizes;
	ColumnSettings columnSettings;
};