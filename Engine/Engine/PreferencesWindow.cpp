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

	UpdateMouseState();
	if (ImGui::CollapsingHeader("Screen options", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Checkbox("Full Screen", &app->window->fullscreen))
		{
			app->window->SetFullScreen(app->window->fullscreen);
		}
		if (ImGui::Checkbox("Full Desktop", &app->window->fulldesktop))
		{
			app->window->SetFullDesktop(app->window->fulldesktop);
		}
		if (ImGui::Checkbox("Borderless", &app->window->borderless))
		{
			app->window->SetBorderless(app->window->borderless);
		}
		if (ImGui::Checkbox("Resizable", &app->window->resizable))
		{
			app->window->SetResizable(app->window->resizable);
		}
	}

	if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Show Textures", &drawTextures);
		if (ImGui::Checkbox("Cull face", &cullFace))
			cullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

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
		if (ImGui::SliderFloat("##Fov", &app->camera->fov, 4.0f, 120.0f))
		{
			app->camera->frustumNeedsUpdate = true;
		}

		ImGui::Text("Near");
		ImGui::SameLine();
		ImGui::InputFloat("##Near", &app->camera->nearPlane);

		ImGui::Text("Far ");
		ImGui::SameLine();
		ImGui::InputFloat("##Far", &app->camera->farPlane);
	}

	ImGui::End();
}