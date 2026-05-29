#pragma once
#include <string>

namespace engine::io {
    class MeshImporter {
    public:
        static void importObj(const std::string& objPath, const std::string& smeshPath);

        static std::string smeshPathFor(const std::string& srcPath);

        static bool needsReimport(const std::string& objPath, const std::string& smeshPath);
    };
}
