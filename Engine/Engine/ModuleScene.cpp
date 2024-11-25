#include "ModuleScene.h"
#include "App.h"

ModuleScene::ModuleScene(App* app) : Module(app), root(nullptr)
{
}

ModuleScene::~ModuleScene()
{
}

bool ModuleScene::Awake()
{
	root = CreateGameObject("Untitled Scene", nullptr);

	sceneBounds = AABB(glm::vec3(-15.0f), glm::vec3(15.0f));
	sceneOctree = new Octree(sceneBounds, 3, 4);

	return true;
}

bool ModuleScene::Update(float dt)
{
	root->Update();

	sceneOctree->Draw();
	sceneOctree->DebugPrintObjects();

	return true;
}

bool ModuleScene::CleanUp()
{
	LOG(LogType::LOG_INFO, "Cleaning ModuleScene");

	return true;
}

GameObject* ModuleScene::CreateGameObject(const char* name, GameObject* parent)
{
	GameObject* gameObject = new GameObject(name, parent);

	if (parent != nullptr) parent->children.push_back(gameObject);

	return gameObject;
}