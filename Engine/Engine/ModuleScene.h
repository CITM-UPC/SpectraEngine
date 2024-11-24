#pragma once

#include "Module.h"
#include "GameObject.h"
#include "Quadtree.h"
#include <vector>
#include <glm/glm.hpp>

class GameObject;

class ModuleScene : public Module
{
public:
	ModuleScene(App* app);
	virtual ~ModuleScene();

	bool Awake();
	bool Update(float dt);
	bool CleanUp();

	GameObject* CreateGameObject(const char* name, GameObject* parent);

	void UpdateQuadtree();
	void DrawQuadtree();

private:
	void AddGameObjectToQuadtree(GameObject* gameObject);
	void CollectMeshes(GameObject* gameObject, std::vector<Mesh*>& meshes, std::vector<glm::mat4>& transforms);

public:
	GameObject* root = nullptr;
	Quadtree* sceneQuadtree = nullptr;
	AABB sceneBounds;
};