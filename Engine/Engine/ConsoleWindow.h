#pragma once

#include "EditorWindow.h"
#include "Logger.h"

class ConsoleWindow : public EditorWindow
{
public:
	ConsoleWindow(const WindowType type, const std::string& name);
	~ConsoleWindow();

	void DrawWindow() override;

private:
	void DrawMenuBar();
	void DrawLogTypeCheckboxes();
	void DrawLogEntries();
	std::string GetSearchTerm() const;
	bool ShouldDisplayLog(const LogInfo& log, ImTextureID& logType, const std::string& searchTerm) const;

private:
	bool showLogInfo = true;
	bool showLogWarnings = true;
	bool showLogErrors = true;

	ImVec4 infoColor = ImVec4(.25f, .5f, 1, 1);
	ImVec4 warningColor = ImVec4(1, .5f, 0, 1);
	ImVec4 errorColor = ImVec4(1, 0, 0, 1);

	char searchBuffer[256] = { 0 };
};