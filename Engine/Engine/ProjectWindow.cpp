#include "ProjectWindow.h"
#include "App.h"

ProjectWindow::ProjectWindow(const WindowType type, const std::string& name) : EditorWindow(type, name), currentPath(".")
{
	UpdateDirectoryContent();
}

ProjectWindow::~ProjectWindow()
{
}

void ProjectWindow::DrawWindow()
{
	ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	DrawMenuBar();

	UpdateMouseState();

	int columns = oneColumnSelected ? 1 : 2;
	ImGui::Columns(columns, "ProjectColumns");

	SetupInitialColumnWidth();

	ImGui::BeginChild("Folders", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None);
	DrawFoldersTree(".");
	ImGui::EndChild();

	if (twoColumnsSelected)
	{
		ImGui::NextColumn();

		ImGui::BeginChild("Assets", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None);
		DrawDirectoryContents();
		ImGui::EndChild();
	}

	ImGui::BeginChild("SelectionBar", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_MenuBar);
	DrawSelectionBar();
	ImGui::EndChild();

	ImGui::Columns(1);

	ImGui::End();
}

void ProjectWindow::SetupInitialColumnWidth()
{
	static bool isFirstTime = true;
	if (isFirstTime)
	{
		ImGui::SetColumnWidth(0, 300);
		isFirstTime = false;
	}
}

void ProjectWindow::UpdateDirectoryContent()
{
	directoryContents.clear();

	bool isRootDir = (currentPath == ".");

	for (const auto& entry : std::filesystem::directory_iterator(currentPath))
	{
		if (!isRootDir || (entry.is_directory() && (entry.path().filename() == "Assets" || (entry.path().filename() == "Engine" && showEngineContent))))
		{
			directoryContents.push_back(entry);
		}
	}
}

std::vector<std::string> ProjectWindow::GetPathParts() const
{
	std::vector<std::string> parts;
	for (const auto& part : currentPath)
	{
		parts.push_back(part.string());
	}
	return parts;
}

void ProjectWindow::DrawFoldersTree(const std::filesystem::path& directoryPath)
{
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	bool hasSubfolders = false;
	bool hasFiles = false;

	for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
	{
		if (entry.is_directory())
		{
			hasSubfolders = true;
		}
		else if (entry.is_regular_file() && oneColumnSelected)
		{
			hasFiles = true;
		}
	}

	if (!hasSubfolders && !hasFiles)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	std::string filename = directoryPath.filename().string();
	std::string displayPath = (directoryPath == ".") ? "All" : filename;

	bool open = ImGui::TreeNodeEx(("##" + displayPath).c_str(), nodeFlags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		currentPath = directoryPath;
		UpdateDirectoryContent();
		isItemSelected = false;
	}

	if (ImGui::IsItemHovered() && !isItemSelected)
	{
		selectedPath = directoryPath;
		showPathBar = true;
	}

	ImTextureID icon = hasSubfolders && open ? (ImTextureID)(uintptr_t)app->importer->icons.openFolderIcon : (ImTextureID)(uintptr_t)app->importer->icons.folderIcon;

	ImGui::SameLine();
	ImGui::Image(icon, ImVec2(iconSizes.smallIcon, iconSizes.smallIcon));

	ImGui::SameLine();
	ImGui::TextUnformatted(displayPath.c_str());

	if (open)
	{
		bool isRootDir = (directoryPath == ".");
		auto entries = std::filesystem::directory_iterator(directoryPath);

		for (const auto& entry : entries)
		{
			bool isValid = (!isRootDir || (entry.path().filename() == "Assets" || (entry.path().filename() == "Engine" && showEngineContent)));

			if (entry.is_directory() && isValid)
				DrawFoldersTree(entry.path());
			else if (oneColumnSelected && entry.is_regular_file() && isValid)
				DrawFileItem(entry);
		}

		ImGui::TreePop();
	}
}

void ProjectWindow::DrawFileItem(const std::filesystem::directory_entry& entry)
{
	ImGui::Image((ImTextureID)(uintptr_t)GetFileIcon(entry.path()), ImVec2(iconSizes.smallIcon, iconSizes.smallIcon));
	ImGui::SameLine();

	bool isSelected = (selectedPath == entry.path() && isItemSelected);
	ImGui::Selectable(entry.path().filename().string().c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick);

	bool shouldBreakLoop = false;

	HandleItemClick(entry, shouldBreakLoop);
	HandleDragDrop(entry);
}

