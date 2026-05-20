#pragma once
#include "core/Component.h"
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace engine::core {
    class Entity {
    public:
        explicit Entity(std::string name);

        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        uint64_t getId() const;
        const std::string& getName() const;
        void setName(std::string name);

        template<typename T, typename... Args>
        T* addComponent(Args&&... args) {
            auto key = std::type_index(typeid(T));
            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            comp->owner = this;
            T* raw = comp.get();
            m_components[key] = std::move(comp);
            return raw;
        }

        template<typename T>
        T* getComponent() const {
            auto it = m_components.find(std::type_index(typeid(T)));
            if (it == m_components.end()) return nullptr;
            return static_cast<T*>(it->second.get());
        }

        template<typename T>
        bool hasComponent() const {
            return m_components.count(std::type_index(typeid(T))) > 0;
        }

        template<typename T>
        void removeComponent() {
            m_components.erase(std::type_index(typeid(T)));
        }

        void onUpdate(float deltaTime);
        void onRender();

    private:
        static uint64_t generateId();

        uint64_t    m_id;
        std::string m_name;

        std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
    };
}
