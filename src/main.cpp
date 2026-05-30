#include "runtime/EngineRuntime.h"
#include <Windows.h>

int WINAPI WinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPSTR     lpCmdLine,
    _In_     int       nShowCmd) {
    try {
        engine::runtime::EngineRuntime app;
        return app.run();
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}