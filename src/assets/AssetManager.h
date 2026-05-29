#pragma once
#include "assets/AssetId.h"
#include "assets/AssetHandle.h"
#include "assets/AsyncAssetHandle.h"
#include "renderer/Mesh.h"
#include "renderer/Texture2D.h"
#include "renderer/Material.h"
#include "io/SmeshLoader.h"
#include "io/MeshImporter.h"
#include "core/Logger.h"
#include "core/ThreadPool.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <atomic>
#include <filesystem>
#include <future>
#include <mutex>
#include <vector>
#include <d3d11.h>

#include "../core/ThreadPool.h"

namespace engine::assets {

    class AssetManager {
    public:
        explicit AssetManager(ID3D11Device* device, ID3D11DeviceContext* context,
                              size_t threadCount = 0)
            : m_device(device), m_context(context)
            , m_threadPool(std::make_unique<ThreadPool>(threadCount)) {}

        ~AssetManager() = default;

        template<typename T>
        AssetHandle<T> load(const std::filesystem::path& path);

        template<typename T>
        AsyncAssetHandle<T> loadAsync(const std::filesystem::path& path);

        template<typename T>
        AssetHandle<T> get(AssetId id) const;

        void flushPendingUploads();

        void unload(AssetId id);
        void unloadUnused();

        bool isLoaded(AssetId id)                        const;
        bool isLoaded(const std::filesystem::path& path) const;

        AssetId idForPath(const std::filesystem::path& path) const;

    private:
        struct Entry {
            AssetId               id;
            std::shared_ptr<void> ptr;
        };

        struct PendingMesh {
            std::string                          key;
            AssetId                              id;
            std::vector<renderer::Vertex>        vertices;
            std::vector<uint32_t>                indices;
            std::promise<AssetHandle<renderer::Mesh>> promise;
        };

        static AssetId nextId() {
            static std::atomic<uint64_t> s_counter{ 1 };
            return s_counter.fetch_add(1, std::memory_order_relaxed);
        }

        static std::string normalize(const std::filesystem::path& p) {
            return std::filesystem::weakly_canonical(p).string();
        }

        template<typename T>
        std::shared_ptr<T> loadFromDisk(const std::filesystem::path& path);

        void loadAsyncMeshInternal(std::shared_ptr<PendingMesh> pending, std::string pathStr);

        ID3D11Device*                    m_device;
        ID3D11DeviceContext*             m_context;
        std::unique_ptr<ThreadPool>      m_threadPool;

        mutable std::mutex                               m_cacheMutex;
        std::unordered_map<std::string, Entry>           m_cache;
        std::unordered_map<AssetId, std::string>         m_idToPath;

        std::mutex                                       m_pendingMutex;
        std::vector<std::shared_ptr<PendingMesh>>        m_pendingUploads;
    };

}
