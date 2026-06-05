#define NOMINMAX
#include "editor/SceneSerializer.h"
#include "editor/EditorApplication.h"
#include "core/Scene.h"
#include "core/Entity.h"
#include "core/Transform.h"
#include "core/MeshRenderer.h"
#include "core/BoundingBoxComponent.h"
#include "math/AABB.h"
#include "core/Logger.h"
#include <json.hpp>
#include <fstream>
#include <sstream>

using json = nlohmann::json;
using namespace DirectX;

namespace engine::editor {

    static json serializeVec3(const XMFLOAT3& v) {
        return { v.x, v.y, v.z };
    }

    static json serializeVec4(const XMFLOAT4& v) {
        return { v.x, v.y, v.z, v.w };
    }

    static XMFLOAT3 deserializeVec3(const json& j) {
        return { j[0].get<float>(), j[1].get<float>(), j[2].get<float>() };
    }

    static XMFLOAT4 deserializeVec4(const json& j) {
        return { j[0].get<float>(), j[1].get<float>(),
                 j[2].get<float>(), j[3].get<float>() };
    }

    static json serializeEntity(const core::Entity* e) {
        json obj;
        obj["id"]   = e->getId();
        obj["name"] = e->getName();

        if (auto* t = e->getComponent<core::Transform>()) {
            json tr;
            tr["position"] = serializeVec3(t->getPosition());
            tr["rotation"] = serializeVec4(t->getRotationQuat());
            tr["scale"]    = serializeVec3(t->getScale());
            obj["transform"] = tr;
        }

        if (e->getComponent<core::MeshRenderer>()) {
            obj["meshRenderer"] = { {"mesh", "cube"} };
        }

        if (auto* bb = e->getComponent<core::BoundingBoxComponent>()) {
            const auto& aabb = bb->getLocalAABB();
            obj["boundingBox"] = {
                {"min", serializeVec3(aabb.min)},
                {"max", serializeVec3(aabb.max)}
            };
        }

        return obj;
    }

    std::string SceneSerializer::serializeToString(const core::Scene* scene) {
        json root;
        json entities = json::array();

        for (const auto& e : scene->getEntities()) {
            entities.push_back(serializeEntity(e.get()));
        }

        root["version"]  = 1;
        root["entities"] = entities;
        return root.dump(2);
    }

    bool SceneSerializer::saveToFile(const core::Scene* scene, const std::string& path) {
        std::ofstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("[SceneSerializer] cannot open for write: " + path);
            return false;
        }
        file << serializeToString(scene);
        LOG_INFO("[SceneSerializer] saved: " + path);
        return true;
    }

    bool SceneSerializer::deserializeFromString(EditorApplication* app,
                                                const std::string& jsonStr) {
        try {
            json root = json::parse(jsonStr);

            for (const auto& obj : root["entities"]) {
                std::string name = obj.value("name", "Entity");

                core::Entity* e = nullptr;

                if (obj.contains("meshRenderer")) {
                    e = app->createCube(name);
                } else {
                    e = app->createEmpty(name);
                }

                if (obj.contains("transform")) {
                    auto* t   = e->getComponent<core::Transform>();
                    const auto& tr = obj["transform"];
                    if (t) {
                        t->setPosition(deserializeVec3(tr["position"]));
                        t->setRotationQuat(deserializeVec4(tr["rotation"]));
                        t->setScale(deserializeVec3(tr["scale"]));
                    }
                }

                if (obj.contains("boundingBox") && e->getComponent<core::BoundingBoxComponent>()) {
                    auto* bb = e->getComponent<core::BoundingBoxComponent>();
                    math::AABB aabb;
                    aabb.min = deserializeVec3(obj["boundingBox"]["min"]);
                    aabb.max = deserializeVec3(obj["boundingBox"]["max"]);
                    bb->setLocalAABB(aabb);
                }
            }

            return true;
        }
        catch (const std::exception& ex) {
            LOG_ERROR(std::string("[SceneSerializer] parse error: ") + ex.what());
            return false;
        }
    }

    bool SceneSerializer::loadFromFile(EditorApplication* app, const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("[SceneSerializer] cannot open: " + path);
            return false;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        bool ok = deserializeFromString(app, ss.str());
        if (ok) LOG_INFO("[SceneSerializer] loaded: " + path);
        return ok;
    }

}
