#include "GameWindow.h"

#include "App.h"

GameWindow::GameWindow(const WindowType type, const std::string& name) : EditorWindow(type, name)
{
}

GameWindow::~GameWindow()
{
}

void GameWindow::DrawWindow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar);

	UpdateMouseState();

	const ImVec2 newWindowSize = ImGui::GetContentRegionAvail();

	if (windowSize.x != newWindowSize.x || windowSize.y != newWindowSize.y)
	{
		windowSize = newWindowSize;
		app->renderer3D->updateFramebuffer = true;
	}

	ImGui::Image((void*)(intptr_t)app->renderer3D->fboGameTexture, windowSize, ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
	ImGui::PopStyleVar();
}