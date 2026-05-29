#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <mutex>

namespace engine {

    class VFS {
    public:
        static VFS& get() {
            static VFS instance;
            return instance;
        }

        void mount(const std::string& alias, const std::filesystem::path& realPath) {
            std::unique_lock lock(m_mutex);
            m_mounts[normalizeAlias(alias)] = std::filesystem::weakly_canonical(realPath).string();
        }

        void unmount(const std::string& alias) {
            std::unique_lock lock(m_mutex);
            m_mounts.erase(normalizeAlias(alias));
        }

        std::string resolve(const std::string& vfsPath) const {
            std::unique_lock lock(m_mutex);

            const auto sep = vfsPath.find("://");
            if (sep == std::string::npos)
                return vfsPath;

            const std::string alias    = vfsPath.substr(0, sep);
            const std::string relative = vfsPath.substr(sep + 3);

            auto it = m_mounts.find(alias);
            if (it == m_mounts.end())
                throw std::runtime_error("VFS: unknown alias '" + alias + "'");

            return (std::filesystem::path(it->second) / relative).string();
        }

        bool isMounted(const std::string& alias) const {
            std::unique_lock lock(m_mutex);
            return m_mounts.count(normalizeAlias(alias)) > 0;
        }

        bool isVfsPath(const std::string& path) const {
            return path.find("://") != std::string::npos;
        }

    private:
        VFS() = default;

        static std::string normalizeAlias(const std::string& alias) {
            std::string a = alias;
            if (a.size() >= 3 && a.substr(a.size() - 3) == "://")
                a = a.substr(0, a.size() - 3);
            return a;
        }

        mutable std::mutex                         m_mutex;
        std::unordered_map<std::string, std::string> m_mounts;
    };

}