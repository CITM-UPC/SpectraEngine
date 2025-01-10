#pragma once

#include "ComponentCamera.h"
#include "Module.h"
#include "GameObject.h"
#include "Octree.h"
#include "Mesh.h"
#include <nlohmann/json.hpp>

class GameObject;

class ModuleScene : public Module
{
public:
	ModuleScene(App* app);
	virtual ~ModuleScene();

	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	GameObject* CreateGameObject(const char* name, GameObject* parent);
	void CollectObjects(const GameObject* gameObject, std::vector<GameObject*>& objects) const;

	void SaveScene(const std::string& filePath) const;
	void LoadScene(const std::string& filePath);

private:
	void UpdateOctree() const;
	void AddGameObjectToOctree(const GameObject* gameObject) const;

public:
	GameObject* root = nullptr;
	Octree* sceneOctree = nullptr;
	AABB sceneBounds;

	int octreeMaxDepth = 3;
	int octreeMaxObjects = 4;
	glm::vec3 octreeColor = glm::vec3(0.0f, 1.0f, 1.0f);
	bool drawOctree = false;

	bool octreeNeedsUpdate = true;

	ComponentCamera* sceneCamera = nullptr;
	ComponentCamera* activeGameCamera = nullptr;
};