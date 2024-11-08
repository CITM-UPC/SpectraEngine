#include "ProjectWindow.h"
#include "App.h"

ProjectWindow::ProjectWindow(const WindowType type, const std::string& name) : EditorWindow(type, name), currentPath("Assets")
{
    UpdateDirectoryContent();
}

ProjectWindow::~ProjectWindow()
{
}

void ProjectWindow::DrawWindow()
{
    ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_MenuBar);

    DrawMenuBar();

	int columns = oneColumnSelected ? 1 : 2;
    ImGui::Columns(columns, "ProjectColumns");

	SetupInitialColumnWidth();

    ImGui::BeginChild("Folders", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None);
    DrawFoldersTree("Assets");
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
    for (const auto& entry : std::filesystem::directory_iterator(currentPath))
    {
        directoryContents.push_back(entry);
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

    bool open = ImGui::TreeNodeEx(("##" + directoryPath.string()).c_str(), nodeFlags);

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
    ImGui::Image(icon, ImVec2(smallIconSize, smallIconSize));

    ImGui::SameLine();
    ImGui::TextUnformatted(directoryPath.filename().string().c_str());

    if (open)
    {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
        {
            if (entry.is_directory())
            {
                DrawFoldersTree(entry.path());
            }
        }

        if (oneColumnSelected)
        {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
            {
                if (entry.is_regular_file())
                {
                    DrawFileItem(entry);
                }
            }
        }


        ImGui::TreePop();
    }
}

void ProjectWindow::DrawFileItem(const std::filesystem::directory_entry& entry)
{
    ImGui::Image((ImTextureID)(uintptr_t)app->importer->icons.fileIcon, ImVec2(smallIconSize, smallIconSize));
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
        int columns = static_cast<int>(ImGui::GetContentRegionAvail().x / columnWidth);

        if (columns > 1)
        {
            ImGui::Columns(columns, nullptr, false);

            for (int i = 0; i < columns; i++)
            {
                ImGui::SetColumnWidth(i, columnWidth);
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
    float cellHeight = columnHeight;

    std::string uniqueID = "##SelectableTile_" + entry.path().string();
    bool isSelected = (selectedPath == entry.path() && isItemSelected);

    ImGui::Selectable(uniqueID.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(cellWidth, cellHeight));

    HandleItemClick(entry, shouldBreakLoop);

    HandleDragDrop(entry);

	if (shouldBreakLoop) return;

    ImGui::SetCursorPosX((cellWidth * ImGui::GetColumnIndex()));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - cellHeight);

    ImGui::BeginGroup();

    ImGui::SetCursorPosX(((cellWidth - largeIconSize) * 0.5f) + (cellWidth * ImGui::GetColumnIndex()));
    ImGui::Image((ImTextureID)(uintptr_t)(entry.is_directory() ? app->importer->icons.folderIcon : app->importer->icons.fileIcon), ImVec2(largeIconSize, largeIconSize));

    std::string displayedName = GetTruncatedFilename(entry.path().filename().stem().string(), maxTextWidth);
    ImVec2 textSize = ImGui::CalcTextSize(displayedName.c_str());

    ImGui::SetCursorPosX(((cellWidth - textSize.x) * 0.5f) + (cellWidth * ImGui::GetColumnIndex()));
    ImGui::Text("%s", displayedName.c_str());
    ImGui::EndGroup();    
    
    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
    ImGui::NextColumn();
}

void ProjectWindow::DrawListItem(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop)
{
    ImGui::Image((ImTextureID)(uintptr_t)(entry.is_directory() ? app->importer->icons.folderIcon : app->importer->icons.fileIcon), ImVec2(mediumIconSize, mediumIconSize));
    ImGui::SameLine();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (mediumIconSize / 3.0f));

    bool isSelected = (selectedPath == entry.path() && isItemSelected);
    ImGui::Selectable(entry.path().filename().string().c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick);

    HandleItemClick(entry, shouldBreakLoop);

    HandleDragDrop(entry);
}

void ProjectWindow::DrawColumnItem(const std::filesystem::directory_entry& entry, bool& shouldBreakLoop)
{
    ImGui::Image((ImTextureID)(uintptr_t)(entry.is_directory() ? app->importer->icons.folderIcon : app->importer->icons.fileIcon), ImVec2(smallIconSize, smallIconSize));
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
        }
    }
}

void ProjectWindow::HandleDragDrop(const std::filesystem::directory_entry& entry)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        const std::string filePath = entry.path().string();

        ImTextureID iconTexture = entry.is_directory() ? (ImTextureID)(uintptr_t)app->importer->icons.folderIcon : (ImTextureID)(uintptr_t)app->importer->icons.fileIcon;

        ImGui::Image(iconTexture, ImVec2(mediumIconSize, mediumIconSize));

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (mediumIconSize / 3.0f));

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
                "All Files (*.fbx;*.png;*.dds)\0*.fbx;*.png;*.dds\0"
                "FBX Files (*.fbx)\0*.fbx\0"
                "PNG Files (*.png)\0*.png\0"
                "DDS Files (*.dds)\0*.dds\0"
                "\0";

            std::string selectedFile = app->importer->OpenFileDialog(filter);
            if (!selectedFile.empty())
            {
				app->importer->ImportFile(selectedFile);
				UpdateDirectoryContent();
            }
        }

        std::vector<std::string> pathParts = GetPathParts();

        for (size_t i = 0; i < pathParts.size(); ++i)
        {
            if (i == pathParts.size() - 1)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
            }

            if (ImGui::MenuItem(pathParts[i].c_str()))
            {
                std::filesystem::path newPath = std::filesystem::path("Assets");
                for (size_t j = 1; j <= i; ++j)
                {
                    newPath /= pathParts[j];
                }

                if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath))
                {
                    currentPath = newPath;
                    UpdateDirectoryContent();
                }
            }

            if (i == pathParts.size() - 1)
            {

                ImGui::PopStyleColor();
            }

            if (i < pathParts.size() - 1)
            {
                if (ImGui::MenuItem(">"))
                {
                    ImGui::OpenPopup("NextFoldersPopup");
                }

            }                
            
        }
        if (ImGui::BeginPopup("NextFoldersPopup"))
        {
            std::filesystem::path parentDir = currentPath.parent_path();
            for (const auto& entry : std::filesystem::directory_iterator(parentDir))
            {
                if (entry.is_directory())
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

        ImGui::SameLine(ImGui::GetWindowWidth() - 30);

        if (ImGui::ImageButton((ImTextureID)(uintptr_t)app->importer->icons.dotsIcon, ImVec2(12, 12)))
        {
            ImGui::OpenPopup("OptionsPopup");
        }

        if (ImGui::BeginPopup("OptionsPopup"))
        {   
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

                ImGui::Separator();
            }

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
            ImGui::Image((ImTextureID)(uintptr_t)app->importer->icons.folderIcon, ImVec2(smallIconSize, smallIconSize));
            ImGui::SameLine();
            ImGui::Text("%s", selectedPath.string().c_str());

            ImGui::EndMenuBar();
        }
    }
}
