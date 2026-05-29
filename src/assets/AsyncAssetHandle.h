#pragma once
#include "assets/AssetHandle.h"
#include <future>
#include <chrono>

namespace engine::assets {

    template<typename T>
    class AsyncAssetHandle {
    public:
        AsyncAssetHandle() = default;

        explicit AsyncAssetHandle(std::future<AssetHandle<T>> fut)
            : m_future(std::move(fut)) {}

        bool isReady() const {
            if (!m_future.valid()) return false;
            return m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }

        bool isValid() const {
            return m_future.valid();
        }

        AssetHandle<T> get() {
            if (!m_future.valid()) return {};
            return m_future.get();
        }

        AssetHandle<T> tryGet() {
            if (!isReady()) return {};
            return m_future.get();
        }

    private:
        mutable std::future<AssetHandle<T>> m_future;
    };
}