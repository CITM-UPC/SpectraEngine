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

    if (ImGui::BeginMenuBar())
    {
        ImVec4 selectedColor = ImVec4{ 0.24f, 0.28f, 0.34f, 1.0f };

        bool isPlaying = app->time.GetState() == GameState::PLAY || app->time.IsPlaying();
        if (isPlaying)
			ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);

        if (ImGui::ImageButton((ImTextureID)(uintptr_t)app->importer->icons.playIcon, ImVec2(12, 12)))
        	app->time.Play();

        if (isPlaying)
        	ImGui::PopStyleColor();

		playInfoTag.ShowInfoTag("Play");

        ImGui::SameLine();

        bool isPaused = app->time.GetState() == GameState::PAUSE;
        if (isPaused)
        	ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);

        if (ImGui::ImageButton((ImTextureID)(uintptr_t)app->importer->icons.pauseIcon, ImVec2(12, 12)))
        	app->time.Pause();
  
        if (isPaused)
        	ImGui::PopStyleColor();

		pauseInfoTag.ShowInfoTag("Pause");

        ImGui::SameLine();

        if (ImGui::ImageButton((ImTextureID)(uintptr_t)app->importer->icons.stepIcon, ImVec2(12, 12)))
        	app->time.Step();

		stepInfoTag.ShowInfoTag("Step");

        ImGui::EndMenuBar();
    }

	UpdateMouseState();

	const ImVec2 newWindowSize = ImGui::GetContentRegionAvail();

	if (windowSize.x != newWindowSize.x || windowSize.y != newWindowSize.y)
	{
		windowSize = newWindowSize;
		app->renderer3D->updateFramebuffer = true;
	}

	ImGui::Image((void*)(intptr_t)app->renderer3D->fboGameTexture, windowSize, ImVec2(0, 1), ImVec2(1, 0));

	DrawTimeOverlay();

	ImGui::End();
	ImGui::PopStyleVar();
}

void GameWindow::DrawTimeOverlay()
{
	ImVec2 windowPos = ImGui::GetWindowPos();

	ImVec2 overlayPos = ImVec2(windowPos.x + 10, windowPos.y + 50);

	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

	if (ImGui::Begin("Time Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("Frame Count: %.0f", app->time.GetFrameCount());
		ImGui::Text("Time Since Startup: %.2f s", app->time.GetTimeSinceStartup());
		ImGui::Text("Time Scale: %.2f", app->time.GetTimeScale());
		ImGui::Text("Delta Time: %.2f ms", app->time.GetDeltaTime());
		ImGui::Text("Real Time Since Startup: %.2f s", app->time.GetRealTimeSinceStartup());
		ImGui::Text("Real Delta Time: %.2f ms", app->time.GetRealDeltaTime());
		ImGui::End();
	}

	ImGui::PopStyleVar();
}