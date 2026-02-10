#include "../../src/Logger.h"
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

/**
 * @brief Test fixture for Logger tests
 * 
 * Provides setup and teardown for Logger unit tests, including
 * test log file management.
 */
class LoggerTest : public ::testing::Test {
protected:
    const std::string testLogFile = "test_logger.log";

    void SetUp() override {
        // Clean up any existing test log file
        if (fs::exists(testLogFile)) {
            fs::remove(testLogFile);
        }
    }

    void TearDown() override {
        // Clean up test log file after test
        if (fs::exists(testLogFile)) {
            fs::remove(testLogFile);
        }
    }

    /**
     * @brief Read the contents of the test log file
     * 
     * @return std::string Contents of the log file
     */
    std::string ReadLogFile() {
        std::ifstream file(testLogFile);
        if (!file.is_open()) {
            return "";
        }
        
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        
        return content;
    }

    /**
     * @brief Count the number of lines in the log file
     * 
     * @return int Number of lines
     */
    int CountLogLines() {
        std::ifstream file(testLogFile);
        if (!file.is_open()) {
            return 0;
        }
        
        int count = 0;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                count++;
            }
        }
        
        return count;
    }
};

/**
 * @brief Test Logger construction and default state
 */
TEST_F(LoggerTest, ConstructorDefaultState) {
    Logger logger;
    
    // Logger should be constructed successfully
    // Default level is OFF, so no messages should be written
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.Error("This should not be written");
    logger.Info("This should not be written");
    logger.Debug("This should not be written");
    
    logger.Close();
    
    // File should exist but be empty (or only have messages from file opening)
    int lineCount = CountLogLines();
    EXPECT_EQ(lineCount, 0) << "No messages should be written when log level is OFF";
}

/**
 * @brief Test SetLevel functionality
 */
TEST_F(LoggerTest, SetLevel) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    // Test ERROR level
    logger.SetLevel(LogLevel::ERROR);
    logger.Error("Error message");
    logger.Info("Info message");
    logger.Debug("Debug message");
    logger.Close();
    
    std::string content = ReadLogFile();
    EXPECT_NE(content.find("Error message"), std::string::npos) << "ERROR message should be written at ERROR level";
    EXPECT_EQ(content.find("Info message"), std::string::npos) << "INFO message should not be written at ERROR level";
    EXPECT_EQ(content.find("Debug message"), std::string::npos) << "DEBUG message should not be written at ERROR level";
}

/**
 * @brief Test INFO log level filtering
 */
TEST_F(LoggerTest, InfoLevelFiltering) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::INFO);
    logger.Error("Error message");
    logger.Info("Info message");
    logger.Debug("Debug message");
    logger.Close();
    
    std::string content = ReadLogFile();
    EXPECT_NE(content.find("Error message"), std::string::npos) << "ERROR message should be written at INFO level";
    EXPECT_NE(content.find("Info message"), std::string::npos) << "INFO message should be written at INFO level";
    EXPECT_EQ(content.find("Debug message"), std::string::npos) << "DEBUG message should not be written at INFO level";
}

/**
 * @brief Test DEBUG log level filtering
 */
TEST_F(LoggerTest, DebugLevelFiltering) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::DEBUG);
    logger.Error("Error message");
    logger.Info("Info message");
    logger.Debug("Debug message");
    logger.Close();
    
    std::string content = ReadLogFile();
    EXPECT_NE(content.find("Error message"), std::string::npos) << "ERROR message should be written at DEBUG level";
    EXPECT_NE(content.find("Info message"), std::string::npos) << "INFO message should be written at DEBUG level";
    EXPECT_NE(content.find("Debug message"), std::string::npos) << "DEBUG message should be written at DEBUG level";
}

/**
 * @brief Test OFF log level (no messages written)
 */
