#pragma once

namespace engine::editor {

    enum class GizmoMode {
        None,
        Translate,
        Rotate,
        Scale
    };

    enum class GizmoAxis {
        None,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ,
        XYZ
    };

}
