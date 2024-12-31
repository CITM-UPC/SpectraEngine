#pragma once

#include "EditorWindow.h"
#include "ModuleWindow.h"

class GameWindow : public EditorWindow
{
public:
	GameWindow(const WindowType type, const std::string& name);
	~GameWindow();

	void DrawWindow() override;
	void DrawTimeOverlay();

public:
	ImVec2 windowSize = ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT);

private:
	InfoTag playInfoTag;
	InfoTag pauseInfoTag;
	InfoTag stepInfoTag;

	bool showTimeOverlay = false;
};
