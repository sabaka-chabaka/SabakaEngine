#pragma once
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <windows.h>

namespace engine {

    struct FileChangeEvent {
        std::string path;
    };

    class FileWatcher {
    public:
        using Callback = std::function<void(const FileChangeEvent&)>;

        explicit FileWatcher(const std::string& directory, Callback callback)
            : m_directory(directory), m_callback(std::move(callback))
        {
            m_handle = CreateFileA(
                m_directory.c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                nullptr);

            if (m_handle == INVALID_HANDLE_VALUE)
                return;

            m_running = true;
            m_thread  = std::thread(&FileWatcher::watchLoop, this);
        }

        ~FileWatcher() {
            m_running = false;
            if (m_handle != INVALID_HANDLE_VALUE) {
                CancelIoEx(m_handle, nullptr);
                CloseHandle(m_handle);
            }
            if (m_thread.joinable())
                m_thread.join();
        }

        FileWatcher(const FileWatcher&)            = delete;
        FileWatcher& operator=(const FileWatcher&) = delete;

    private:
        void watchLoop() {
            constexpr DWORD BUFFER_SIZE = 4096;
            alignas(DWORD) char buffer[BUFFER_SIZE];
            OVERLAPPED overlapped{};
            overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

            while (m_running) {
                DWORD bytesReturned = 0;

                ReadDirectoryChangesW(
                    m_handle,
                    buffer,
                    BUFFER_SIZE,
                    FALSE,
                    FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                    nullptr,
                    &overlapped,
                    nullptr);

                DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 200);

                if (!m_running) break;

                if (waitResult != WAIT_OBJECT_0) continue;

                if (!GetOverlappedResult(m_handle, &overlapped, &bytesReturned, FALSE))
                    continue;

                ResetEvent(overlapped.hEvent);

                const auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                while (true) {
                    if (info->Action == FILE_ACTION_MODIFIED ||
                        info->Action == FILE_ACTION_ADDED    ||
                        info->Action == FILE_ACTION_RENAMED_NEW_NAME)
                    {
                        int nameLen = static_cast<int>(info->FileNameLength / sizeof(wchar_t));
                        std::wstring wname(info->FileName, nameLen);
                        std::string  name(wname.begin(), wname.end());

                        FileChangeEvent ev;
                        ev.path = m_directory + "\\" + name;
                        m_callback(ev);
                    }

                    if (info->NextEntryOffset == 0) break;
                    info = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
                        reinterpret_cast<const char*>(info) + info->NextEntryOffset);
                }
            }

            CloseHandle(overlapped.hEvent);
        }

        std::string      m_directory;
        Callback         m_callback;
        HANDLE           m_handle  = INVALID_HANDLE_VALUE;
        std::atomic<bool> m_running{ false };
        std::thread      m_thread;
    };

}