#include "SceneWindow.h"
#include "App.h"

#include <limits>

SceneWindow::SceneWindow(const WindowType type, const std::string& name) : EditorWindow(type, name)
{
}

SceneWindow::~SceneWindow()
{
}

void SceneWindow::DrawWindow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
		if (ImGui::BeginMenu("Shading Mode"))
		{
			static int w = 0;
			for (int n = 0; n < 3; n++)
			{
				const char* names[] = { "Shaded", "Wireframe", "Shaded Wireframe" };
				if (ImGui::Selectable(names[n], w == n))
				{
					w = n;
					if (n == 0)
					{
						app->editor->preferencesWindow->wireframe = false;
						app->editor->preferencesWindow->shadedWireframe = false;
					}
					else if (n == 1)
					{
						app->editor->preferencesWindow->wireframe = true;
						app->editor->preferencesWindow->shadedWireframe = false;
					}
					else if (n == 2)
					{
						app->editor->preferencesWindow->wireframe = false;
						app->editor->preferencesWindow->shadedWireframe = true;
					}
				}
			}

			ImGui::EndMenu();
		}
		ImGui::PopStyleVar();

		if (ImGui::Selectable("Stats", showStatsOverlay, 0, ImVec2(30, 0)))
		{
			showStatsOverlay = !showStatsOverlay;
		}

		ImGui::EndMenuBar();
	}

	UpdateMouseState();

	const ImVec2 newWindowSize = ImGui::GetContentRegionAvail();

	if (windowSize.x != newWindowSize.x || windowSize.y != newWindowSize.y)
	{
		windowSize = newWindowSize;
		app->renderer3D->updateFramebuffer = true;
	}

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) 
		&& app->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_IDLE 
		&& app->input->GetKey(SDL_SCANCODE_LALT) == KEY_IDLE)
	{
		HandleMousePicking();
	}

	ImGui::Image((void*)(intptr_t)app->renderer3D->fboSceneTexture, windowSize, ImVec2(0, 1), ImVec2(1, 0));

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_FILE_PATH"))
		{
			const char* droppedFilePath = static_cast<const char*>(payload->Data);
			app->importer->ImportFile(droppedFilePath, true);
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::IsWindowDocked() && showStatsOverlay)
	{
		if (windowSize.x > 140 && windowSize.y > 140)
		{
			auto formatNumber = [](size_t count) -> std::string
			{
				if (count >= 1000000) return std::to_string(count / 1000000.0).substr(0, std::to_string(count / 1000000.0).find('.') + 2) + "M";
				if (count >= 1000) return std::to_string(count / 1000.0).substr(0, std::to_string(count / 1000.0).find('.') + 2) + "k";
				return std::to_string(count);
			};

			float dt = app->GetDT();
			float currentFps = 1.0f / dt;
			float ms = dt * 1000.0f;

			ImGui::SetNextWindowBgAlpha(0.75f);
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 topRightPos = ImVec2(windowPos.x + windowSize.x - 140, windowPos.y + 50);
			ImGui::SetNextWindowPos(topRightPos);
			ImGui::SetNextWindowSize(ImVec2(130, 125));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
			if (ImGui::Begin("SceneStatsOverlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
			{
				ImGui::Text("FPS: %.2f", currentFps);
				ImGui::Text("CPU: %.2f ms", ms);
				ImGui::Text("Tris: %s", formatNumber(app->scene->sceneCamera->triangleCount).c_str());
				ImGui::Text("Verts: %s", formatNumber(app->scene->sceneCamera->vertexCount).c_str());
				ImGui::Text("Meshes: %d", app->scene->sceneCamera->meshCount);
				ImGui::Text("Screen: %.fx%.f", windowSize.x, windowSize.y);
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void SceneWindow::HandleMousePicking() const
{
	ImVec2 mousePos = ImGui::GetMousePos();
	ImVec2 windowPos = ImGui::GetWindowPos();

	float mouseX = mousePos.x - windowPos.x;
	float mouseY = mousePos.y - windowPos.y - ImGui::GetCursorPosY();

	float normalizedX = (2.0f * mouseX) / windowSize.x - 1.0f;
	float normalizedY = 1.0f - (2.0f * mouseY) / windowSize.y;

	glm::vec4 rayClip = glm::vec4(normalizedX, normalizedY, -1.0f, 1.0f);
	glm::vec4 rayEye = glm::inverse(app->scene->sceneCamera->GetProjectionMatrix()) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
	glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(app->scene->sceneCamera->GetViewMatrix()) * rayEye));

	glm::vec3 rayOrigin = app->scene->sceneCamera->position;
	GameObject* selectedObject = nullptr;

	float closestDistance = (std::numeric_limits<size_t>::max)();

	std::vector<GameObject*> objects;
	app->scene->sceneOctree->CollectIntersectingObjects(rayOrigin, rayWorld, objects);

	std::unordered_set<GameObject*> reviewedObjects;

	for (GameObject* object : objects)
	{
		if (reviewedObjects.find(object) != reviewedObjects.end())
			continue;

		reviewedObjects.insert(object);

		float intersectionDistance;
		if (object->IntersectsRay(rayOrigin, rayWorld, intersectionDistance))
		{
			if (intersectionDistance < closestDistance)
			{
				closestDistance = intersectionDistance;
				selectedObject = object;
			}
		}
	}

	app->editor->selectedGameObject = selectedObject;
}