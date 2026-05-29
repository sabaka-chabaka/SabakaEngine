#include "assets/AssetManager.h"

namespace engine::assets {

    template<>
    std::shared_ptr<renderer::Mesh> AssetManager::loadFromDisk<renderer::Mesh>(
        const std::filesystem::path& path)
    {
        auto mesh = std::make_shared<renderer::Mesh>(
            io::ObjLoader::load(m_device, m_context, path.string()));
        return mesh;
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

        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            auto typed = std::static_pointer_cast<T>(it->second.ptr);
            return AssetHandle<T>(it->second.id, typed);
        }

        LOG_DEBUG("AssetManager: loading " + key);

        std::shared_ptr<T> ptr = loadFromDisk<T>(path);

        AssetId id = nextId();
        m_cache[key]    = Entry{ id, ptr };
        m_idToPath[id]  = key;

        return AssetHandle<T>(id, ptr);
    }

    template<typename T>
    AssetHandle<T> AssetManager::get(AssetId id) const
    {
        auto pathIt = m_idToPath.find(id);
        if (pathIt == m_idToPath.end())
            return {};

        auto entryIt = m_cache.find(pathIt->second);
        if (entryIt == m_cache.end())
            return {};

        auto typed = std::static_pointer_cast<T>(entryIt->second.ptr);
        return AssetHandle<T>(id, typed);
    }

    void AssetManager::unload(AssetId id)
    {
        auto pathIt = m_idToPath.find(id);
        if (pathIt == m_idToPath.end())
            return;

        LOG_DEBUG("AssetManager: unloading id=" + std::to_string(id));

        m_cache.erase(pathIt->second);
        m_idToPath.erase(pathIt);
    }

    void AssetManager::unloadUnused()
    {
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
        return m_idToPath.count(id) > 0;
    }

    bool AssetManager::isLoaded(const std::filesystem::path& path) const
    {
        return m_cache.count(normalize(path)) > 0;
    }

    AssetId AssetManager::idForPath(const std::filesystem::path& path) const
    {
        const std::string key = normalize(path);
        auto it = m_cache.find(key);
        if (it == m_cache.end())
            return INVALID_ASSET_ID;
        return it->second.id;
    }

    template AssetHandle<renderer::Mesh>     AssetManager::load<renderer::Mesh>(const std::filesystem::path&);
    template AssetHandle<renderer::Texture2D> AssetManager::load<renderer::Texture2D>(const std::filesystem::path&);
    template AssetHandle<renderer::Mesh>     AssetManager::get<renderer::Mesh>(AssetId) const;
    template AssetHandle<renderer::Texture2D> AssetManager::get<renderer::Texture2D>(AssetId) const;

}