TEST_F(LoggerTest, OffLevelNoMessages) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::OFF);
    logger.Error("Error message");
    logger.Info("Info message");
    logger.Debug("Debug message");
    logger.Close();
    
    int lineCount = CountLogLines();
    EXPECT_EQ(lineCount, 0) << "No messages should be written when log level is OFF";
}

/**
 * @brief Test log entry format includes timestamp
 * 
 * Requirements: 7.6
 */
TEST_F(LoggerTest, LogEntryFormatWithTimestamp) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::INFO);
    logger.Info("Test message");
    logger.Close();
    
    std::string content = ReadLogFile();
    
    // Check for ISO 8601 timestamp format: [YYYY-MM-DDTHH:MM:SS.mmm]
    EXPECT_NE(content.find("["), std::string::npos) << "Log entry should start with '['";
    EXPECT_NE(content.find("T"), std::string::npos) << "Timestamp should contain 'T' separator";
    EXPECT_NE(content.find("]"), std::string::npos) << "Timestamp should end with ']'";
    EXPECT_NE(content.find("[INFO]"), std::string::npos) << "Log entry should contain log level";
    EXPECT_NE(content.find("Test message"), std::string::npos) << "Log entry should contain message";
}

/**
 * @brief Test log entry format for different log levels
 */
TEST_F(LoggerTest, LogEntryFormatWithLevels) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::DEBUG);
    logger.Error("Error test");
    logger.Info("Info test");
    logger.Debug("Debug test");
    logger.Close();
    
    std::string content = ReadLogFile();
    
    EXPECT_NE(content.find("[ERROR]"), std::string::npos) << "ERROR level should be in log";
    EXPECT_NE(content.find("[INFO]"), std::string::npos) << "INFO level should be in log";
    EXPECT_NE(content.find("[DEBUG]"), std::string::npos) << "DEBUG level should be in log";
}

/**
 * @brief Test SetLogFile with invalid path
 */
TEST_F(LoggerTest, SetLogFileInvalidPath) {
    Logger logger;
    
    // Try to open a file in a non-existent directory
    bool result = logger.SetLogFile("/invalid/path/that/does/not/exist/test.log");
    
    EXPECT_FALSE(result) << "SetLogFile should return false for invalid path";
}

/**
 * @brief Test SetLogFile with valid path
 */
TEST_F(LoggerTest, SetLogFileValidPath) {
    Logger logger;
    
    bool result = logger.SetLogFile(testLogFile);
    
    EXPECT_TRUE(result) << "SetLogFile should return true for valid path";
    EXPECT_TRUE(fs::exists(testLogFile)) << "Log file should be created";
    
    logger.Close();
}

/**
 * @brief Test Close method
 */
TEST_F(LoggerTest, CloseMethod) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::INFO);
    logger.Info("Message before close");
    logger.Close();
    
    // File should exist and contain the message
    EXPECT_TRUE(fs::exists(testLogFile));
    std::string content = ReadLogFile();
    EXPECT_NE(content.find("Message before close"), std::string::npos);
}

/**
 * @brief Test multiple messages are written correctly
 */
TEST_F(LoggerTest, MultipleMessages) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::INFO);
    logger.Info("Message 1");
    logger.Info("Message 2");
    logger.Info("Message 3");
    logger.Close();
    
    std::string content = ReadLogFile();
    EXPECT_NE(content.find("Message 1"), std::string::npos);
    EXPECT_NE(content.find("Message 2"), std::string::npos);
    EXPECT_NE(content.find("Message 3"), std::string::npos);
    
    int lineCount = CountLogLines();
    EXPECT_EQ(lineCount, 3) << "Should have exactly 3 log lines";
}

/**
 * @brief Test timestamp format is ISO 8601
 * 
 * Requirements: 7.6
 */
