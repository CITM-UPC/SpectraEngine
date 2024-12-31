#include "InfoTag.h"
#include <imgui.h>

InfoTag::InfoTag() : timer(), hoverStarted(false) {}

void InfoTag::ShowInfoTag(const char* text, const float time, const float wrapPos)
{
	if (ImGui::IsItemHovered())
	{
		if (!hoverStarted) {
			hoverStarted = true;
			timer.Start();
		}
		if (timer.ReadMs() >= time)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
			if (ImGui::BeginItemTooltip())
			{
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * wrapPos);
				ImGui::TextUnformatted(text);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();
		}
	}
	else
	{
			hoverStarted = false;
	}
	
}
