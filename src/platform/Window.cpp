#include "Window.h"

#include <stdexcept>

#include "core/Logger.h"

namespace engine::platform {
    static const wchar_t* CLASS_NAME  = L"GameEngineWindowClass";

    Window::Window(const WindowDesc &desc) : m_width(desc.width), m_height(desc.height) {
        m_instance = GetModuleHandleW(nullptr);

        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = wndProc;
        wc.hInstance = m_instance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = CLASS_NAME;

        if (!RegisterClassExW(&wc)) {
            throw std::runtime_error("Failed to register window class");
        }

        RECT rect = { 0, 0, m_width, m_height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        desc.title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right  - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        m_instance,
        this
    );

        if (!m_hwnd) {
            throw std::runtime_error("Failed to create window");
        }

        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        m_initialized = true;
    }

    Window::~Window() {
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
        }
        UnregisterClassW(CLASS_NAME, m_instance);
    }

    bool Window::processMessages() {
        MSG msg = {};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return true;
    }

    HWND  Window::getNativeHandle() const { return m_hwnd; }
    int   Window::getWidth()        const { return m_width; }
    int   Window::getHeight()       const { return m_height; }

    void Window::setResizeCallback(std::function<void(int, int)> callback) {
        m_resizeCallback = std::move(callback);
    }

    LRESULT CALLBACK Window::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Window* self = nullptr;
        if (msg == WM_NCCREATE) {
            self = reinterpret_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (self) {
            switch (msg) {
                case WM_SIZE:
                    self->m_width  = LOWORD(lParam);
                    self->m_height = HIWORD(lParam);
                    if (self->m_initialized && self->m_resizeCallback) {
                        self->m_resizeCallback(self->m_width, self->m_height);
                    }
                    return 0;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
            }
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
