#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "Component.h"
#include "Mesh.h"

struct Plane
{
	glm::vec3 normal;
	float distance;
};

class ComponentCamera : public Component
{
public:
	ComponentCamera(GameObject* owner);
	~ComponentCamera();

	void Update() override;

	const glm::mat4& GetViewMatrix() const { return viewMatrix; }
	glm::mat4 GetProjectionMatrix() const;

	bool IsAABBInFrustum(const AABB& aabb) const;

	void CalculateViewMatrix();

private:
	void CalculateFrustumPlanes();

public:
	float fov = 60.0f;
	float nearPlane = 0.125f;
	float farPlane = 512.0f;
	int screenWidth, screenHeight;

	bool frustumNeedsUpdate = true;

	int meshCount = 0;
	int vertexCount = 0;
	int triangleCount = 0;

	glm::vec3 X, Y, Z;
	glm::vec3 position, reference;

private:
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	Plane frustumPlanes[6];
};
