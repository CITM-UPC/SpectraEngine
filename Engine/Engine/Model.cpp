#include "Model.h"

#include "App.h"

Model::Model() : Resource(ResourceType::MODEL)
{
}

Model::~Model()
{
}

void Model::DeleteMesh(const Mesh* mesh)
{
    auto it = std::find_if(meshInfos.begin(), meshInfos.end(),
        [mesh](const MeshInfo& info) { return info.mesh == mesh; });

    if (it != meshInfos.end())
        meshInfos.erase(it);

    if (meshInfos.empty())
    {
        app->resources->ModifyResourceUsageCount(this, -1);
        //app->resources->RemoveUnusedResource(this);
    }
}

Mesh* Model::GetMeshByIndex(uint32_t index) const
{
    auto it = std::find_if(meshInfos.begin(), meshInfos.end(),
        [index](const MeshInfo& info) { return info.originalIndex == index; });
    return it != meshInfos.end() ? it->mesh : nullptr;
}

void Model::AddMesh(Mesh* mesh, uint32_t index)
{
    meshInfos.push_back({ mesh, index });
}

bool Model::HasMeshAtIndex(uint32_t index) const
{
    return std::find_if(meshInfos.begin(), meshInfos.end(),
    [index](const MeshInfo& info) { return info.originalIndex == index; }) != meshInfos.end();
}