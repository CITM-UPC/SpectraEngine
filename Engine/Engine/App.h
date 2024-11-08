#pragma once
#include "Logger.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleCamera.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ModuleImporter.h"

#include "Timer.h"

#include <list>

class App
{
public:
	App(int argc, char* argv[]);
	~App();

	bool Awake();
	bool Start();
	bool Update();
	bool CleanUp();

	float GetDT() { return dt; }

private:
	void AddModule(Module* module, bool enable = true);

	void PrepareUpdate();
	void FinishUpdate();

public:
	ModuleWindow* window = nullptr;
	ModuleCamera* camera = nullptr;
	ModuleInput* input = nullptr;
	ModuleScene* scene = nullptr;	
	ModuleImporter* importer = nullptr;
	ModuleRenderer3D* renderer3D = nullptr;
	ModuleEditor* editor = nullptr;

	bool exit = false;

private:
	Timer	ms_timer;
	float	dt;

	std::list<Module*> modules;
};

extern App* app;