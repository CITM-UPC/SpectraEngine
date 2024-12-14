#include "ModuleEditor.h"
#include "App.h"

#include "imgui_internal.h"

ModuleEditor::ModuleEditor(App* app) : Module(app)
{}

ModuleEditor::~ModuleEditor()
{
	for (auto editorWindow : editorWindows)
	{
		delete editorWindow;
	}
	editorWindows.clear();
}

bool ModuleEditor::Awake()
{
	LOG(LogType::LOG_INFO, "ModuleEditor");
	bool ret = true;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	ImFont* customFont = io.Fonts->AddFontFromFileTTF("Engine/Fonts/Roboto-Regular.ttf", 14.f);
	if (customFont != nullptr)
		io.FontDefault = customFont;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ApplyStyle();

	ImGui_ImplSDL2_InitForOpenGL(app->window->window, app->window->context);
	ImGui_ImplOpenGL3_Init();

	hierarchyWindow = new HierarchyWindow(WindowType::HIERARCHY, "Hierarchy");
	editorWindows.push_back(hierarchyWindow);
	inspectorWindow = new InspectorWindow(WindowType::INSPECTOR, "Inspector");
	editorWindows.push_back(inspectorWindow);
	consoleWindow = new ConsoleWindow(WindowType::CONSOLE, "Console");
	editorWindows.push_back(consoleWindow);
	projectWindow = new ProjectWindow(WindowType::PROJECT, "Project");
	editorWindows.push_back(projectWindow);
	sceneWindow = new SceneWindow(WindowType::SCENE, "Scene");
	editorWindows.push_back(sceneWindow);
	performanceWindow = new PerformanceWindow(WindowType::PERFORMANCE, "Performance");
	editorWindows.push_back(performanceWindow);
	preferencesWindow = new PreferencesWindow(WindowType::PREFERENCES, "Preferences");
	editorWindows.push_back(preferencesWindow);
	aboutWindow = new AboutWindow(WindowType::ABOUT, "About");
	editorWindows.push_back(aboutWindow);

	return ret;
}

bool ModuleEditor::CleanUp()
{
	LOG(LogType::LOG_INFO, "Cleaning ModuleEditor");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void ModuleEditor::DrawEditor()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// Bar
	MainMenuBar();

	// Docking
	Docking();

	// Draw windows
	for (const auto& editorWindow : editorWindows)
	{
		if (editorWindow->IsEnabled())
			editorWindow->DrawWindow();
	}

	/*// Draw status bar
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

	if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, ImGui::GetFrameHeight(), windowFlags))
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}*/

	if (app->importer->isDraggingFile)
		app->importer->TryImportFile();

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ModuleEditor::Docking()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Dockspace", NULL, windowFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspaceId = ImGui::GetID("Dockspace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();
}