void ProjectWindow::DrawDirectoryContents()
{
	ConfigureColumns();

	bool shouldBreakLoop = false;

	for (const auto& entry : directoryContents)
	{
		bool isDirectory = entry.is_directory();
		bool isFile = entry.is_regular_file();
		if (!isDirectory && !isFile) continue;

		if (tilesSelected)
			DrawTile(entry, shouldBreakLoop);
		else if (listSelected)
			DrawListItem(entry, shouldBreakLoop);
		else if (columnSelected)
			DrawColumnItem(entry, shouldBreakLoop);

		if (shouldBreakLoop)
			break;
	}

	ImGui::Columns(1);

	if ((!isItemSelected || (isItemSelected && ImGui::IsMouseClicked(0))) && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
	{
		isItemSelected = false;
		showPathBar = false;
	}
}

void ProjectWindow::ConfigureColumns()
{
	if (tilesSelected)
	{
		int columns = static_cast<int>(ImGui::GetContentRegionAvail().x / columnSettings.width);

		if (columns > 1)
		{
			ImGui::Columns(columns, nullptr, false);

			for (int i = 0; i < columns; i++)
			{
				ImGui::SetColumnWidth(i, columnSettings.width);
			}
		}
	}
	else if (columnSelected)
	{
		ImGui::Columns(2, nullptr, true);
	}
}

void ProjectWindow::DrawTile(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop)
{
	float cellWidth = ImGui::GetColumnWidth();
	float cellHeight = columnSettings.height;

	std::string uniqueID = "##SelectableTile_" + entry.path().string();
	bool isSelected = (selectedPath == entry.path() && isItemSelected);

	ImGui::Selectable(uniqueID.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(cellWidth, cellHeight));

	HandleItemClick(entry, shouldBreakLoop);

	HandleDragDrop(entry);

	if (shouldBreakLoop) return;

	ImGui::SetCursorPosX((cellWidth * ImGui::GetColumnIndex()));
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - cellHeight);

	ImGui::BeginGroup();

	ImGui::SetCursorPosX(((cellWidth - iconSizes.largeIcon) * 0.5f) + (cellWidth * ImGui::GetColumnIndex()));
	ImGui::Image((ImTextureID)(uintptr_t)(entry.is_directory() ? app->importer->icons.folderIcon : GetFileIcon(entry.path())), ImVec2(iconSizes.largeIcon, iconSizes.largeIcon));

	std::string displayedName = GetTruncatedFilename(entry.path().filename().stem().string(), columnSettings.maxTextWidth);
	ImVec2 textSize = ImGui::CalcTextSize(displayedName.c_str());

	ImGui::SetCursorPosX(((cellWidth - textSize.x) * 0.5f) + (cellWidth * ImGui::GetColumnIndex()));
	ImGui::Text("%s", displayedName.c_str());
	ImGui::EndGroup();

	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::NextColumn();
}

void ProjectWindow::DrawListItem(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop)
{
	ImGui::Image((ImTextureID)(uintptr_t)(entry.is_directory() ? app->importer->icons.folderIcon : GetFileIcon(entry.path())), ImVec2(iconSizes.mediumIcon, iconSizes.mediumIcon));
	ImGui::SameLine();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (iconSizes.mediumIcon / 3.0f));

	bool isSelected = (selectedPath == entry.path() && isItemSelected);
	ImGui::Selectable(entry.path().filename().string().c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick);

	HandleItemClick(entry, shouldBreakLoop);

	HandleDragDrop(entry);
}

void ProjectWindow::DrawColumnItem(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop)
{
	ImGui::Image((ImTextureID)(uintptr_t)(entry.is_directory() ? app->importer->icons.folderIcon : GetFileIcon(entry.path())), ImVec2(iconSizes.smallIcon, iconSizes.smallIcon));
	ImGui::SameLine();

	bool isSelected = (selectedPath == entry.path() && isItemSelected);
	ImGui::Selectable(entry.path().filename().string().c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick);

	HandleItemClick(entry, shouldBreakLoop);

	HandleDragDrop(entry);

	ImGui::NextColumn();
	ImGui::Text(entry.is_directory() ? "Folder" : "File");
	ImGui::NextColumn();
}

