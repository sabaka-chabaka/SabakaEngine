#pragma once
#include "Entity.h"
#include "renderer/ConstantBuffer.h"
#include "renderer/TransformData.h"
#include "renderer/Camera.h"
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace engine::core {

    class Scene {
    public:
        Scene()  = default;
        ~Scene() = default;

        Scene(const Scene&)            = delete;
        Scene& operator=(const Scene&) = delete;

        Entity* createEntity(std::string name = "Entity");
        void    destroyEntity(uint64_t id);
        void    destroyEntity(Entity* entity);

        Entity* findById(uint64_t id)              const;
        Entity* findByName(const std::string& name) const;

        void update(float deltaTime);
        void render();
        void renderDepthOnly(renderer::ConstantBuffer<renderer::TransformData>* transformCB, const DirectX::XMMATRIX& lightSpaceMatrix);

        const std::vector<std::unique_ptr<Entity>>& getEntities() const;

    private:
        void flushPendingDestroy();

        std::vector<std::unique_ptr<Entity>> m_entities;
        std::unordered_set<uint64_t>         m_pendingDestroy;
    };
}