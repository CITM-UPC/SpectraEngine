#include "ModuleScene.h"
#include "App.h"

ModuleScene::ModuleScene(App* app) : Module(app), root(nullptr)
{
	sceneBounds = AABB(glm::vec3(-20, -20, -20), glm::vec3(20, 20, 20));
}

ModuleScene::~ModuleScene()
{
}

bool ModuleScene::Awake()
{
	root = CreateGameObject("Untitled Scene", nullptr);

	sceneQuadtree = new Quadtree(sceneBounds);

	return true;
}

bool ModuleScene::Update(float dt)
{
	UpdateQuadtree();

	root->Update();

	DrawQuadtree();

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

void ModuleScene::UpdateQuadtree()
{
    delete sceneQuadtree;
    sceneQuadtree = new Quadtree(sceneBounds);

    AddGameObjectToQuadtree(root);
}

void ModuleScene::AddGameObjectToQuadtree(GameObject* gameObject)
{
    if (gameObject == nullptr)
        return;

    std::vector<Mesh*> meshes;
	std::vector<glm::mat4> transforms;
    CollectMeshes(gameObject, meshes, transforms);

    for (size_t i = 0; i < meshes.size(); ++i)
    {
        if (meshes[i] != nullptr)
        {
            sceneQuadtree->AddObject(meshes[i], transforms[i]);
        }
    }
}

void ModuleScene::CollectMeshes(GameObject* gameObject, std::vector<Mesh*>& meshes, std::vector<glm::mat4>& transforms)
{
    if (gameObject == nullptr)
        return;

    for (auto* component : gameObject->components)
    {
        if (component->type == ComponentType::MESH)
        {
            ComponentMesh* meshComponent = static_cast<ComponentMesh*>(component);
            if (meshComponent->mesh != nullptr)
            {
                meshes.push_back(meshComponent->mesh);
				transforms.push_back(gameObject->transform->globalTransform);
            }
        }
    }

    for (auto* child : gameObject->children)
    {
        CollectMeshes(child, meshes, transforms);
    }
}

void ModuleScene::DrawQuadtree()
{
    if (sceneQuadtree != nullptr)
        sceneQuadtree->Draw();
}