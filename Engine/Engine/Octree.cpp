#include "Octree.h"
#include <algorithm>
#include <iostream>

Octree::Octree(const AABB& sceneBounds, uint maxDepth, uint maxObjects)
    : root(std::make_unique<OctreeNode>(sceneBounds)), maxDepth(maxDepth), maxObjects(maxObjects) {}

void Octree::Insert(Mesh* object, const AABB& objectBounds)
{
    Insert(root.get(), object, objectBounds, 0);
}

void Octree::Insert(OctreeNode* node, Mesh* object, const AABB& objectBounds, uint depth)
{
    if (!Intersect(node->bounds, objectBounds))
    {
        return;
    }

    if (node->IsLeaf() && (node->objects.size() < maxObjects || depth >= maxDepth))
    {
        node->objects.push_back(object);
        return;
    }

    if (node->IsLeaf())
    {
        Subdivide(node);
    }

    for (auto& child : node->children)
    {
        if (child)
        {
            Insert(child.get(), object, objectBounds, depth + 1);
        }
    }
}

void Octree::Subdivide(OctreeNode* node)
{
    glm::vec3 size = (node->bounds.max - node->bounds.min) * 0.5f;
    glm::vec3 mid = node->bounds.min + size;

    for (int i = 0; i < 8; ++i) 
    {
        glm::vec3 offset(
            (i & 1) ? size.x : 0.0f,
            (i & 2) ? size.y : 0.0f,
            (i & 4) ? size.z : 0.0f
        );

        glm::vec3 childMin = node->bounds.min + offset;
        glm::vec3 childMax = childMin + size;

        node->children[i] = std::make_unique<OctreeNode>(AABB(childMin, childMax));
    }
}

void Octree::Remove(Mesh* object) 
{
    Remove(root.get(), object);
}

void Octree::Remove(OctreeNode* node, Mesh* object) {
    if (!node) return;

    auto it = std::remove(node->objects.begin(), node->objects.end(), object);
    if (it != node->objects.end())
    {
        node->objects.erase(it, node->objects.end());
    }

    for (auto& child : node->children)
    {
        if (child)
        {
            Remove(child.get(), object);
        }
    }

    if (!node->IsLeaf())
    {
        int totalObjects = TotalObjects(node);

        if (totalObjects <= maxObjects)
        {
            for (auto& child : node->children)
            {
                if (child) 
                {
                    node->objects.insert(node->objects.end(), child->objects.begin(), child->objects.end());
                    child->objects.clear();
                }
            }

            for (auto& child : node->children)
            {
                child.reset();
            }
        }
    }
}


int Octree::TotalObjects(const OctreeNode* node) const
{
    if (!node) return 0;

    int count = node->objects.size();
    for (const auto& child : node->children)
    {
        if (child)
        {
            count += TotalObjects(child.get());
        }
    }

    return count;
}


std::vector<Mesh*> Octree::Query(const AABB& region) const
{
    std::vector<Mesh*> results;
    Query(root.get(), region, results);
    return results;
}

void Octree::Query(const OctreeNode* node, const AABB& region, std::vector<Mesh*>& results) const
{
    if (!node || !Intersect(node->bounds, region))
    {
        return;
    }

    for (Mesh* object : node->objects)
    {
        if (Intersect(object->GetAABB(), region))
        {
            results.push_back(object);
        }
    }

    for (const auto& child : node->children)
    {
        Query(child.get(), region, results);
    }
}

void Octree::Clear()
{
    root = std::make_unique<OctreeNode>(root->bounds);
}

