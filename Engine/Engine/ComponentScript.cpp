#include "ComponentScript.h"

#include "GameObject.h"


ComponentScript::ComponentScript(GameObject* gameObject)
	: Component(gameObject, ComponentType::SCRIPT), initialTransform(glm::mat4(1.0f))
{
}

ComponentScript::~ComponentScript()
{
}

void ComponentScript::Init()
{
    initialTransform = gameObject->transform->localTransform;
}

void ComponentScript::Awake()
{
}

void ComponentScript::Start()
{
}

void ComponentScript::Reset()
{
	glm::vec3 position, scale;
	glm::quat rotation;
    gameObject->transform->Decompose(initialTransform, position, rotation, scale);
    gameObject->transform->SetTransformMatrix(position, rotation, scale, gameObject->parent->transform);
	gameObject->transform->UpdateTransform();
}

void ComponentScript::Update()
{
}

void ComponentScript::OnEditor()
{
    if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto& var : exposedVariables)
        {
            switch (var.type)
        	{
            case ScriptVariable::Type::FLOAT:
                ImGui::SliderFloat(var.name.c_str(), static_cast<float*>(var.ptr), var.limits.f.min, var.limits.f.max, "%.1f");
                break;
            case ScriptVariable::Type::INT:
                ImGui::SliderInt(var.name.c_str(), static_cast<int*>(var.ptr), var.limits.i.min, var.limits.i.max);
                break;
            case ScriptVariable::Type::BOOL:
                ImGui::Checkbox(var.name.c_str(), static_cast<bool*>(var.ptr));
                break;
            case ScriptVariable::Type::STRING:
                ImGui::InputText(var.name.c_str(), static_cast<std::string*>(var.ptr)->data(), static_cast<std::string*>(var.ptr)->capacity());
                break;
            }
        }
    }
}

void ComponentScript::ExposeFloat(const char* name, float* value, float min, float max)
{
    ScriptVariable var;
    var.name = name;
    var.ptr = value;
    var.type = ScriptVariable::Type::FLOAT;
    var.limits.f = { min, max };
    exposedVariables.push_back(var);
}

void ComponentScript::ExposeInt(const char* name, int* value, int min, int max)
{
    ScriptVariable var;
    var.name = name;
    var.ptr = value;
    var.type = ScriptVariable::Type::INT;
    var.limits.i = { min, max };
    exposedVariables.push_back(var);
}

void ComponentScript::ExposeBool(const char* name, bool* value)
{
    exposedVariables.push_back({ name, value, ScriptVariable::Type::BOOL });
}

void ComponentScript::ExposeString(const char* name, std::string* value)
{
    exposedVariables.push_back({ name, value, ScriptVariable::Type::STRING });
}
