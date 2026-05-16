#pragma once
#include <windows.h>
#include <string>
#include <functional>

namespace engine::platform {
    struct WindowDesc {
        std::wstring title = L"Engine";
        int width = 1280;
        int height = 720;
    };

    class Window {
    public:
        explicit Window(const WindowDesc& desc);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        bool processMessages();

        HWND getNativeHandle() const;
        int getWidth() const;
        int getHeight() const;

        void setResizeCallback(std::function<void(int, int)> callback);

    private:
        static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        HWND m_hwnd = nullptr;
        HINSTANCE m_instance = nullptr;
        int m_width = 0;
        int m_height = 0;
        std::function<void(int, int)> m_resizeCallback;
        bool m_initialized = false;
    };
}