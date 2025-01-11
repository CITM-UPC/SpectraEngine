#include "ComponentScript.h"

#include "GameObject.h"
#include "ScriptMoveInCircle.h"


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

void ComponentScript::Serialize(nlohmann::json& json) const
{
	Component::Serialize(json);

    json["scriptType"] = GetScriptTypeName();

    for (const auto& var : exposedVariables)
    {
        nlohmann::json varJson;
        varJson["name"] = var.name;
        varJson["type"] = static_cast<uint8_t>(var.type);
        switch (var.type)
        {
        case ScriptVariable::Type::FLOAT:
            varJson["value"] = *static_cast<float*>(var.ptr);
            varJson["min"] = var.limits.f.min;
            varJson["max"] = var.limits.f.max;
            break;
        case ScriptVariable::Type::INT:
            varJson["value"] = *static_cast<int*>(var.ptr);
            varJson["min"] = var.limits.i.min;
            varJson["max"] = var.limits.i.max;
            break;
        case ScriptVariable::Type::BOOL:
            varJson["value"] = *static_cast<bool*>(var.ptr);
            break;
        case ScriptVariable::Type::STRING:
            varJson["value"] = *static_cast<std::string*>(var.ptr);
            break;
        }
        json["exposedVariables"].push_back(varJson);
    }
}

void ComponentScript::Deserialize(const nlohmann::json& json)
{
	Component::Deserialize(json);

    for (const auto& varJson : json["exposedVariables"])
    {
        std::string name = varJson["name"];
        ScriptVariable::Type type = static_cast<ScriptVariable::Type>(varJson["type"].get<uint8_t>());
        switch (type)
        {
        case ScriptVariable::Type::FLOAT:
        {
            float value = varJson["value"];
            float min = varJson["min"];
            float max = varJson["max"];
            ExposeFloat(name.c_str(), new float(value), min, max);
            break;
        }
        case ScriptVariable::Type::INT:
        {
            int value = varJson["value"];
            int min = varJson["min"];
            int max = varJson["max"];
            ExposeInt(name.c_str(), new int(value), min, max);
            break;
        }
        case ScriptVariable::Type::BOOL:
        {
            bool value = varJson["value"];
            ExposeBool(name.c_str(), new bool(value));
            break;
        }
        case ScriptVariable::Type::STRING:
        {
            std::string value = varJson["value"];
            ExposeString(name.c_str(), new std::string(value));
            break;
        }
        }
    }
}

const char* ComponentScript::GetScriptTypeName() const
{
    const char* fullName = typeid(*this).name();
    const char* prefix = "class ";

    if (std::strncmp(fullName, prefix, std::strlen(prefix)) == 0) {
        return fullName + std::strlen(prefix);
    }
    return fullName;
}

ComponentScript* ComponentScript::CreateScriptByType(const char* typeName, GameObject* gameObject)
{
    if (strcmp(typeName, "ScriptMoveInCircle") == 0)
        return new ScriptMoveInCircle(gameObject);

    return new ComponentScript(gameObject);
}
