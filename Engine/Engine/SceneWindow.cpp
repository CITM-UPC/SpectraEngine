#include "SceneWindow.h"
#include "App.h"

#include "glm/gtc/type_ptr.hpp"

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

		static int operation = 0;
        const char* operationNames[] = { "Translate", "Rotate", "Scale" };
		for (int i = 0; i < 3; i++)
		{
			if (ImGui::RadioButton(operationNames[i], operation == i))
			{
				operation = i;
				if (i == 0)
					gizmoOperation = ImGuizmo::TRANSLATE;
				else if (i == 1)
					gizmoOperation = ImGuizmo::ROTATE;
				else if (i == 2)
					gizmoOperation = ImGuizmo::SCALE;
			}
		}

        if (ImGui::Button(gizmoMode == ImGuizmo::LOCAL ? "Local" : "World"))
            gizmoMode = gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

        if (ImGui::BeginMenu("Shading Mode"))
        {
            static int w = 0;
            const char* names[] = { "Shaded", "Wireframe", "Shaded Wireframe" };
            for (int n = 0; n < 3; n++)
            {
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

        ImGui::EndMenuBar();
    }

	UpdateMouseState();

    ImVec2 textureSize = ImVec2(app->window->width, app->window->height);
    ImVec2 windowSize = ImGui::GetWindowSize();

    ImVec2 uv0 = ImVec2((textureSize.x - windowSize.x) / (2 * textureSize.x),
        (textureSize.y + windowSize.y) / (2 * textureSize.y));
    ImVec2 uv1 = ImVec2((textureSize.x + windowSize.x) / (2 * textureSize.x),
        (textureSize.y - windowSize.y) / (2 * textureSize.y));

    ImGui::Image((void*)(intptr_t)app->renderer3D->fboTexture, windowSize, uv0, uv1);

    if (ImGui::BeginDragDropTarget()) 
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_FILE_PATH"))
        {
            const char* droppedFilePath = static_cast<const char*>(payload->Data);
            app->importer->ImportFile(droppedFilePath, true);
        }
        ImGui::EndDragDropTarget();
    }

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    float windowWidth = ImGui::GetWindowWidth();
    float windowHeight = ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

    glm::mat4 viewMatrix = app->camera->GetViewMatrix();
    glm::mat4 projectionMatrix = app->camera->GetProjectionMatrix();

    if (app->editor->selectedGameObject)
    {
        ComponentTransform* transform = app->editor->selectedGameObject->transform;

        if (transform)
        {
            glm::mat4 transformMatrix = transform->localTransform;

            if (ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), gizmoOperation, gizmoMode, glm::value_ptr(transformMatrix)))
            {
                transform->SetTransformMatrix(transformMatrix);
                transform->UpdateTransform();
            }
        }
    }

    if (app->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
        gizmoOperation = ImGuizmo::TRANSLATE;
    else if (app->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
        gizmoOperation = ImGuizmo::ROTATE;
    else if (app->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
        gizmoOperation = ImGuizmo::SCALE;

    if (ImGui::IsWindowDocked() && app->editor->performanceWindow->showFpsOverlay)
    {
        if (windowSize.x > 120 && windowSize.y > 100)
        {        
            float dt = app->GetDT();
            float currentFps = 1.0f / dt;
            float ms = dt * 1000.0f;

            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 topRightPos = ImVec2(windowPos.x + windowSize.x - 80, windowPos.y + 60);
            ImVec4 overlayColor = ImVec4(0.0f, 1.0f, 0.8f, 1.0f);
            ImGui::SetNextWindowPos(topRightPos);
            ImGui::Begin("FPSOverlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(overlayColor, "%.2f FPS", currentFps);
            ImGui::TextColored(overlayColor, "%.2f ms", ms);
            ImGui::End();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