std::string ProjectWindow::GetTruncatedFilename(const std::string& filename, float maxTextWidth)
{
	std::string displayedName = filename;
	ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());

	if (textSize.x > maxTextWidth)
	{
		std::string ellipsis = "...";
		ImVec2 ellipsisSize = ImGui::CalcTextSize(ellipsis.c_str());
		size_t maxLength = filename.length();

		while (textSize.x > (maxTextWidth - ellipsisSize.x) && maxLength > 0)
		{
			displayedName = filename.substr(0, maxLength - 1);
			textSize = ImGui::CalcTextSize(displayedName.c_str());
			maxLength--;
		}

		if (displayedName != filename)
			displayedName += ellipsis;
	}
	return displayedName;
}

void ProjectWindow::HandleItemClick(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop)
{
	if (ImGui::IsItemHovered())
	{
		if (!isItemSelected)
		{
			selectedPath = entry.path();
			showPathBar = true;
		}

		if (ImGui::IsItemClicked())
		{
			selectedPath = entry.path();
			showPathBar = true;
			isItemSelected = true;
		}

		if (entry.is_directory() && ImGui::IsMouseDoubleClicked(0))
		{
			currentPath = entry.path();
			UpdateDirectoryContent();
			isItemSelected = false;
			shouldBreakLoop = true;
			return;
		}
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
	{
		ImGui::OpenPopup(("ItemContextMenu" + entry.path().string()).c_str());
	}

	if (ImGui::BeginPopup(("ItemContextMenu" + entry.path().string()).c_str()))
	{
		if (ImGui::MenuItem("Open"))
		{
			if (entry.is_directory())
			{
				currentPath = entry.path();
				UpdateDirectoryContent();
				isItemSelected = false;
				shouldBreakLoop = true;
			}
			else if (entry.is_regular_file())
			{
				std::string command = "explorer \"" + entry.path().string() + "\"";
				system(command.c_str());
			}
		}
		if (ImGui::MenuItem("Show in Explorer"))
		{
			std::string command = "explorer /select,\"" + entry.path().string() + "\"";
			system(command.c_str());
		}
		if (ImGui::MenuItem("Copy Path"))
		{
			char fullPath[MAX_PATH];
			GetFullPathName(entry.path().string().c_str(), MAX_PATH, fullPath, nullptr);
			ImGui::SetClipboardText(fullPath);
		}
		ImGui::EndPopup();
	}
}

void ProjectWindow::HandleDragDrop(const std::filesystem::directory_entry& entry)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		std::string filePath = entry.path().string();

		if (filePath.substr(0, 2) == ".\\")
		{
			filePath = filePath.substr(2);
		}

		ImTextureID iconTexture = (ImTextureID)(uintptr_t)(entry.is_directory()
			? app->importer->icons.folderIcon
			: GetFileIcon(entry.path()));

		ImGui::Image(iconTexture, ImVec2(iconSizes.mediumIcon, iconSizes.mediumIcon));

		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (iconSizes.mediumIcon / 3.0f));

		ImGui::Text("Dragging %s", entry.path().filename().string().c_str());

		ImGui::SetDragDropPayload("ASSET_FILE_PATH", filePath.c_str(), filePath.size() + 1);

		ImGui::EndDragDropSource();
	}
}

