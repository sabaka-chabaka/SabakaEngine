#pragma once
#include <fstream>
#include <mutex>
#include <source_location>
#include <string>

namespace engine::core {
    enum class LogLevel : int {
        Trace = 0,
        Debug = 1,
        Info  = 2,
        Warn  = 3,
        Error = 4,
        Fatal = 5,
    };

    class Logger {
    public:
        static Logger& get();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void setMinLevel(LogLevel level);
        void openLogFile(const std::string& path);

        void log(LogLevel level,
                 const std::string& message,
                 const std::source_location& location = std::source_location::current());
    private:
        Logger() = default;
        ~Logger();

        std::string formatMessage(LogLevel level, const std::string& message, const std::source_location& location);
        void             writeToConsole(LogLevel level, const std::string& formatted);
        void             writeToFile(const std::string& formatted);

        static const char* levelToString(LogLevel level);
        static int         levelToConsoleColor(LogLevel level);

        std::mutex   m_mutex;
        std::ofstream m_file;
        LogLevel     m_minLevel = LogLevel::Trace;
    };
}

#define LOG_TRACE(msg) ::engine::core::Logger::get().log(::engine::core::LogLevel::Trace, msg)
#define LOG_DEBUG(msg) ::engine::core::Logger::get().log(::engine::core::LogLevel::Debug, msg)
#define LOG_INFO(msg)  ::engine::core::Logger::get().log(::engine::core::LogLevel::Info,  msg)
#define LOG_WARN(msg)  ::engine::core::Logger::get().log(::engine::core::LogLevel::Warn,  msg)
#define LOG_ERROR(msg) ::engine::core::Logger::get().log(::engine::core::LogLevel::Error, msg)
#define LOG_FATAL(msg) ::engine::core::Logger::get().log(::engine::core::LogLevel::Fatal, msg)