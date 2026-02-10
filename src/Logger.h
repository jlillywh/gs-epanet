#pragma once

#include <string>
#include <fstream>

/**
 * @brief Log level enumeration for controlling logging verbosity
 * 
 * OFF    - No logging
 * ERROR  - Only error messages
 * INFO   - Informational messages and errors
 * DEBUG  - All messages including debug information
 */
enum class LogLevel {
    OFF,
    ERROR,
    INFO,
    DEBUG
};

/**
 * @brief Logger class for writing timestamped log messages to file
 * 
 * The Logger class provides a simple interface for writing log messages
 * with different severity levels. Messages are filtered based on the
 * configured log level and written to a file with ISO 8601 timestamps.
 * 
 * Requirements: 7.1, 7.6
 */
class Logger {
public:
    /**
     * @brief Construct a new Logger object with default settings
     * 
     * Default log level is OFF, no log file is opened until SetLogFile() is called.
     */
    Logger();

    /**
     * @brief Destroy the Logger object and close the log file
     */
    ~Logger();

    /**
     * @brief Set the logging level
     * 
     * @param level The log level to set (OFF, ERROR, INFO, DEBUG)
     */
    void SetLevel(LogLevel level);

    /**
     * @brief Set the log file path and open the file for writing
     * 
     * @param filename Path to the log file (relative or absolute)
     * @param truncate If true, truncate (overwrite) the file; if false, append (default: false)
     * @return true if the file was opened successfully, false otherwise
     */
    bool SetLogFile(const std::string& filename, bool truncate = false);

    /**
     * @brief Write an error message to the log
     * 
     * Error messages are written if log level is ERROR, INFO, or DEBUG.
     * 
     * @param message The error message to log
     */
    void Error(const std::string& message);

    /**
     * @brief Write an informational message to the log
     * 
     * Info messages are written if log level is INFO or DEBUG.
     * 
     * @param message The informational message to log
     */
    void Info(const std::string& message);

    /**
     * @brief Write a debug message to the log
     * 
     * Debug messages are written only if log level is DEBUG.
     * 
     * @param message The debug message to log
     */
    void Debug(const std::string& message);

    /**
     * @brief Close the log file
     * 
     * This method is called automatically by the destructor.
     */
    void Close();

private:
    LogLevel m_level;        ///< Current log level
    std::ofstream m_file;    ///< Output file stream for log file

    /**
     * @brief Get the current timestamp in ISO 8601 format
     * 
     * Format: YYYY-MM-DDTHH:MM:SS.mmm
     * Example: 2026-02-07T14:32:15.123
     * 
     * @return std::string Formatted timestamp
     */
    std::string GetTimestamp();

    /**
     * @brief Write a log entry to the file
     * 
     * Format: [timestamp] [LEVEL] message
     * Example: [2026-02-07T14:32:15.123] [INFO] XF_INITIALIZE called
     * 
     * @param level The log level of this message
     * @param message The message to write
     */
    void WriteLog(LogLevel level, const std::string& message);

    /**
     * @brief Convert LogLevel enum to string representation
     * 
     * @param level The log level to convert
     * @return std::string String representation ("ERROR", "INFO", "DEBUG")
     */
    std::string LogLevelToString(LogLevel level);
};
