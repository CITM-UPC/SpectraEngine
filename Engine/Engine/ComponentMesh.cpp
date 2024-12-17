#include "ComponentMesh.h"
#include "App.h"

#include <glm/gtc/type_ptr.hpp>

ComponentMesh::ComponentMesh(GameObject* gameObject) : Component(gameObject, ComponentType::MESH), mesh(nullptr)
{
}

ComponentMesh::~ComponentMesh()
{
}

void ComponentMesh::Update()
{
    ComponentTransform* transform = gameObject->transform;
    ComponentMaterial* material = gameObject->material;

    if (transform != nullptr)
    {   
        const AABB meshAABB = mesh->GetAABB(transform->globalTransform);

        if (!gameObject->isOctreeInFrustum) return;

        if (app->camera->IsAABBInFrustum(meshAABB))
        {
            app->camera->meshCount++;
            app->camera->vertexCount += mesh->verticesCount;
            app->camera->triangleCount += mesh->indicesCount / 3;

            glPushMatrix();
            glMultMatrixf(glm::value_ptr(transform->globalTransform));

            const auto& preferences = app->editor->preferencesWindow;

            mesh->DrawMesh(
                material->textureId,
                preferences->drawTextures,
                preferences->wireframe,
                preferences->shadedWireframe
            );

            if (drawOutline)
                mesh->DrawOutline(gameObject->isParentSelected);

            if (showVertexNormals || showFaceNormals)
            {
                mesh->DrawNormals(
                    showVertexNormals,
                    showFaceNormals,
                    preferences->vertexNormalLength,
                    preferences->faceNormalLength,
                    preferences->vertexNormalColor,
                    preferences->faceNormalColor
                );
            }

            glPopMatrix();

            if (app->editor->selectedGameObject == gameObject)
            {
                if (showAABB)
                    mesh->DrawAABB(transform->globalTransform);
                if (showOBB)
                    mesh->DrawOBB(transform->globalTransform);
            }
        }
    }
}

void ComponentMesh::OnEditor()
{
	if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Vertices: %d", mesh->verticesCount);
		ImGui::Text("Indices: %d", mesh->indicesCount);
		ImGui::Text("Normals: %d", mesh->normalsCount);
		ImGui::Text("Texture Coords: %d", mesh->texCoordsCount);

		ImGui::Spacing();

		ImGui::Checkbox("Vertex Normals", &showVertexNormals);
		ImGui::Checkbox("Face Normals", &showFaceNormals);

        ImGui::Spacing();
        
		ImGui::Checkbox("Show AABB", &showAABB);
		ImGui::Checkbox("Show OBB", &showOBB);
	}
}