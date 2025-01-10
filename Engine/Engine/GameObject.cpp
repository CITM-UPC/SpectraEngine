#include "GameObject.h"

#include <random>

#include "App.h"
#include "ComponentScript.h"

GameObject::GameObject(const char* name, GameObject* parent) : parent(parent), name(name), uuid(GenerateUUID())
{
	transform = new ComponentTransform(this);
	mesh = new ComponentMesh(this);
	material = new ComponentMaterial(this);

	AddComponent(transform);
}

GameObject::~GameObject()
{
    for (auto& child : children)
    {
        delete child;
        child = nullptr;
    }
    children.clear();

    if (std::find(components.begin(), components.end(), transform) == components.end() && transform)
        delete transform;

    if (std::find(components.begin(), components.end(), mesh) == components.end() && mesh)
        delete mesh;

    if (std::find(components.begin(), components.end(), material) == components.end() && material)
        delete material;

    for (auto& component : components)
    {
        delete component;
        component = nullptr;
    }
    components.clear();

    transform = nullptr;
    mesh = nullptr;
    material = nullptr;
}

void GameObject::Update()
{
	if (isActive)
	{
		for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		{
			if ((*it)->type == ComponentType::SCRIPT && !(app->time.GetState() == GameState::PLAY || app->time.GetState() == GameState::STEP))
				continue;

			(*it)->Update();
		}
		for (std::vector<GameObject*>::iterator it = children.begin(); it != children.end(); ++it)
		{
			(*it)->Update();
		}
	}

    isParentSelected = parent && (app->editor->selectedGameObject == parent || parent->isParentSelected);

    if (mesh)
        mesh->drawOutline = isParentSelected || app->editor->selectedGameObject == this;
}

void GameObject::Enable()
{
}
void GameObject::Disable()
{
}

Component* GameObject::AddComponent(Component* component)
{
	components.push_back(component);

	return component;
}

Component* GameObject::GetComponent(ComponentType type)
{
	for (auto it = components.begin(); it != components.end(); ++it) {
		if ((*it)->type == type) {
			return (*it);
		}
	}

	return nullptr;
}

AABB GameObject::GetAABB()
{
	if (transform != nullptr && mesh != nullptr && mesh->mesh != nullptr)
		aabb = mesh->mesh->GetAABB(transform->globalTransform);

	return aabb;
}

bool GameObject::IntersectsRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, float& intersectionDistance) const
{
    if (!transform || !mesh || !mesh->mesh)
        return false;

    const glm::mat4& transformMatrix = transform->globalTransform;
    const float* vertices = mesh->mesh->vertices;
    const uint* indices = mesh->mesh->indices;
    uint indicesCount = mesh->mesh->indicesCount;
	uint verticesCount = mesh->mesh->verticesCount;

    bool hit = false;
    float closestDistance = FLT_MAX;

    for (uint i = 0; i < indicesCount; i += 3)
    {
        if (indices[i] >= verticesCount || indices[i + 1] >= verticesCount || indices[i + 2] >= verticesCount)
            return false;

        glm::vec3 v0 = glm::vec3(transformMatrix * glm::vec4(vertices[indices[i] * 3], vertices[indices[i] * 3 + 1], vertices[indices[i] * 3 + 2], 1.0f));
        glm::vec3 v1 = glm::vec3(transformMatrix * glm::vec4(vertices[indices[i + 1] * 3], vertices[indices[i + 1] * 3 + 1], vertices[indices[i + 1] * 3 + 2], 1.0f));
        glm::vec3 v2 = glm::vec3(transformMatrix * glm::vec4(vertices[indices[i + 2] * 3], vertices[indices[i + 2] * 3 + 1], vertices[indices[i + 2] * 3 + 2], 1.0f));

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 h = glm::cross(rayDirection, edge2);
        float a = glm::dot(edge1, h);

        if (fabs(a) < 0.0001f)
            continue;

        float f = 1.0f / a;
        glm::vec3 s = rayOrigin - v0;
        float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f)
            continue;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(rayDirection, q);

        if (v < 0.0f || u + v > 1.0f)
            continue;

        float t = f * glm::dot(edge2, q);

        if (t > 0.0001f && t < closestDistance)
        {
            closestDistance = t;
            hit = true;
        }
    }

    if (hit)
    {
        intersectionDistance = closestDistance;
        return true;
    }

    return false;
}

void GameObject::Serialize(nlohmann::json& json) const
{
    json["name"] = name;
	json["uuid"] = uuid;

    json["parent"] = (parent != nullptr && parent->parent != nullptr) ? parent->uuid : "";

	json["components"] = nlohmann::json::array();
    for (auto& component : components)
    {
        nlohmann::json componentJson;
        component->Serialize(componentJson);
        json["components"].push_back(componentJson);
    }
}

void GameObject::Deserialize(const nlohmann::json& json)
{
    for (const auto& componentJson : json["components"])
    {
        ComponentType type = static_cast<ComponentType>(componentJson["type"].get<int>());

        Component* component = nullptr;
        switch (type)
        {
        case ComponentType::TRANSFORM:
			component = transform;
            break;
        case ComponentType::MESH:
            component = mesh;
            break;
        case ComponentType::MATERIAL:
            component = material;
            break;
        case ComponentType::CAMERA:
            component = new ComponentCamera(this);
            app->scene->activeGameCamera = dynamic_cast<ComponentCamera*>(component);
            break;
        case ComponentType::SCRIPT:
            component = new ComponentScript(this);
            break;
        }

        if (component)
        {
            if (type != ComponentType::TRANSFORM)
				AddComponent(component);

			component->Deserialize(componentJson);
        }
    }
}

std::string GameObject::GenerateUUID()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 15);
    std::uniform_int_distribution<int> dis2(8, 11);

    std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    for (char& c : uuid) {
        if (c == 'x') {
            c = "0123456789abcdef"[dis(gen)];
        }
        else if (c == 'y') {
            c = "89ab"[dis2(gen)];
        }
    }
    return uuid;
}