void ProjectWindow::DrawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::Button("Import"))
		{
			const char* filter =
				"All Files (*.fbx;*.png;*.dds;*.tga)\0*.fbx;*.png;*.dds;*.tga\0"
				"FBX Files (*.fbx)\0*.fbx\0"
				"PNG Files (*.png)\0*.png\0"
				"DDS Files (*.dds)\0*.dds\0"
				"TGA Files (*.tga)\0*.tga\0"
				"\0";

			std::string selectedFile = app->fileSystem->OpenFileDialog(filter);
			if (!selectedFile.empty())
			{
				app->importer->ImportFile(selectedFile);
				UpdateDirectoryContent();
			}
		}

		infoTag.ShowInfoTag("Import Assets");

		std::vector<std::string> pathParts = GetPathParts();
		std::vector<std::filesystem::path> fullPaths;

		for (auto& part : pathParts) {
			if (part == ".") {
				part = "All";
			}
		}

		for (size_t i = 0; i < pathParts.size(); ++i)
		{
			std::filesystem::path newPath = std::filesystem::path(".");
			for (size_t j = 1; j <= i; ++j)
			{
				newPath /= pathParts[j];
			}
			fullPaths.push_back(newPath);

			if (i == pathParts.size() - 1)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, app->editor->dataTextColor);
			}

			if (ImGui::MenuItem(pathParts[i].c_str()))
			{
				if (std::filesystem::exists(fullPaths[i]) && std::filesystem::is_directory(fullPaths[i]))
				{
					currentPath = fullPaths[i];
					UpdateDirectoryContent();
				}
			}

			if (i == pathParts.size() - 1)
			{
				ImGui::PopStyleColor();
			}

			if (i < pathParts.size() - 1)
			{
				std::string popupName = "NextFoldersPopup_" + std::to_string(i);
				if (ImGui::MenuItem(">"))
				{
					ImGui::OpenPopup(popupName.c_str());
				}

				if (ImGui::BeginPopup(popupName.c_str()))
				{
					std::filesystem::path parentDir = fullPaths[i];
					for (const auto& entry : std::filesystem::directory_iterator(parentDir))
					{
						bool isAssetsOrEngine = (entry.is_directory() && parentDir == "."
							&& (entry.path().filename() == "Assets" || (entry.path().filename() == "Engine" && showEngineContent)));

						if (entry.is_directory() && (parentDir != "." || isAssetsOrEngine))
						{
							if (ImGui::MenuItem(entry.path().filename().string().c_str()))
							{
								currentPath = entry.path();
								UpdateDirectoryContent();
								ImGui::CloseCurrentPopup();
							}
						}
					}
					ImGui::EndPopup();
				}
			}
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 30);

		if (ImGui::ImageButton((ImTextureID)(uintptr_t)app->importer->icons.dotsIcon, ImVec2(12, 12)))
		{
			ImGui::OpenPopup("OptionsPopup");
		}

		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("DOT TEXT");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		if (ImGui::BeginPopup("OptionsPopup"))
		{
			ImGui::SeparatorText("View Type");

			if (twoColumnsSelected)
			{
				if (ImGui::MenuItem("Tiles", nullptr, tilesSelected))
				{
					listSelected = false;
					tilesSelected = true;
					columnSelected = false;
				}
				if (ImGui::MenuItem("List", nullptr, listSelected))
				{
					listSelected = true;
					tilesSelected = false;
					columnSelected = false;
				}
				if (ImGui::MenuItem("Column", nullptr, columnSelected))
				{
					listSelected = false;
					tilesSelected = false;
					columnSelected = true;
				}
			}

			ImGui::SeparatorText("Columns");

			if (ImGui::MenuItem("One Column", nullptr, oneColumnSelected))
			{
				oneColumnSelected = true;
				twoColumnsSelected = false;
			}
			if (ImGui::MenuItem("Two Columns", nullptr, twoColumnsSelected))
			{
				oneColumnSelected = false;
				twoColumnsSelected = true;
			}

			ImGui::SeparatorText("Content");

			if (ImGui::MenuItem("Show Engine Content", nullptr, showEngineContent))
			{
				showEngineContent = !showEngineContent;
				UpdateDirectoryContent();
			}

			ImGui::EndPopup();
		}

		ImGui::EndMenuBar();
	}
}

void ProjectWindow::DrawSelectionBar()
{
	if (showPathBar && !selectedPath.empty())
	{
		if (ImGui::BeginMenuBar())
		{
			ImTextureID icon = (ImTextureID)(uintptr_t)(std::filesystem::is_directory(selectedPath)
				? app->importer->icons.folderIcon
				: GetFileIcon(selectedPath));

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (iconSizes.smallIcon / 8.f));
			ImGui::Image(icon, ImVec2(iconSizes.smallIcon, iconSizes.smallIcon));

			ImGui::SameLine();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (iconSizes.smallIcon / 8.f));
			ImGui::TextUnformatted(selectedPath.string().c_str());

			ImGui::EndMenuBar();
		}
	}
}

GLuint ProjectWindow::GetFileIcon(const std::filesystem::path& entry) const
{
	std::string extension = app->fileSystem->GetExtension(entry.string());

	if (extension == "png")
		return app->importer->icons.pngFileIcon;

	if (extension == "tga")
		return app->importer->icons.tgaFileIcon;

	if (extension == "dds")
		return app->importer->icons.ddsFileIcon;

	if (extension == "fbx")
		return app->importer->icons.fbxFileIcon;

	return app->importer->icons.fileIcon;
}