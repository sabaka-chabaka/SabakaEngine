#pragma once
#include "assets/AssetId.h"
#include <memory>

namespace engine::assets {

    template<typename T>
    class AssetHandle {
    public:
        AssetHandle() = default;

        AssetHandle(AssetId id, std::weak_ptr<T> ptr)
            : m_id(id), m_ptr(std::move(ptr)) {}

        AssetId getId() const {
            return m_id;
        }

        bool isValid() const {
            return assets::isValid(m_id) && !m_ptr.expired();
        }

        std::shared_ptr<T> lock() const {
            return m_ptr.lock();
        }

        void reset() {
            m_id = INVALID_ASSET_ID;
            m_ptr.reset();
        }

        bool operator==(const AssetHandle<T>& other) const {
            return m_id == other.m_id;
        }

        bool operator!=(const AssetHandle<T>& other) const {
            return m_id != other.m_id;
        }

        explicit operator bool() const {
            return isValid();
        }

    private:
        AssetId            m_id  = INVALID_ASSET_ID;
        std::weak_ptr<T>   m_ptr;
    };
}