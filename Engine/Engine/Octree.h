#pragma once

#include "Mesh.h"
#include <vector>
#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

struct OctreeNode 
{
    AABB bounds;
    std::vector<Mesh*> objects;
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

    void Insert(Mesh* object, const AABB& objectBounds);

    void Remove(Mesh* object);
    void Remove(OctreeNode* node, Mesh* object);

    std::vector<Mesh*> Query(const AABB& region) const;

    void Clear();

    void Draw(const glm::vec3& color = glm::vec3(1.0f, 1.0f , 0.0f)) const;

    void Update(Mesh* object, const glm::mat4& transform);
    int TotalObjects(const OctreeNode* node) const;

	void DebugPrintObjects() const;
    void DebugPrintNodeObjects(const OctreeNode* node, uint depth) const;

    void DrawTopDownView(ImDrawList* draw_list, float scale, const ImVec2& window_size, const ImVec2& window_pos) const;
    void DrawTopDownNode(const OctreeNode* node, ImDrawList* draw_list, float scale, const ImVec2& window_size, const glm::vec3& origin, const glm::vec3& size, const ImVec2& window_pos, const glm::vec2& translation) const;

    void DrawSideView(ImDrawList* draw_list, float scale, const ImVec2& window_size, const ImVec2& window_pos) const;
    void DrawSideNode(const OctreeNode* node, ImDrawList* draw_list, float scale, const ImVec2& window_size, const glm::vec3& origin, const glm::vec3& size, const ImVec2& window_pos, const glm::vec2& translation) const;

    void DrawFrontView(ImDrawList* draw_list, float scale, const ImVec2& window_size, const ImVec2& window_pos) const;
    void DrawFrontNode(const OctreeNode* node, ImDrawList* draw_list, float scale, const ImVec2& window_size, const glm::vec3& origin, const glm::vec3& size, const ImVec2& window_pos, const glm::vec2& translation) const;

private:
    void Insert(OctreeNode* node, Mesh* object, const AABB& objectBounds, uint depth);
    void Subdivide(OctreeNode* node);
    void Query(const OctreeNode* node, const AABB& region, std::vector<Mesh*>& results) const;
    void DrawNode(const OctreeNode* node, const glm::vec3& color) const;
    void DrawAABB(const AABB& aabb, const glm::vec3& color) const;
    bool Intersect(const AABB& a, const AABB& b) const;

private:
    std::unique_ptr<OctreeNode> root;
    uint maxDepth;
    uint maxObjects;
};