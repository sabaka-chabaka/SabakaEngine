#define NOMINMAX
#include "assets/AssetManager.h"
#include "io/SmeshFormat.h"
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace engine::assets {

    template<>
    std::shared_ptr<renderer::Mesh> AssetManager::loadFromDisk<renderer::Mesh>(
        const std::filesystem::path& path)
    {
        const std::string ext     = path.extension().string();
        const std::string pathStr = path.string();

        if (ext == ".smesh") {
            return std::make_shared<renderer::Mesh>(
                io::SmeshLoader::load(m_device, m_context, pathStr));
        }

        const std::string smeshPath = io::MeshImporter::smeshPathFor(pathStr);

        if (io::MeshImporter::needsReimport(pathStr, smeshPath))
            io::MeshImporter::importObj(pathStr, smeshPath);

        return std::make_shared<renderer::Mesh>(
            io::SmeshLoader::load(m_device, m_context, smeshPath));
    }

    template<>
    std::shared_ptr<renderer::Texture2D> AssetManager::loadFromDisk<renderer::Texture2D>(
        const std::filesystem::path& path)
    {
        return std::make_shared<renderer::Texture2D>(
            m_device, m_context, path.string());
    }

    template<typename T>
    AssetHandle<T> AssetManager::load(const std::filesystem::path& path)
    {
        const std::string key = normalize(path);

        {
            std::unique_lock lock(m_cacheMutex);
            auto it = m_cache.find(key);
            if (it != m_cache.end()) {
                auto typed = std::static_pointer_cast<T>(it->second.ptr);
                return AssetHandle<T>(it->second.id, typed);
            }
        }

        LOG_DEBUG("AssetManager: loading " + key);

        std::shared_ptr<T> ptr = loadFromDisk<T>(path);
        AssetId id = nextId();

        {
            std::unique_lock lock(m_cacheMutex);
            m_cache[key]   = Entry{ id, ptr };
            m_idToPath[id] = key;
        }

        return AssetHandle<T>(id, ptr);
    }

    template<>
    AsyncAssetHandle<renderer::Mesh> AssetManager::loadAsync<renderer::Mesh>(
        const std::filesystem::path& path)
    {
        const std::string key = normalize(path);

        {
            std::unique_lock lock(m_cacheMutex);
            auto it = m_cache.find(key);
            if (it != m_cache.end()) {
                auto typed = std::static_pointer_cast<renderer::Mesh>(it->second.ptr);
                AssetHandle<renderer::Mesh> handle(it->second.id, typed);
                std::promise<AssetHandle<renderer::Mesh>> p;
                p.set_value(handle);
                return AsyncAssetHandle<renderer::Mesh>(p.get_future());
            }
        }

        LOG_DEBUG("AssetManager: async loading " + key);

        auto pending = std::make_shared<PendingMesh>();
        pending->key = key;
        pending->id  = nextId();

        std::future<AssetHandle<renderer::Mesh>> fut = pending->promise.get_future();

        {
            std::unique_lock lock(m_pendingMutex);
            m_pendingUploads.push_back(pending);
        }

        const std::string pathStr = path.string();

        m_threadPool->submit([this, pending, pathStr] {
            loadAsyncMeshInternal(pending, pathStr);
        });

        return AsyncAssetHandle<renderer::Mesh>(std::move(fut));
    }

    void AssetManager::loadAsyncMeshInternal(std::shared_ptr<PendingMesh> pendingPtr, std::string pathStr)
    {
        try {
            const std::string ext       = std::filesystem::path(pathStr).extension().string();
            const std::string smeshPath = ext == ".smesh"
                ? pathStr
                : io::MeshImporter::smeshPathFor(pathStr);

            if (ext != ".smesh" && io::MeshImporter::needsReimport(pathStr, smeshPath))
                io::MeshImporter::importObj(pathStr, smeshPath);

            FILE* f = ::fopen(smeshPath.c_str(), "rb");
            if (!f) throw std::runtime_error("cannot open " + smeshPath);

            io::SmeshHeader myHdr;
            ::memset(&myHdr, 0, sizeof(myHdr));
            ::fread(&myHdr, sizeof(myHdr), 1, f);

            pendingPtr->vertices.resize(myHdr.vertexCount);
            pendingPtr->indices.resize(myHdr.indexCount);
            ::fread(pendingPtr->vertices.data(), sizeof(renderer::Vertex), myHdr.vertexCount, f);
            ::fread(pendingPtr->indices.data(),  sizeof(uint32_t),         myHdr.indexCount,  f);
            ::fclose(f);

            LOG_DEBUG("AssetManager: async disk read done for " + smeshPath);
        } catch (const std::exception& e) {
            pendingPtr->promise.set_exception(std::current_exception());
        }
    }

    void AssetManager::flushPendingUploads()
    {
        std::vector<std::shared_ptr<PendingMesh>> ready;

        {
            std::unique_lock lock(m_pendingMutex);
            std::vector<std::shared_ptr<PendingMesh>> remaining;
            for (auto& p : m_pendingUploads) {
                if (!p->vertices.empty())
                    ready.push_back(p);
                else
                    remaining.push_back(p);
            }
            m_pendingUploads = std::move(remaining);
        }

        for (auto& p : ready) {
            try {
                std::vector<unsigned int> indices(p->indices.begin(), p->indices.end());
                auto mesh = std::make_shared<renderer::Mesh>(
                    m_device, m_context, p->vertices, indices);

                {
                    std::unique_lock lock(m_cacheMutex);
                    m_cache[p->key]   = Entry{ p->id, mesh };
                    m_idToPath[p->id] = p->key;
                }

                p->promise.set_value(AssetHandle<renderer::Mesh>(p->id, mesh));
                LOG_DEBUG("AssetManager: GPU upload done for " + p->key);
            } catch (...) {
                p->promise.set_exception(std::current_exception());
            }
        }
    }

    template<typename T>
    AssetHandle<T> AssetManager::get(AssetId id) const
    {
        std::unique_lock lock(m_cacheMutex);

        auto pathIt = m_idToPath.find(id);
        if (pathIt == m_idToPath.end()) return {};

        auto entryIt = m_cache.find(pathIt->second);
        if (entryIt == m_cache.end()) return {};

        auto typed = std::static_pointer_cast<T>(entryIt->second.ptr);
        return AssetHandle<T>(id, typed);
    }

    void AssetManager::unload(AssetId id)
    {
        std::unique_lock lock(m_cacheMutex);

        auto pathIt = m_idToPath.find(id);
        if (pathIt == m_idToPath.end()) return;

        LOG_DEBUG("AssetManager: unloading id=" + std::to_string(id));
        m_cache.erase(pathIt->second);
        m_idToPath.erase(pathIt);
    }

    void AssetManager::unloadUnused()
    {
        std::unique_lock lock(m_cacheMutex);
        std::vector<std::string> toRemove;

        for (auto& [key, entry] : m_cache) {
            if (entry.ptr.use_count() == 1)
                toRemove.push_back(key);
        }

        for (const auto& key : toRemove) {
            auto id = m_cache[key].id;
            LOG_DEBUG("AssetManager: unloading unused id=" + std::to_string(id) + " path=" + key);
            m_idToPath.erase(id);
            m_cache.erase(key);
        }
    }

    bool AssetManager::isLoaded(AssetId id) const
    {
        std::unique_lock lock(m_cacheMutex);
        return m_idToPath.count(id) > 0;
    }

    bool AssetManager::isLoaded(const std::filesystem::path& path) const
    {
        std::unique_lock lock(m_cacheMutex);
        return m_cache.count(normalize(path)) > 0;
    }

    AssetId AssetManager::idForPath(const std::filesystem::path& path) const
    {
        std::unique_lock lock(m_cacheMutex);
        const std::string key = normalize(path);
        auto it = m_cache.find(key);
        if (it == m_cache.end()) return INVALID_ASSET_ID;
        return it->second.id;
    }

    template AssetHandle<renderer::Mesh>      AssetManager::load<renderer::Mesh>(const std::filesystem::path&);
    template AssetHandle<renderer::Texture2D> AssetManager::load<renderer::Texture2D>(const std::filesystem::path&);
    template AssetHandle<renderer::Mesh>      AssetManager::get<renderer::Mesh>(AssetId) const;
    template AssetHandle<renderer::Texture2D> AssetManager::get<renderer::Texture2D>(AssetId) const;

}