TEST_F(LoggerTest, TimestampFormatISO8601) {
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    
    logger.SetLevel(LogLevel::INFO);
    logger.Info("Timestamp test");
    logger.Close();
    
    std::string content = ReadLogFile();
    
    // ISO 8601 format: YYYY-MM-DDTHH:MM:SS.mmm
    // Example: [2026-02-07T14:32:15.123]
    
    // Check for basic structure
    size_t start = content.find("[");
    size_t end = content.find("]");
    ASSERT_NE(start, std::string::npos);
    ASSERT_NE(end, std::string::npos);
    
    std::string timestamp = content.substr(start + 1, end - start - 1);
    
    // Check format: YYYY-MM-DDTHH:MM:SS.mmm
    EXPECT_EQ(timestamp.length(), 23) << "Timestamp should be 23 characters long";
    EXPECT_EQ(timestamp[4], '-') << "Character at position 4 should be '-'";
    EXPECT_EQ(timestamp[7], '-') << "Character at position 7 should be '-'";
    EXPECT_EQ(timestamp[10], 'T') << "Character at position 10 should be 'T'";
    EXPECT_EQ(timestamp[13], ':') << "Character at position 13 should be ':'";
    EXPECT_EQ(timestamp[16], ':') << "Character at position 16 should be ':'";
    EXPECT_EQ(timestamp[19], '.') << "Character at position 19 should be '.'";
}

/**
 * @brief Test destructor closes file properly
 */
TEST_F(LoggerTest, DestructorClosesFile) {
    {
        Logger logger;
        ASSERT_TRUE(logger.SetLogFile(testLogFile));
        logger.SetLevel(LogLevel::INFO);
        logger.Info("Test message");
        // Logger goes out of scope here, destructor should close file
    }
    
    // File should exist and be readable
    EXPECT_TRUE(fs::exists(testLogFile));
    std::string content = ReadLogFile();
    EXPECT_NE(content.find("Test message"), std::string::npos);
}

/**
 * @brief Test changing log file
 */
TEST_F(LoggerTest, ChangeLogFile) {
    const std::string secondLogFile = "test_logger2.log";
    
    Logger logger;
    ASSERT_TRUE(logger.SetLogFile(testLogFile));
    logger.SetLevel(LogLevel::INFO);
    logger.Info("Message in first file");
    
    // Change to second file
    ASSERT_TRUE(logger.SetLogFile(secondLogFile));
    logger.Info("Message in second file");
    logger.Close();
    
    // Check first file
    std::string content1 = ReadLogFile();
    EXPECT_NE(content1.find("Message in first file"), std::string::npos);
    EXPECT_EQ(content1.find("Message in second file"), std::string::npos);
    
    // Check second file
    std::ifstream file2(secondLogFile);
    ASSERT_TRUE(file2.is_open());
    std::string content2;
    std::string line;
    while (std::getline(file2, line)) {
        content2 += line + "\n";
    }
    file2.close();
    
    EXPECT_EQ(content2.find("Message in first file"), std::string::npos);
    EXPECT_NE(content2.find("Message in second file"), std::string::npos);
    
    // Clean up second file
    if (fs::exists(secondLogFile)) {
        fs::remove(secondLogFile);
    }
}

/**
 * @brief Test append mode (messages are appended, not overwritten)
 */
TEST_F(LoggerTest, AppendMode) {
    {
        Logger logger;
        ASSERT_TRUE(logger.SetLogFile(testLogFile));
        logger.SetLevel(LogLevel::INFO);
        logger.Info("First message");
        logger.Close();
    }
    
    {
        Logger logger;
        ASSERT_TRUE(logger.SetLogFile(testLogFile));
        logger.SetLevel(LogLevel::INFO);
        logger.Info("Second message");
        logger.Close();
    }
    
    std::string content = ReadLogFile();
    EXPECT_NE(content.find("First message"), std::string::npos) << "First message should still be in file";
    EXPECT_NE(content.find("Second message"), std::string::npos) << "Second message should be appended";
    
    int lineCount = CountLogLines();
    EXPECT_EQ(lineCount, 2) << "Should have 2 log lines (appended)";
}
