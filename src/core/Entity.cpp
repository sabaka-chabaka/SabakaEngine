#include "Entity.h"
#include <atomic>

namespace engine::core {
    static std::atomic<uint64_t> s_nextId = 0;

    uint64_t Entity::generateId() {
        return s_nextId.fetch_add(1, std::memory_order_relaxed);
    }

    Entity::Entity(std::string name) : m_id(generateId()), m_name(std::move(name)){}

    uint64_t Entity::getId() const {
        return m_id;
    }

    const std::string& Entity::getName() const {
        return m_name;
    }

    void Entity::setName(std::string name) {
        m_name = std::move(name);
    }

    void Entity::onUpdate(float deltaTime) {
        for (auto& [key, comp] : m_components) {
            comp->onUpdate(deltaTime);
        }
    }

    void Entity::onRender() {
        for (auto& [key, comp] : m_components) {
            comp->onRender();
        }
    }
}
