#pragma once

namespace engine::core {
    class Component {
    public:
        virtual ~Component() = default;

        virtual void onUpdate(float deltaTime) {}
        virtual void onRender() {}

        class Entity* owner = nullptr;
    };
}