void Octree::DrawAABB(const AABB& aabb, const glm::vec3& color) const
{
    glm::vec3 vertices[8] = {
        aabb.min,
        glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
        aabb.max
    };

    unsigned int indices[24] = {
        0, 1, 0, 2, 0, 4, // Min corner
        1, 3, 1, 5,       // Adjacent min corner
        2, 3, 2, 6,       // Top of the min face
        3, 7,             // Remaining edge on the top face
        4, 5, 4, 6,       // Max corner
        5, 7, 6, 7        // Edges completing the cube
    };

    glColor3f(color.r, color.g, color.b);

    glBegin(GL_LINES);
    for (int i = 0; i < 24; ++i) 
    {
        const glm::vec3& vertex = vertices[indices[i]];
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();
}

void Octree::Draw(const glm::vec3& color) const
{
    DrawNode(root.get(), color);
}

void Octree::DrawNode(const OctreeNode* node, const glm::vec3& color) const
{
    if (!node) return;

    DrawAABB(node->bounds, color);

    for (const auto& child : node->children)
    {
        if (child)
        {
            DrawNode(child.get(), color);
        }
    }
}

bool Octree::Intersect(const AABB& a, const AABB& b) const
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) && (a.min.y <= b.max.y && a.max.y >= b.min.y) && (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

void Octree::Update(Mesh* object, const glm::mat4& transform) 
{
    AABB transformedAABB = object->GetAABB(transform);

    Remove(object);
    Insert(object, transformedAABB);
}

void Octree::DebugPrintObjects() const 
{
    system("cls");
    //DebugPrintNodeObjects(root.get(), 0);
}

void Octree::DebugPrintNodeObjects(const OctreeNode* node, uint depth) const 
{
    if (!node) return;

    if (node->objects.size() > 0)
    {
        std::cout << "Depth: " << depth << " | AABB: ("
        << node->bounds.min.x << ", " << node->bounds.min.y << ", " << node->bounds.min.z
        << ") to ("
        << node->bounds.max.x << ", " << node->bounds.max.y << ", " << node->bounds.max.z
        << ") | Objects: " << node->objects.size() << std::endl;
    }

    for (auto* object : node->objects) 
    {
        std::cout << "  Object: " << object << std::endl;
    }

    for (const auto& child : node->children)
    {
        if (child)
        {
            DebugPrintNodeObjects(child.get(), depth + 1);
        }
    }
}

void Octree::DrawTopDownView(ImDrawList* draw_list, float scale, const ImVec2& window_size, const ImVec2& window_pos) const
{
    glm::vec3 origin = root->bounds.min;
    glm::vec3 size = root->bounds.max - root->bounds.min;

    // XZ
    glm::vec3 octree_center = origin + size * 0.5f;
    glm::vec2 center_offset = glm::vec2(window_size.x, window_size.y) * 0.5f;
    glm::vec2 center_pos(octree_center.x, octree_center.z);

    glm::vec2 translation = center_offset - center_pos;

    DrawTopDownNode(root.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
}

void Octree::DrawTopDownNode(const OctreeNode* node, ImDrawList* draw_list, float scale, const ImVec2& window_size, const glm::vec3& origin, const glm::vec3& size, const ImVec2& window_pos, const glm::vec2& translation) const
{
    if (!node) return;

    // XZ
    glm::vec2 min2D(node->bounds.min.x, node->bounds.min.z);
    glm::vec2 max2D(node->bounds.max.x, node->bounds.max.z);

    min2D = (min2D - glm::vec2(origin.x, origin.z)) * scale;
    max2D = (max2D - glm::vec2(origin.x, origin.z)) * scale;

    min2D += translation;
    max2D += translation;

    ImVec2 imMin(min2D.x + window_pos.x, window_pos.y + min2D.y);
    ImVec2 imMax(max2D.x + window_pos.x, window_pos.y + max2D.y);

    draw_list->AddRect(imMin, imMax, IM_COL32(255, 0, 0, 255));

    for (const auto& child : node->children)
    {
        if (child)
        {
            DrawTopDownNode(child.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
        }
    }
}

void Octree::DrawBottomView(ImDrawList* draw_list, float scale, const ImVec2& window_size, const ImVec2& window_pos) const
{
    glm::vec3 origin = root->bounds.min;
    glm::vec3 size = root->bounds.max - root->bounds.min;

    // XZ
    glm::vec3 octree_center = origin + size * 0.5f;
    glm::vec2 center_offset = glm::vec2(window_size.x, window_size.y) * 0.5f;
    glm::vec2 center_pos(octree_center.x, octree_center.z);

    glm::vec2 translation = center_offset - center_pos;

    DrawBottomNode(root.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
}

void Octree::DrawBottomNode(const OctreeNode* node, ImDrawList* draw_list, float scale, const ImVec2& window_size, const glm::vec3& origin, const glm::vec3& size, const ImVec2& window_pos, const glm::vec2& translation) const
{
    if (!node) return;

    // XZ
    glm::vec2 min2D(node->bounds.min.x, node->bounds.min.z);
    glm::vec2 max2D(node->bounds.max.x, node->bounds.max.z);

    min2D = (min2D - glm::vec2(origin.x, origin.z)) * scale;
    max2D = (max2D - glm::vec2(origin.x, origin.z)) * scale;

    min2D += translation;
    max2D += translation;

    ImVec2 imMin(min2D.x + window_pos.x, window_pos.y + window_size.y - max2D.y);
    ImVec2 imMax(max2D.x + window_pos.x, window_pos.y + window_size.y - min2D.y);

    draw_list->AddRect(imMin, imMax, IM_COL32(0, 255, 0, 255));

    for (const auto& child : node->children)
    {
        if (child)
        {
            DrawBottomNode(child.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
        }
    }
}

void Octree::DrawFrontView(ImDrawList* draw_list, float scale, const ImVec2& window_size, const ImVec2& window_pos) const
{
    glm::vec3 origin = root->bounds.min;
    glm::vec3 size = root->bounds.max - root->bounds.min;

    // XY
    glm::vec3 octree_center = origin + size * 0.5f;
    glm::vec2 center_offset = glm::vec2(window_size.x, window_size.y) * 0.5f;
    glm::vec2 center_pos(octree_center.x, octree_center.y);

    glm::vec2 translation = center_offset - center_pos;

    DrawFrontNode(root.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
}

void Octree::DrawFrontNode(const OctreeNode* node, ImDrawList* draw_list, float scale, const ImVec2& window_size, const glm::vec3& origin, const glm::vec3& size, const ImVec2& window_pos, const glm::vec2& translation) const
{
    if (!node) return;

    // XY
    glm::vec2 min2D(node->bounds.min.x, node->bounds.min.y);
    glm::vec2 max2D(node->bounds.max.x, node->bounds.max.y);

    min2D.y = -min2D.y;
    max2D.y = -max2D.y;

    min2D = (min2D - glm::vec2(origin.x, origin.y)) * scale;
    max2D = (max2D - glm::vec2(origin.x, origin.y)) * scale;

    min2D += translation;
    max2D += translation;

    ImVec2 imMin(min2D.x + window_pos.x, window_pos.y + min2D.y);
    ImVec2 imMax(max2D.x + window_pos.x, window_pos.y + max2D.y);

    draw_list->AddRect(imMin, imMax, IM_COL32(0, 0, 255, 255));

    for (const auto& child : node->children)
    {
        if (child)
        {
            DrawFrontNode(child.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
        }
    }
}

void Octree::DrawBackView(ImDrawList* draw_list, float scale, const ImVec2& window_size, const ImVec2& window_pos) const
{
    glm::vec3 origin = root->bounds.min;
    glm::vec3 size = root->bounds.max - root->bounds.min;

    // XY
    glm::vec3 octree_center = origin + size * 0.5f;
    glm::vec2 center_offset = glm::vec2(window_size.x, window_size.y) * 0.5f;
    glm::vec2 center_pos(octree_center.x, octree_center.y);

    glm::vec2 translation = center_offset - center_pos;

    DrawBackNode(root.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
}

void Octree::DrawBackNode(const OctreeNode* node, ImDrawList* draw_list, float scale, const ImVec2& window_size, const glm::vec3& origin, const glm::vec3& size, const ImVec2& window_pos, const glm::vec2& translation) const
{
    if (!node) return;

    // XY
    glm::vec2 min2D(node->bounds.min.x, node->bounds.min.y);
    glm::vec2 max2D(node->bounds.max.x, node->bounds.max.y);

    min2D = -min2D;
    max2D = -max2D;

    min2D = (min2D - glm::vec2(origin.x, origin.y)) * scale;
    max2D = (max2D - glm::vec2(origin.x, origin.y)) * scale;

    min2D += translation;
    max2D += translation;

    ImVec2 imMin(min2D.x + window_pos.x, window_pos.y + min2D.y);
    ImVec2 imMax(max2D.x + window_pos.x, window_pos.y + max2D.y);

    draw_list->AddRect(imMin, imMax, IM_COL32(255, 0, 255, 255));

    for (const auto& child : node->children)
    {
        if (child)
        {
            DrawBackNode(child.get(), draw_list, scale, window_size, origin, size, window_pos, translation);
        }
    }
}