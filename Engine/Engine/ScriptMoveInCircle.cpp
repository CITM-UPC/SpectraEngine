#include "ScriptMoveInCircle.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "Time.h"
#include "App.h"

ScriptMoveInCircle::ScriptMoveInCircle(GameObject* gameObject)
	: ComponentScript(gameObject), speed(1.0f), radius(1.0f), angle(0.0f)
{
    initialPosition = gameObject->transform->position;

    ExposeFloat("Speed", &speed, 1.0f, 10.0f);
    ExposeFloat("Radius", &radius, 1.0f, 10.0f);
}

ScriptMoveInCircle::~ScriptMoveInCircle()
{
}

void ScriptMoveInCircle::Start()
{
    angle = 0.0f;
    initialPosition = gameObject->transform->position;
}

void ScriptMoveInCircle::Reset()
{
	ComponentScript::Reset();
    Start();
}

void ScriptMoveInCircle::Update()
{
    float angularSpeed = (speed / 1000.0f) / radius;

    angle += angularSpeed * app->time.GetDeltaTime();

    float x = initialPosition.x + radius * cos(angle);
    float z = initialPosition.z + radius * sin(angle);

    gameObject->transform->position = glm::vec3(x, initialPosition.y, z);
    gameObject->transform->UpdateTransform();
}