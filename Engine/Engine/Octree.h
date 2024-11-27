#pragma once

#include "Mesh.h"
#include "GameObject.h"

#include <vector>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

struct OctreeNode 
{
    AABB bounds;
    std::vector<GameObject*> objects;
    std::array<std::unique_ptr<OctreeNode>, 8> children;

    OctreeNode(const AABB& bounds) : bounds(bounds), children({ nullptr }) {}

    bool IsLeaf() const
    {
        for (const auto& child : children)
        {
            if (child) return false;
        }
        return true;
    }
};

class Octree
{
public:
    Octree(const AABB& sceneBounds, uint maxDepth = 5, uint maxObjects = 5);

    void Insert(GameObject* object, const AABB& objectBounds);
    void Remove(GameObject* object);
    std::vector<GameObject*> Query(const AABB& region) const;
    void Update(GameObject* object);

	void DebugPrintObjects() const;
    void DebugPrintNodeObjects(const OctreeNode* node, uint depth) const;

    void Draw(const glm::vec3& color = glm::vec3(1.0f, 1.0f , 0.0f)) const;
    void DrawView(ImDrawList* drawList, float scale, const ImVec2& windowSize, const ImVec2& windowPos, int type) const;

private:
    void Insert(OctreeNode* node, GameObject* object, const AABB& objectBounds, uint depth);
    void Remove(OctreeNode* node, GameObject* object);
    void Subdivide(OctreeNode* node);
    void Query(const OctreeNode* node, const AABB& region, std::vector<GameObject*>& results) const;
    void DrawNode(const OctreeNode* node, const glm::vec3& color) const;
    void DrawAABB(const AABB& aabb, const glm::vec3& color) const;
    bool Intersect(const AABB& a, const AABB& b) const;
    int TotalObjects(const OctreeNode* node) const;

    void DrawNodeView(const OctreeNode* node, ImDrawList* drawList, float scale, const ImVec2& windowSize, const glm::vec3& origin, const ImVec2& windowPos, const glm::vec2& translation, int type, uint depth) const;

private:
    std::unique_ptr<OctreeNode> root;
    uint maxDepth;
    uint maxObjects;
};