#include "Input.h"
#include <vector>
#include "core/Logger.h"

namespace engine::platform {
    InputSystem &InputSystem::get() {
        static InputSystem instance;
        return instance;
    }

    void InputSystem::initialize(HWND hwnd) {
        m_hwnd = hwnd;

        RAWINPUTDEVICE devices[2] = {};

        devices[0].usUsagePage = 0x01;
        devices[0].usUsage = 0x06;
        devices[0].dwFlags = RIDEV_NOLEGACY;
        devices[0].hwndTarget = hwnd;

        devices[1].usUsagePage = 0x01;
        devices[1].usUsage = 0x02;
        devices[1].dwFlags = 0;
        devices[1].hwndTarget = hwnd;

        if (!RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE))) {
            throw std::runtime_error("Failed to register raw input devices");
        }

        LOG_INFO("InputSystem initialized (RawInput)");
    }

    void InputSystem::processRawInput(LPARAM lParam) {
        UINT size = 0;
        GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            nullptr,
            &size,
            sizeof(RAWINPUTHEADER)
        );

        if (size == 0) return;

        static std::vector<BYTE> buffer;
        buffer.resize(size);

        if (GetRawInputData(
                reinterpret_cast<HRAWINPUT>(lParam),
                RID_INPUT,
                buffer.data(),
                &size,
                sizeof(RAWINPUTHEADER)) != size) {
            return;
        }

        const RAWINPUT *raw = reinterpret_cast<const RAWINPUT *>(buffer.data());

        if (raw->header.dwType == RIM_TYPEKEYBOARD) {
            const RAWKEYBOARD &kb = raw->data.keyboard;
            unsigned int vk = kb.VKey;

            if (vk == 0 || vk >= KEY_COUNT) return;

            bool isDown = !(kb.Flags & RI_KEY_BREAK);
            m_currKeys[vk] = isDown;
            LOG_TRACE("Key " + std::to_string(vk) + " " + (isDown ? "down" : "up"));
        } else if (raw->header.dwType == RIM_TYPEMOUSE) {
            const RAWMOUSE &mouse = raw->data.mouse;

            m_deltaX += static_cast<float>(mouse.lLastX);
            m_deltaY += static_cast<float>(mouse.lLastY);

            if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) m_currBtns[0] = true;
            if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) m_currBtns[0] = false;
            if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) m_currBtns[1] = true;
            if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) m_currBtns[1] = false;
            if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) m_currBtns[2] = true;
            if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) m_currBtns[2] = false;

            POINT pos = {};
            GetCursorPos(&pos);
            ScreenToClient(m_hwnd, &pos);
            m_mousePosX = static_cast<float>(pos.x);
            m_mousePosY = static_cast<float>(pos.y);
        }
    }

    void InputSystem::endFrame() {
        m_prevKeys = m_currKeys;
        m_prevBtns = m_currBtns;
        m_deltaX = 0.0f;
        m_deltaY = 0.0f;

        if (m_captured) {
            RECT rect = {};
            GetClientRect(m_hwnd, &rect);

            POINT center = {
                (rect.left + rect.right) / 2,
                (rect.top + rect.bottom) / 2
            };
            ClientToScreen(m_hwnd, &center);
            SetCursorPos(center.x, center.y);
        }
    }

    bool InputSystem::isKeyDown(Key key) const {
        unsigned int idx = static_cast<unsigned int>(key);
        if (idx >= KEY_COUNT) return false;
        return m_currKeys[idx];
    }

    bool InputSystem::isKeyPressed(Key key) const {
        unsigned int idx = static_cast<unsigned int>(key);
        if (idx >= KEY_COUNT) return false;
        return m_currKeys[idx] && !m_prevKeys[idx];
    }

    bool InputSystem::isKeyReleased(Key key) const {
        unsigned int idx = static_cast<unsigned int>(key);
        if (idx >= KEY_COUNT) return false;
        return !m_currKeys[idx] && m_prevKeys[idx];
    }

    bool InputSystem::isMouseDown(MouseButton btn) const {
        unsigned int idx = static_cast<unsigned int>(btn);
        if (idx >= BUTTON_COUNT) return false;
        return m_currBtns[idx];
    }

    bool InputSystem::isMousePressed(MouseButton btn) const {
        unsigned int idx = static_cast<unsigned int>(btn);
        if (idx >= BUTTON_COUNT) return false;
        return m_currBtns[idx] && !m_prevBtns[idx];
    }

    bool InputSystem::isMouseReleased(MouseButton btn) const {
        unsigned int idx = static_cast<unsigned int>(btn);
        if (idx >= BUTTON_COUNT) return false;
        return !m_currBtns[idx] && m_prevBtns[idx];
    }

    DirectX::XMFLOAT2 InputSystem::getMouseDelta() const {
        return {m_deltaX, m_deltaY};
    }

    DirectX::XMFLOAT2 InputSystem::getMousePosition() const {
        return {m_mousePosX, m_mousePosY};
    }

    void InputSystem::setMouseCaptured(bool captured) {
        if (m_captured == captured) return;
        m_captured = captured;

        if (captured) {
            applyCursorCapture();
            LOG_DEBUG("Mouse captured");
        } else {
            releaseCursorCapture();
            LOG_DEBUG("Mouse released");
        }
    }

    bool InputSystem::isMouseCaptured() const {
        return m_captured;
    }

    void InputSystem::onKillFocus() {
        m_currKeys.fill(false);
        m_currBtns.fill(false);

        if (m_captured) {
            releaseCursorCapture();
            m_captured = false;
            LOG_DEBUG("Mouse capture released (focus lost)");
        }
    }

    void InputSystem::applyCursorCapture() {
        ShowCursor(FALSE);

        RECT rect = {};
        GetClientRect(m_hwnd, &rect);

        POINT topLeft = {rect.left, rect.top};
        POINT bottomRight = {rect.right, rect.bottom};
        ClientToScreen(m_hwnd, &topLeft);
        ClientToScreen(m_hwnd, &bottomRight);

        RECT screenRect = {
            topLeft.x, topLeft.y,
            bottomRight.x, bottomRight.y
        };
        ClipCursor(&screenRect);
    }

    void InputSystem::releaseCursorCapture() {
        ClipCursor(nullptr);
        ShowCursor(TRUE);
    }
}
