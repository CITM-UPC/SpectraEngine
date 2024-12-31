#include "ComponentCamera.h"

#include "App.h"

ComponentCamera::ComponentCamera(GameObject* owner) : Component(owner, ComponentType::CAMERA)
{
	screenWidth = app->window->width;
	screenHeight = app->window->height;

	CalculateViewMatrix();

	X = glm::vec3(1.0f, 0.0f, 0.0f);
	Y = glm::vec3(0.0f, 1.0f, 0.0f);
	Z = glm::vec3(0.0f, 0.0f, 1.0f);

	position = glm::vec3(0.0f, 5.0f, 5.0f);
	reference = glm::vec3(0.0f, 0.0f, 0.0f);
}

ComponentCamera::~ComponentCamera()
{
}

void ComponentCamera::Update()
{
	CalculateViewMatrix();

	if (frustumNeedsUpdate)
	{
		CalculateFrustumPlanes();
		frustumNeedsUpdate = false;
	}

	meshCount = 0;
	vertexCount = 0;
	triangleCount = 0;

	if (gameObject)
	{
		position = gameObject->transform->position;

		X = glm::vec3(gameObject->transform->localTransform[0]);
		Y = glm::vec3(gameObject->transform->localTransform[1]);
		Z = glm::vec3(gameObject->transform->localTransform[2]);

	}
}

void ComponentCamera::OnEditor()
{
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Fov ");
		ImGui::SameLine();
		if (ImGui::SliderFloat("##Fov", &app->scene->activeGameCamera->fov, 4.0f, 120.0f))
			app->scene->activeGameCamera->frustumNeedsUpdate = true;

		ImGui::Text("Near");
		ImGui::SameLine();
		ImGui::InputFloat("##Near", &app->scene->activeGameCamera->nearPlane);

		ImGui::Text("Far ");
		ImGui::SameLine();
		ImGui::InputFloat("##Far", &app->scene->activeGameCamera->farPlane);
	}
}

void ComponentCamera::CalculateViewMatrix()
{
	viewMatrix = glm::mat4(
		X.x, Y.x, Z.x, 0.0f,
		X.y, Y.y, Z.y, 0.0f,
		X.z, Y.z, Z.z, 0.0f,
		-glm::dot(X, position), -glm::dot(Y, position), -glm::dot(Z, position), 1.0f
	);
}

glm::mat4 ComponentCamera::GetProjectionMatrix() const
{
	float aspectRatio = app->editor->sceneWindow->windowSize.x / app->editor->sceneWindow->windowSize.y;
	return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void ComponentCamera::CalculateFrustumPlanes()
{
	projectionMatrix = GetProjectionMatrix();
	glm::mat4 viewProjMatrix = projectionMatrix * viewMatrix;

	// Left, Right, Bottom, Top, Near, Far
	const int components[6] = { 0, 0, 1, 1, 2, 2 };
	const float signs[6] = { 1, -1, 1, -1, 1, -1 };

	for (int i = 0; i < 6; ++i)
	{
		int c = components[i / 2];
		float sign = signs[i];

		frustumPlanes[i].normal.x = viewProjMatrix[0][3] + sign * viewProjMatrix[0][c];
		frustumPlanes[i].normal.y = viewProjMatrix[1][3] + sign * viewProjMatrix[1][c];
		frustumPlanes[i].normal.z = viewProjMatrix[2][3] + sign * viewProjMatrix[2][c];
		frustumPlanes[i].distance = viewProjMatrix[3][3] + sign * viewProjMatrix[3][c];

		float length = glm::length(frustumPlanes[i].normal);
		frustumPlanes[i].normal /= length;
		frustumPlanes[i].distance /= length;
	}

	app->scene->sceneOctree->UpdateAllNodesVisibility(this);
}

bool ComponentCamera::IsAABBInFrustum(const AABB& aabb) const
{
	for (int i = 0; i < 6; ++i)
	{
		const Plane& plane = frustumPlanes[i];
		bool anyVertexInside = false;

		for (int corner = 0; corner < 8 && !anyVertexInside; ++corner)
		{
			glm::vec3 vertex(
				(corner & 1) ? aabb.max.x : aabb.min.x,
				(corner & 2) ? aabb.max.y : aabb.min.y,
				(corner & 4) ? aabb.max.z : aabb.min.z
			);

			if (glm::dot(plane.normal, vertex) + plane.distance >= 0)
				anyVertexInside = true;
		}

		if (!anyVertexInside)
			return false;
	}

	return true;
}