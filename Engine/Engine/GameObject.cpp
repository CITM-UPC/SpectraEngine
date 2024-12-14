#include "GameObject.h"

GameObject::GameObject(const char* name, GameObject* parent) : parent(parent), name(name)
{
	transform = new ComponentTransform(this);
	mesh = new ComponentMesh(this);
	material = new ComponentMaterial(this);

	AddComponent(transform);
}

GameObject::~GameObject()
{
}

void GameObject::Update()
{
	if (isActive)
	{
		for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		{
			(*it)->Update();
		}
		for (std::vector<GameObject*>::iterator it = children.begin(); it != children.end(); ++it)
		{
			(*it)->Update();
		}
	}
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

    bool hit = false;
    float closestDistance = FLT_MAX;

    for (uint i = 0; i < indicesCount; i += 3)
    {
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