void ModuleEditor::MainMenuBar()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Exit", "Alt+F4"))
		{
			app->exit = true;
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Assets"))
	{
		if (ImGui::MenuItem("Show in Explorer"))
		{
			std::string folderPath = "Assets";
			std::string command = "explorer \"" + folderPath + "\"";
			system(command.c_str());
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("GameObject"))
	{
		if (ImGui::MenuItem("Create Empty"))
		{
			app->scene->CreateGameObject("GameObject", app->scene->root);
			selectedGameObject = app->scene->root->children.back();
		}
		if (ImGui::BeginMenu("3D Object"))
		{
			const char* objectNames[] = { "Cube", "Sphere", "Capsule", "Cylinder" };
			const char* basePath = "Engine/Primitives/";
			const char* extension = ".fbx";

			for (const char* name : objectNames)
			{
				std::string fullPath = std::string(basePath) + name + extension;

				if (ImGui::MenuItem(name))
				{
					Resource* resource = app->resources->FindResourceInLibrary(fullPath, ResourceType::MODEL);
					if (!resource)
						resource = app->importer->ImportFileToLibrary(fullPath, ResourceType::MODEL);

					app->importer->modelImporter->LoadModel(resource, app->scene->root);

					selectedGameObject = app->scene->root->children.back();
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Windows"))
	{
		int index = 1;
		for (auto& editorWindow : editorWindows)
		{
			bool isEnabled = editorWindow->IsEnabled();
			if (ImGui::MenuItem(editorWindow->GetName().c_str(), ("Ctrl+" + std::to_string(index)).c_str(), &isEnabled))
			{
				editorWindow->SetEnabled(isEnabled);
			}
			++index;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void ModuleEditor::ApplyStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// --- Backgrounds ---
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.12f, 0.12f, 0.14f, 1.0f };
	colors[ImGuiCol_ChildBg] = ImVec4{ 0.12f, 0.12f, 0.14f, 1.0f };
	colors[ImGuiCol_PopupBg] = ImVec4{ 0.14f, 0.14f, 0.17f, 0.95f };

	// --- Borders ---
	colors[ImGuiCol_Border] = ImVec4{ 0.32f, 0.32f, 0.36f, 0.5f };
	colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.2f };

	// --- Text ---
	colors[ImGuiCol_Text] = ImVec4{ 0.95f, 0.95f, 0.95f, 1.0f };
	colors[ImGuiCol_TextDisabled] = ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f };
	colors[ImGuiCol_TextSelectedBg] = ImVec4{ 0.45f, 0.55f, 0.70f, 0.6f };

	// --- Selectable ---
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.24f, 0.30f, 0.36f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.32f, 0.38f, 0.44f, 1.0f };

	// --- Buttons ---
	colors[ImGuiCol_Button] = ImVec4{ 0.22f, 0.22f, 0.26f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.30f, 0.35f, 0.42f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.28f, 0.32f, 0.38f, 1.0f };
	colors[ImGuiCol_CheckMark] = ImVec4{ 0.58f, 0.78f, 1.0f, 1.0f };

	// --- Sliders ---
	colors[ImGuiCol_SliderGrab] = ImVec4{ 0.45f, 0.55f, 0.70f, 0.75f };
	colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.65f, 0.75f, 0.90f, 1.0f };

	// --- Frame Background ---
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.18f, 0.18f, 0.22f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.26f, 0.26f, 0.30f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.24f, 0.24f, 0.28f, 1.0f };

	// --- Tabs ---
	colors[ImGuiCol_Tab] = ImVec4{ 0.22f, 0.22f, 0.26f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.32f, 0.38f, 0.45f, 1.0f };
	colors[ImGuiCol_TabSelected] = ImVec4{ 0.28f, 0.32f, 0.38f, 1.0f };
	colors[ImGuiCol_TabSelectedOverline] = ImVec4{ 0.65f, 0.75f, 0.90f, 1.0f };
	colors[ImGuiCol_TabDimmed] = ImVec4{ 0.18f, 0.18f, 0.22f, 1.0f };
	colors[ImGuiCol_TabDimmedSelected] = ImVec4{ 0.24f, 0.28f, 0.34f, 1.0f };
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4{ 0.58f, 0.68f, 0.80f, 1.0f };

	// --- Title Bar ---
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.22f, 0.22f, 0.26f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.24f, 0.28f, 0.34f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.22f, 0.22f, 0.26f, 1.0f };

	// --- Scrollbars ---
	colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.14f, 0.14f, 0.17f, 1.0f };
	colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.24f, 0.28f, 0.34f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.32f, 0.38f, 0.45f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.42f, 0.50f, 0.60f, 1.0f };

	// --- Separators ---
	colors[ImGuiCol_Separator] = ImVec4{ 0.32f, 0.32f, 0.36f, 1.0f };
	colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.58f, 0.68f, 0.80f, 1.0f };
	colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.65f, 0.75f, 0.90f, 1.0f };

	// --- Resize Grip ---
	colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.32f, 0.32f, 0.36f, 0.25f };
	colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.58f, 0.68f, 0.80f, 0.25f };
	colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.65f, 0.75f, 0.90f, 0.25f };

	// --- Docking ---
	colors[ImGuiCol_DockingPreview] = ImVec4{ 0.24f, 0.28f, 0.34f, 1.0f };
	colors[ImGuiCol_DockingEmptyBg] = ImVec4{ 0.12f, 0.12f, 0.14f, 1.0f };

	// --- Plot ---
	colors[ImGuiCol_PlotLines] = ImVec4{ 0.56f, 0.72f, 0.88f, 1.0f };
	colors[ImGuiCol_PlotLinesHovered] = ImVec4{ 0.84f, 0.91f, 1.0f, 1.0f };
	colors[ImGuiCol_PlotHistogram] = ImVec4{ 0.56f, 0.72f, 0.88f, 1.0f };
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4{ 0.84f, 0.91f, 1.0f, 1.0f };

	// --- Table ---
	colors[ImGuiCol_TableHeaderBg] = ImVec4{ 0.22f, 0.22f, 0.26f, 1.0f };
	colors[ImGuiCol_TableBorderStrong] = ImVec4{ 0.32f, 0.32f, 0.36f, 1.0f };
	colors[ImGuiCol_TableBorderLight] = ImVec4{ 0.18f, 0.18f, 0.22f, 1.0f };
	colors[ImGuiCol_TableRowBg] = ImVec4{ 0.16f, 0.16f, 0.18f, 1.0f };
	colors[ImGuiCol_TableRowBgAlt] = ImVec4{ 0.14f, 0.14f, 0.16f, 1.0f };

	// --- Drag & Drop ---
	colors[ImGuiCol_DragDropTarget] = ImVec4{ 0.84f, 0.91f, 1.0f, 1.0f };

	// --- Navigation ---
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4{ 0.58f, 0.68f, 0.80f, 0.8f };

	// --- Roundings ---
	style.TabRounding = 5;
	style.ScrollbarRounding = 9;
	style.WindowRounding = 8;
	style.GrabRounding = 3;
	style.FrameRounding = 4;
	style.PopupRounding = 4;
	style.ChildRounding = 4;

	style.WindowMenuButtonPosition = ImGuiDir_None;
}