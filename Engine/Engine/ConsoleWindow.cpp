#include "ConsoleWindow.h"
#include "App.h"

ConsoleWindow::ConsoleWindow(const WindowType type, const std::string& name) : EditorWindow(type, name)
{
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::DrawWindow()
{
	ImGui::Begin(name.c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

	DrawMenuBar();
	DrawLogEntries();

	ImGui::End();
}

void ConsoleWindow::DrawMenuBar()
{
	ImGui::BeginMenuBar();

	UpdateMouseState();

	ImGui::SetNextItemWidth(300.0f);
	ImGui::InputTextWithHint("##Search", "Search Log", searchBuffer, IM_ARRAYSIZE(searchBuffer));

	DrawLogTypeCheckboxes();

	if (ImGui::Button("Clear"))
		logger.Clear();

	infoTag.ShowInfoTag("Clear Console Log");

	ImGui::EndMenuBar();
}

void ConsoleWindow::DrawLogTypeCheckboxes()
{
	ImGui::PushStyleColor(ImGuiCol_CheckMark, infoColor);
	ImGui::Checkbox("##Info", &showLogInfo);
	ImGui::SameLine();
	ImGui::Image((ImTextureID)(uintptr_t)app->importer->icons.infoIcon, ImVec2(20, 20));
	ImGui::PopStyleColor();


	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_CheckMark, warningColor);
	ImGui::Checkbox("##Warning", &showLogWarnings);
	ImGui::SameLine();
	ImGui::Image((ImTextureID)(uintptr_t)app->importer->icons.warningIcon, ImVec2(20, 20));
	ImGui::PopStyleColor();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_CheckMark, errorColor);
	ImGui::Checkbox("##Error", &showLogErrors);
	ImGui::SameLine();
	ImGui::Image((ImTextureID)(uintptr_t)app->importer->icons.errorIcon, ImVec2(20, 20));
	ImGui::PopStyleColor();
}

void ConsoleWindow::DrawLogEntries()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
	ImTextureID logType = 0;
	std::string searchTerm = GetSearchTerm();

	for (const auto& log : logger.GetLogs())
	{
		if (ShouldDisplayLog(log, logType, searchTerm))
		{
			ImGui::Image(logType, ImVec2(16, 16));
			ImGui::SameLine();
			ImGui::Text(log.message.c_str());
		}
	}

	ImGui::PopStyleVar();
}

std::string ConsoleWindow::GetSearchTerm() const
{
	std::string searchTerm = searchBuffer;
	std::transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(), ::tolower);
	return searchTerm;
}

bool ConsoleWindow::ShouldDisplayLog(const LogInfo& log, ImTextureID& logType, const std::string& searchTerm)
{
	bool showLog = false;

	switch (log.type)
	{
	case LogType::LOG_INFO:
		logType = (ImTextureID)(uintptr_t)app->importer->icons.infoIcon;
		if (!showLogInfo) return false;
		showLog = true;
		break;

	case LogType::LOG_WARNING:
		logType = (ImTextureID)(uintptr_t)app->importer->icons.warningIcon;
		if (!showLogWarnings) return false;
		showLog = true;
		break;

	case LogType::LOG_ERROR:
		logType = (ImTextureID)(uintptr_t)app->importer->icons.errorIcon;
		if (!showLogErrors) return false;
		showLog = true;
		break;
	}

	if (showLog && !searchTerm.empty())
	{
		std::string logMessage = log.message;
		std::transform(logMessage.begin(), logMessage.end(), logMessage.begin(), ::tolower);
		return logMessage.find(searchTerm) != std::string::npos;
	}

	return showLog;
}