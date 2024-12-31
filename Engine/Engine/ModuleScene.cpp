#include "ModuleScene.h"
#include "App.h"
#include "ScriptMoveInCircle.h"

ModuleScene::ModuleScene(App* app) : Module(app), sceneBounds(glm::vec3(-15.0f), glm::vec3(15.0f))
{
	sceneCamera = new ComponentCamera(nullptr);
}

ModuleScene::~ModuleScene()
{
}

bool ModuleScene::Awake()
{
	root = CreateGameObject("Untitled Scene", nullptr);

	GameObject* camera = CreateGameObject("Camera", root);
	activeGameCamera = new ComponentCamera(camera);
	camera->AddComponent(activeGameCamera);
	camera->transform->position = glm::vec3(0.0f, 6.0f, 8.0f);
	camera->transform->eulerRotation = glm::vec3(-30.0f, 0.0f, 0.0f);
	camera->transform->UpdateTransform();

	sceneOctree = new Octree(sceneBounds, octreeMaxDepth, octreeMaxObjects);

	return true;
}

bool ModuleScene::Start()
{
	app->importer->ImportFile("Assets/Models/Street environment_V01.fbx", true);
	app->editor->selectedGameObject = app->scene->root->children[0];

	std::string fullPath = "Engine/Primitives/Capsule.fbx";

	Resource* resource = app->resources->FindResourceInLibrary(fullPath, ResourceType::MODEL);
	if (!resource)
		resource = app->importer->ImportFileToLibrary(fullPath, ResourceType::MODEL);

	app->resources->ModifyResourceUsageCount(resource, 1);
	app->importer->modelImporter->LoadModel(resource, app->scene->root);
	app->editor->selectedGameObject = app->scene->root->children.back();
	app->editor->selectedGameObject->transform->position = glm::vec3(0.0f, 1.0f, 0.0f);
	app->editor->selectedGameObject->transform->UpdateTransform();
	app->editor->selectedGameObject->AddComponent(new ScriptMoveInCircle(app->editor->selectedGameObject));
	app->editor->selectedGameObject->name = "Player";

	return true;
}

bool ModuleScene::Update(float dt)
{
	if (octreeNeedsUpdate)
	{
		UpdateOctree();
		sceneCamera->frustumNeedsUpdate = true;
		activeGameCamera->frustumNeedsUpdate = true;
		octreeNeedsUpdate = false;
	}

	root->Update();

	if (app->time.GetState() == GameState::STEP)
		app->time.SetState(GameState::PAUSE);

	return true;
}

void ModuleScene::UpdateOctree() const
{
	AABB newBounds;
	bool firstObject = true;

	std::vector<GameObject*> objects;
	CollectObjects(root, objects);

	for (const auto& object : objects)
	{
		if (object != nullptr)
		{
			if (firstObject)
			{
				newBounds = object->GetAABB();
				firstObject = false;
			}
			else
			{
				newBounds.min = glm::min(newBounds.min, object->GetAABB().min);
				newBounds.max = (glm::max)(newBounds.max, object->GetAABB().max);
			}
		}
	}

	sceneOctree->SetBounds(newBounds);
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
		if (object != nullptr && object->GetAABB().min != glm::vec3(0,0,0) && object->GetAABB().max != glm::vec3(0,0,0))
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
		if (child->parent != nullptr)
			objects.push_back(child);

		CollectObjects(child, objects);
	}
}

bool ModuleScene::CleanUp()
{
	LOG(LogType::LOG_INFO, "Cleaning ModuleScene");

	delete sceneOctree;
	sceneOctree = nullptr;

	delete root;
	root = nullptr;

	return true;
}

GameObject* ModuleScene::CreateGameObject(const char* name, GameObject* parent)
{
	GameObject* gameObject = new GameObject(name, parent);

	if (parent != nullptr) parent->children.push_back(gameObject);

	return gameObject;
}