#include "Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Application.h"

namespace engine::core {
    Logger& Logger::get() {
        static Logger instance;
        return instance;
    }

    Logger::~Logger() {
        if (m_file.is_open()) {
            m_file.flush();
            m_file.close();
        }
    }

    void Logger::setMinLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minLevel = level;
    }

    void Logger::openLogFile(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_file.open(path, std::ios::out | std::ios::trunc);
    }

    void Logger::log(LogLevel level,
                     const std::string& message,
                     const std::source_location& location)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (level < m_minLevel) {
            return;
        }

        std::string formatted = formatMessage(level, message, location);
        writeToConsole(level, formatted);
        writeToFile(formatted);
    }

    std::string Logger::formatMessage(LogLevel level,
                                  const std::string& message,
                                  const std::source_location& location)
{
    auto now   = std::chrono::system_clock::now();
    auto time  = std::chrono::system_clock::to_time_t(now);
    auto ms    = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now.time_since_epoch()) % 1000;

    std::tm localTime{};
    localtime_s(&localTime, &time);

    std::string filename = location.file_name();
    auto        slash    = filename.find_last_of("/\\");
    if (slash != std::string::npos) {
        filename = filename.substr(slash + 1);
    }

    std::ostringstream oss;
    oss << "["
        << std::setfill('0')
        << std::setw(2) << localTime.tm_hour << ":"
        << std::setw(2) << localTime.tm_min  << ":"
        << std::setw(2) << localTime.tm_sec  << "."
        << std::setw(3) << ms.count()
        << "] "
        << "[" << levelToString(level) << "] "
        << "[" << filename << ":" << location.line() << "] "
        << message;

    return oss.str();
}

void Logger::writeToConsole(LogLevel level, const std::string& formatted) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, levelToConsoleColor(level));
    std::cout << formatted << "\n";

    SetConsoleTextAttribute(hConsole, 7);
}

void Logger::writeToFile(const std::string& formatted) {
    if (m_file.is_open()) {
        m_file << formatted << "\n";
        m_file.flush();
    }
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default:              return "?????";
    }
}

int Logger::levelToConsoleColor(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return 8;
        case LogLevel::Debug: return 7;
        case LogLevel::Info:  return 10;
        case LogLevel::Warn:  return 14;
        case LogLevel::Error: return 12;
        case LogLevel::Fatal: return 12 | BACKGROUND_RED;
        default:              return 7;
    }
}
}
