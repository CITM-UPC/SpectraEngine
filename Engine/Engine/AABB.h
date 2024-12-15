#pragma once
#include <glm/vec3.hpp>
#include <algorithm>

class AABB {
public:
    glm::vec3 min;
    glm::vec3 max;

    // Constructor por defecto
    AABB();

    // Constructor con par�metros
    AABB(const glm::vec3& min, const glm::vec3& max);

    // M�todos
    bool Intersects(const AABB& other) const;
    void Update(const glm::vec3& point);

	//Draw the bounding box
	void DrawAABB() const;
};

