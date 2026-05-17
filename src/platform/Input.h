#pragma once
#include <windows.h>
#include <array>
#include <DirectXMath.h>

namespace engine::platform {
    enum class Key : unsigned int {
        None = 0,

        A = 'A', B = 'B', C = 'C', D = 'D', E = 'E',
        F = 'F', G = 'G', H = 'H', I = 'I', J = 'J',
        K = 'K', L = 'L', M = 'M', N = 'N', O = 'O',
        P = 'P', Q = 'Q', R = 'R', S = 'S', T = 'T',
        U = 'U', V = 'V', W = 'W', X = 'X', Y = 'Y',
        Z = 'Z',

        Alpha0 = '0', Alpha1 = '1', Alpha2 = '2',
        Alpha3 = '3', Alpha4 = '4', Alpha5 = '5',
        Alpha6 = '6', Alpha7 = '7', Alpha8 = '8',
        Alpha9 = '9',

        Space     = VK_SPACE,
        Enter     = VK_RETURN,
        Escape    = VK_ESCAPE,
        Tab       = VK_TAB,
        Backspace = VK_BACK,
        Delete    = VK_DELETE,

        Left  = VK_LEFT,
        Right = VK_RIGHT,
        Up    = VK_UP,
        Down  = VK_DOWN,

        Shift   = VK_SHIFT,
        Control = VK_CONTROL,
        Alt     = VK_MENU,

        F1  = VK_F1,  F2  = VK_F2,  F3  = VK_F3,  F4  = VK_F4,
        F5  = VK_F5,  F6  = VK_F6,  F7  = VK_F7,  F8  = VK_F8,
        F9  = VK_F9,  F10 = VK_F10, F11 = VK_F11, F12 = VK_F12,
    };

    enum class MouseButton : unsigned int {
        Left   = 0,
        Right  = 1,
        Middle = 2,
    };

    class InputSystem {
    public:
        static InputSystem& get();

        InputSystem(const InputSystem&)            = delete;
        InputSystem& operator=(const InputSystem&) = delete;

        void initialize(HWND hwnd);
        void processRawInput(LPARAM lParam);
        void endFrame();

        bool isKeyDown    (Key key) const;
        bool isKeyPressed (Key key) const;
        bool isKeyReleased(Key key) const;

        bool isMouseDown    (MouseButton btn) const;
        bool isMousePressed (MouseButton btn) const;
        bool isMouseReleased(MouseButton btn) const;

        DirectX::XMFLOAT2 getMouseDelta()    const;
        DirectX::XMFLOAT2 getMousePosition() const;

        void setMouseCaptured(bool captured);
        bool isMouseCaptured() const;

        void onKillFocus();

    private:
        InputSystem() = default;

        static constexpr unsigned int KEY_COUNT    = 256;
        static constexpr unsigned int BUTTON_COUNT = 3;

        std::array<bool, KEY_COUNT>    m_currKeys = {};
        std::array<bool, KEY_COUNT>    m_prevKeys = {};
        std::array<bool, BUTTON_COUNT> m_currBtns = {};
        std::array<bool, BUTTON_COUNT> m_prevBtns = {};

        float m_deltaX    = 0.0f;
        float m_deltaY    = 0.0f;
        float m_mousePosX = 0.0f;
        float m_mousePosY = 0.0f;

        bool m_captured = false;
        HWND m_hwnd     = nullptr;

        void applyCursorCapture();
        void releaseCursorCapture();
    };
}