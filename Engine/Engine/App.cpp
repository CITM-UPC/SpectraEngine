#include "App.h"

App* app = nullptr;

App::App(int argc, char* argv[])
{
	app = this;

	window = new ModuleWindow(this);
	camera = new ModuleCamera(this);
	input = new ModuleInput(this);
	scene = new ModuleScene(this);
	importer = new ModuleImporter(this);
	renderer3D = new ModuleRenderer3D(this);
	editor = new ModuleEditor(this);
	fileSystem = new ModuleFileSystem(this);
	resources = new ModuleResources(this);

	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(fileSystem);
	AddModule(resources);
	AddModule(importer);
	AddModule(scene);
	AddModule(editor);
	AddModule(renderer3D);

	window->loadingBarWidth = static_cast<int>(335 / (modules.size() * 2));
}

App::~App()
{
	for (auto it = modules.rbegin(); it != modules.rend(); ++it)
	{
		delete (*it);
	}

	modules.clear();
}

bool App::Awake()
{
	bool ret = true;

	for (const auto& module : modules)
	{
		if (!module->active)
			continue;

		ret = module->Awake();
		window->RenderInitialScreen();
	}

	timer.Start();

	return ret;
}

bool App::Start()
{
	bool ret = true;

	for (const auto& module : modules)
	{
		if (!module->active)
			continue;

		ret = module->Start();
		window->RenderInitialScreen();
	}

	ret = window->StartWindow();

	return ret;
}

void App::PrepareUpdate()
{
	dt = timer.ReadMs() / 1000.0f;
	timer.Start();
}

bool App::Update()
{
	bool ret = true;

	PrepareUpdate();

	if (ret)
	{
		for (const auto& module : modules)
		{
			if (!module->active)
				continue;

			ret = module->PreUpdate(dt);

			if (!ret)
				break;
		}
	}

	if (ret)
	{
		for (const auto& module : modules)
		{
			if (!module->active)
				continue;

			ret = module->Update(dt);

			if (!ret)
				break;
		}
	}

	if (ret)
	{
		for (const auto& module : modules)
		{
			if (!module->active)
				continue;

			ret = module->PostUpdate(dt);

			if (!ret)
				break;
		}
	}

	FinishUpdate();

	return ret;
}

void App::FinishUpdate()
{
	if (!vsync)
	{
		const float frameDelay = 1000.0f / maxFps;

		float frameTime = timer.ReadMs();

		if (frameTime < frameDelay)
			SDL_Delay((Uint32)(frameDelay - frameTime));
	}

	time.Update();
}

bool App::CleanUp()
{
	bool ret = true;

	for (auto it = modules.rbegin(); it != modules.rend(); ++it)
	{
		ret = (*it)->CleanUp();
	}

	return ret;
}

void App::AddModule(Module* module, bool activate)
{
	if (activate)
		module->Init();

	modules.push_back(module);
}