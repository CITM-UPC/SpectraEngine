#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Component.h"

class ComponentScript : public Component
{
public:
	ComponentScript(GameObject* gameObject);
	virtual ~ComponentScript();

    void Init();

    virtual void Awake();
    virtual void Start();
    virtual void Reset();

	void Update() override;
	void OnEditor() override;

    void Serialize(nlohmann::json& json) const override;
    void Deserialize(const nlohmann::json& json) override;

    const char* GetScriptTypeName() const;
    static ComponentScript* CreateScriptByType(const char* typeName, GameObject* gameObject);

protected:
    struct ScriptVariable {
        std::string name;
        void* ptr = nullptr;
        enum class Type : uint8_t { FLOAT, INT, BOOL, STRING } type = Type::FLOAT;
        union {
            struct { float min; float max; } f;
            struct { int min; int max; } i;
        } limits{ { 0.0f, 10.0f } };
    };
    std::vector<ScriptVariable> exposedVariables;

    void ExposeFloat(const char* name, float* value, float min = 0.0f, float max = 10.0f);
    void ExposeInt(const char* name, int* value, int min = 0, int max = 10);
    void ExposeBool(const char* name, bool* value);
    void ExposeString(const char* name, std::string* value);

private:
    glm::mat4 initialTransform;
};
