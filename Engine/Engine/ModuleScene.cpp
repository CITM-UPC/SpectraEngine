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
	if (octreeNeedsUpdate)
	{
		UpdateOctree();
		app->camera->frustumNeedsUpdate = true;
		octreeNeedsUpdate = false;
	}

	root->Update();

	sceneOctree->Draw();

	return true;
}

void ModuleScene::UpdateOctree() const
{
	sceneOctree->Clear();
	AddGameObjectToOctree(root);
}

void ModuleScene::AddGameObjectToOctree(const GameObject* gameObject) const
{
	if (gameObject == nullptr)
		return;

	std::vector<GameObject*> objects;
	CollectObjects(gameObject, objects);

	for (const auto& object : objects)
	{
		if (object != nullptr)
		{
			sceneOctree->Insert(object, object->GetAABB());
		}
	}
}

void ModuleScene::CollectObjects(const GameObject* gameObject, std::vector<GameObject*>& objects) const
{
	if (gameObject == nullptr)
		return;

	for (auto* child : gameObject->children)
	{
		if (gameObject->parent != nullptr)
			objects.push_back(child);

		CollectObjects(child, objects);
	}
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