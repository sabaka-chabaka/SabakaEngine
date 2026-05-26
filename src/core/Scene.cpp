#include "Scene.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "SceneNode.h"
#include "Logger.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

namespace engine::core {

    Entity* Scene::createEntity(std::string name) {
        auto entity = std::make_unique<Entity>(std::move(name));
        Entity* raw = entity.get();
        m_entities.push_back(std::move(entity));
        return raw;
    }

    void Scene::destroyEntity(uint64_t id) {
        m_pendingDestroy.insert(id);
    }

    void Scene::destroyEntity(Entity* entity) {
        if (entity) destroyEntity(entity->getId());
    }

    Entity* Scene::findById(uint64_t id) const {
        for (auto& e : m_entities) {
            if (e->getId() == id) return e.get();
        }
        return nullptr;
    }

    Entity* Scene::findByName(const std::string& name) const {
        for (auto& e : m_entities) {
            if (e->getName() == name) return e.get();
        }
        return nullptr;
    }

    void Scene::update(float deltaTime) {
        flushPendingDestroy();
        for (auto& e : m_entities) {
            e->onUpdate(deltaTime);
        }
    }

    void Scene::render() {
        for (auto& e : m_entities) {
            e->onRender();
        }
    }

    void Scene::renderDepthOnly(renderer::ConstantBuffer<renderer::TransformData>* transformCB, const DirectX::XMMATRIX& lightSpaceMatrix) {
        for (auto& e : m_entities) {
            auto* mr = e->getComponent<MeshRenderer>();
            if (!mr || !mr->getMesh()) continue;

            XMMATRIX world = XMMatrixIdentity();
            if (auto* node = e->getComponent<SceneNode>())
                world = node->getWorldMatrix();
            else if (auto* t = e->getComponent<Transform>())
                world = t->getWorldMatrix();

            renderer::TransformData td = {};
            td.model        = XMMatrixTranspose(world);
            td.view         = lightSpaceMatrix; // Already transposed in ShadowPass
            td.projection   = XMMatrixIdentity();
            td.normalMatrix = XMMatrixIdentity();

            transformCB->update(td);
            transformCB->bindVS(0);

            mr->getMesh()->draw();
        }
    }

    const std::vector<std::unique_ptr<Entity>>& Scene::getEntities() const {
        return m_entities;
    }

    void Scene::flushPendingDestroy() {
        if (m_pendingDestroy.empty()) return;

        m_entities.erase(
            std::remove_if(m_entities.begin(), m_entities.end(),
                [&](const std::unique_ptr<Entity>& e) {
                    return m_pendingDestroy.count(e->getId()) > 0;
                }),
            m_entities.end()
        );

        m_pendingDestroy.clear();
    }
}