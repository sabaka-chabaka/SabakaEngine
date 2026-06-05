#pragma once
#include <string>

namespace engine::core  { class Scene; }
namespace engine::editor { class EditorApplication; }

namespace engine::editor {

    class SceneSerializer {
    public:
        static bool saveToFile(const core::Scene* scene, const std::string& path);
        static bool loadFromFile(EditorApplication* app,  const std::string& path);

        static std::string  serializeToString(const core::Scene* scene);
        static bool         deserializeFromString(EditorApplication* app,
                                                  const std::string& json);
    };

}
