#include "ModuleScene.h"
#include "App.h"
#include "ScriptMoveInCircle.h"
#include <fstream>

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

	if (app->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN)
	{
		SaveScene("Assets/Scenes/Scene.json");
	}
	if (app->input->GetKey(SDL_SCANCODE_H) == KEY_DOWN)
	{
		LoadScene("Assets/Scenes/Scene.json");
	}

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

void ModuleScene::SaveScene(const std::string& filePath) const
{
	nlohmann::json sceneJson;
	std::vector<GameObject*> objects;
	CollectObjects(root, objects);

	for (const auto& object : objects)
	{
		if (object != nullptr)
		{
			nlohmann::json objectJson;
			object->Serialize(objectJson);
			sceneJson["objects"].push_back(objectJson);
		}
	}

	nlohmann::json resourcesJson;
	const auto& resources = app->resources->GetResources();
	for (const auto& resource : resources)
	{
		if (resource != nullptr)
		{
			nlohmann::json resourceJson;
			resourceJson["type"] = static_cast<int>(resource->GetType());
			resourceJson["assetFileDir"] = resource->GetAssetFileDir();
			resourceJson["libraryFileDir"] = resource->GetLibraryFileDir();
			resourcesJson.push_back(resourceJson);
		}
	}
	sceneJson["resources"] = resourcesJson;

	std::ofstream file(filePath);
	if (file.is_open())
	{
		file << sceneJson.dump(4);
		file.close();
	}
}

void ModuleScene::LoadScene(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		return;

	nlohmann::json sceneJson;
	file >> sceneJson;
	file.close();

	for (auto* child : root->children) {
		delete child;
	}
	root->children.clear();

	app->renderer3D->meshQueue.clear();

	app->editor->selectedGameObject = nullptr;

	const auto& resourcesJson = sceneJson["resources"];
	for (const auto& resourceJson : resourcesJson)
	{
		ResourceType type = static_cast<ResourceType>(resourceJson["type"].get<int>());
		std::string assetFileDir = resourceJson["assetFileDir"].get<std::string>();
		std::string libraryFileDir = resourceJson["libraryFileDir"].get<std::string>();

		Resource* resource = app->resources->FindResourceInLibrary(libraryFileDir, type);
		if (!resource)
		{
			resource = app->resources->CreateResource(assetFileDir, type);
			resource->SetLibraryFileDir(libraryFileDir);
		}
		app->resources->ModifyResourceUsageCount(resource, 1);
	}

	std::unordered_map<std::string, GameObject*> objectMap;
	for (const auto& objectJson : sceneJson["objects"])
	{
		std::string name = objectJson["name"].get<std::string>();
		std::string uuid = objectJson["uuid"].get<std::string>();
		GameObject* object = CreateGameObject(name.c_str(), nullptr);
		objectMap[uuid] = object;
	}

	for (const auto& objectJson : sceneJson["objects"])
	{
		std::string name = objectJson["name"].get<std::string>();
		std::string uuid = objectJson["uuid"].get<std::string>();
		std::string parentUuid = objectJson["parent"].get<std::string>();
		GameObject* object = objectMap[uuid];

		GameObject* parent = parentUuid.empty() ? root : objectMap[parentUuid];
		if (parent && object)
		{
			object->parent = parent;
			parent->children.push_back(object);
		}

		object->Deserialize(objectJson);
	}
}