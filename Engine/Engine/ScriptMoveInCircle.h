#pragma once

#include <glm/vec3.hpp>

#include "ComponentScript.h"

class ScriptMoveInCircle : public ComponentScript
{
public:
	ScriptMoveInCircle(GameObject* gameObject);
	virtual ~ScriptMoveInCircle();

	void Start() override;
	void Reset() override;
	void Update() override;

private:
	float speed;
	float radius;
	float angle;
	glm::vec3 initialPosition;
};