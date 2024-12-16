#include "OctreeWindow.h"
#include "App.h"

#include <glm/gtc/type_ptr.hpp>

OctreeWindow::OctreeWindow(const WindowType type, const std::string& name) : EditorWindow(type, name)
{
}

OctreeWindow::~OctreeWindow()
{
}

void OctreeWindow::DrawWindow()
{
	ImGui::Begin(name.c_str());

	UpdateMouseState();

	if (ImGui::CollapsingHeader("Preferences", ImGuiTreeNodeFlags_DefaultOpen))
	{
		int maxDepth = app->scene->octreeMaxDepth;
		ImGui::InputInt("Max Depth", &app->scene->octreeMaxDepth);
		if (maxDepth != app->scene->octreeMaxDepth)
		{
			app->scene->octreeMaxDepth = std::clamp(app->scene->octreeMaxDepth, 1, 5);
			app->scene->sceneOctree->SetMaxDepth(app->scene->octreeMaxDepth);
			app->scene->octreeNeedsUpdate = true;
		}

		int maxObjects = app->scene->octreeMaxObjects;
		ImGui::InputInt("Max Objects", &maxObjects);
		if (maxObjects != app->scene->octreeMaxObjects)
		{
			app->scene->octreeMaxObjects = std::clamp(maxObjects, 1, 10);
			app->scene->sceneOctree->SetMaxObjects(app->scene->octreeMaxObjects);
			app->scene->octreeNeedsUpdate = true;
		}

		ImGui::ColorEdit3("Octree Color", glm::value_ptr(app->scene->octreeColor));

		ImGui::Checkbox("Draw Octree", &app->scene->drawOctree);
	}

	if (ImGui::CollapsingHeader("Views", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* views[] = { "Top", "Bottom", "Front", "Back", "Left", "Right" };

		for (int i = 0; i < 6; ++i)
		{
			if (i > 0) ImGui::SameLine();
			if (ImGui::Button(views[i])) currentView = i;
		}

		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetContentRegionAvail();

        ImVec2 padding(windowSize.x / baseWidth * basePadding, windowSize.y / baseHeight * basePadding);
        ImVec2 viewPos(windowPos.x + (padding.x / 2), windowPos.y + ImGui::GetCursorPosY() + (padding.y / 2));
        ImVec2 viewSize(windowSize.x - padding.x, windowSize.y - padding.y);

		app->scene->sceneOctree->DrawView(ImGui::GetWindowDrawList(), viewSize, viewPos, currentView);
	}

	ImGui::End();
}