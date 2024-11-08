#include "PreferencesWindow.h"
#include "App.h"

PreferencesWindow::PreferencesWindow(const WindowType type, const std::string& name) : EditorWindow(type, name)
{
}

PreferencesWindow::~PreferencesWindow()
{
}

void PreferencesWindow::DrawWindow()
{
    ImGui::Begin(name.c_str());

    if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Show Textures", &drawTextures);

        ImGui::Spacing();
        ImGui::Separator();

        static int w = 0;
        for (int n = 0; n < 3; n++)
        {
            const char* names[] = { "Shaded", "Wireframe", "Shaded Wireframe" };

            if (ImGui::Selectable(names[n], w == n))
            {
                w = n;
                if (n == 0)
                {
                    wireframe = false;
                    shadedWireframe = false;
                }
                else if (n == 1)
                {
                    wireframe = true;
                    shadedWireframe = false;
                }
                else if (n == 2)
                {
                    wireframe = false;
                    shadedWireframe = true;
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PushItemWidth(200.f);
        ImGui::SliderFloat("Vertex Normals Length", &vertexNormalLength, 0.05f, 0.25f, "%.2f", ImGuiSliderFlags_NoInput);
        ImGui::SliderFloat("Face Normals Length", &faceNormalLength, 0.05f, 0.25f, "%.2f", ImGuiSliderFlags_NoInput);
        ImGui::PopItemWidth();

        ImGui::ColorEdit3("Vertex Color", (float*)&vertexNormalColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::SameLine();
        ImGui::Text("Vertex Normal Color");

        ImGui::ColorEdit3("Face Color", (float*)&faceNormalColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::SameLine();
        ImGui::Text("Face Normal Color");
    }

    if (ImGui::CollapsingHeader("Grid", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit4("Grid Color", app->renderer3D->grid.lineColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

        ImGui::PushItemWidth(100.f);
        ImGui::SliderFloat("Cell Size", &app->renderer3D->grid.cellSize, 1.f, 10.f, "%1.f");
        ImGui::PopItemWidth();

        float gridSizeOptions[] = { 50.f, 100.f, 150.f, 200.f };
        float currentOption = (app->renderer3D->grid.gridSize / 50.f);

        ImGui::PushItemWidth(100.f);
        if (ImGui::SliderFloat("Grid Size", &currentOption, 1, 4, "%1.f")) {
            app->renderer3D->grid.gridSize = gridSizeOptions[(int)currentOption - 1];
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(100.f);
        ImGui::SliderFloat("Line Width", &app->renderer3D->grid.lineWidth, 1.f, 5.f, "%1.f");
        ImGui::PopItemWidth();

        static int selectedGrid = 1;
        for (int n = 0; n < 3; n++)
        {
            const char* names[] = { "X", "Y", "Z" };

            if (ImGui::Selectable(names[n], selectedGrid == n))
            {
                selectedGrid = n;
                if (n == 0)
                {
                    app->renderer3D->grid.normal = glm::vec3(1, 0, 0);
                }
                else if (n == 1)
                {
                    app->renderer3D->grid.normal = glm::vec3(0, 1, 0);
                }
                else if (n == 2)
                {
                    app->renderer3D->grid.normal = glm::vec3(0, 0, 1);
                }
            }
        }
    }

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
        ImGui::Text("Fov ");
        ImGui::SameLine();
		ImGui::SliderFloat("##Fov", &app->camera->fov, 4.0f, 120.0f);

        ImGui::Text("Near");
        ImGui::SameLine();
        ImGui::InputFloat("##Near", &app->camera->nearPlane);

        ImGui::Text("Far ");
        ImGui::SameLine();
        ImGui::InputFloat("##Far", &app->camera->farPlane);
	}

    ImGui::End();
}