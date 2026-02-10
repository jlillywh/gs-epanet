#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * @brief Construct a new Logger object with default settings
 */
Logger::Logger() : m_level(LogLevel::OFF) {
    // Log file is not opened until SetLogFile() is called
}

/**
 * @brief Destroy the Logger object and close the log file
 */
Logger::~Logger() {
    Close();
}

/**
 * @brief Set the logging level
 * 
 * @param level The log level to set (OFF, ERROR, INFO, DEBUG)
 */
void Logger::SetLevel(LogLevel level) {
    m_level = level;
}

/**
 * @brief Set the log file path and open the file for writing
 * 
 * If a log file is already open, it will be closed first.
 * The file is opened in append mode to preserve existing content.
 * To start fresh, delete the log file before running.
 * 
 * @param filename Path to the log file (relative or absolute)
 * @param truncate If true, truncate (overwrite) the file; if false, append
 * @return true if the file was opened successfully, false otherwise
 */
bool Logger::SetLogFile(const std::string& filename, bool truncate) {
    // Close existing file if open
    if (m_file.is_open()) {
        m_file.close();
    }

    // Open file in truncate or append mode
    if (truncate) {
        m_file.open(filename, std::ios::out | std::ios::trunc);
    } else {
        m_file.open(filename, std::ios::out | std::ios::app);
    }
    
    return m_file.is_open();
}

/**
 * @brief Write an error message to the log
 * 
 * Error messages are written if log level is ERROR, INFO, or DEBUG.
 * 
 * @param message The error message to log
 */
void Logger::Error(const std::string& message) {
    if (m_level >= LogLevel::ERROR) {
        WriteLog(LogLevel::ERROR, message);
    }
}

/**
 * @brief Write an informational message to the log
 * 
 * Info messages are written if log level is INFO or DEBUG.
 * 
 * @param message The informational message to log
 */
void Logger::Info(const std::string& message) {
    if (m_level >= LogLevel::INFO) {
        WriteLog(LogLevel::INFO, message);
    }
}

/**
 * @brief Write a debug message to the log
 * 
 * Debug messages are written only if log level is DEBUG.
 * 
 * @param message The debug message to log
 */
void Logger::Debug(const std::string& message) {
    if (m_level >= LogLevel::DEBUG) {
        WriteLog(LogLevel::DEBUG, message);
    }
}

/**
 * @brief Close the log file
 * 
 * This method is called automatically by the destructor.
 */
void Logger::Close() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

/**
 * @brief Get the current timestamp in ISO 8601 format
 * 
 * Format: YYYY-MM-DDTHH:MM:SS.mmm
 * Example: 2026-02-07T14:32:15.123
 * 
 * Requirements: 7.6
 * 
 * @return std::string Formatted timestamp
 */
std::string Logger::GetTimestamp() {
    // Get current time with millisecond precision
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    // Format timestamp as ISO 8601: YYYY-MM-DDTHH:MM:SS.mmm
    std::tm tm_buf;
    localtime_s(&tm_buf, &now_time_t);  // Windows-safe version
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    
    return oss.str();
}

/**
 * @brief Write a log entry to the file
 * 
 * Format: [timestamp] [LEVEL] message
 * Example: [2026-02-07T14:32:15.123] [INFO] XF_INITIALIZE called
 * 
 * Requirements: 7.1, 7.6
 * 
 * @param level The log level of this message
 * @param message The message to write
 */
void Logger::WriteLog(LogLevel level, const std::string& message) {
    // Only write if file is open and level is not OFF
    if (!m_file.is_open() || m_level == LogLevel::OFF) {
        return;
    }

    // Format: [timestamp] [LEVEL] message
    m_file << "[" << GetTimestamp() << "] "
           << "[" << LogLevelToString(level) << "] "
           << message << std::endl;
    
    // Flush to ensure message is written immediately
    m_file.flush();
}

/**
 * @brief Convert LogLevel enum to string representation
 * 
 * @param level The log level to convert
 * @return std::string String representation ("ERROR", "INFO", "DEBUG")
 */
std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::OFF:
        default:
            return "UNKNOWN";
    }
}
