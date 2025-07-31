#pragma once
#include <string>
#include <mutex>
#include <fstream>

/**
 * @class Logfile
 * @brief A thread-safe logging utility that writes messages to a file.
 * 
 * This class provides a simple interface to write logs to a file in a
 * thread-safe manner using a mutex. It supports only one instance per
 * log file to avoid file contention or race conditions.
 */
class Logfile
{
public:
    /**
     * @brief Constructs a Logfile object.
     * 
     * @param path The filesystem path to the log file. The file is opened
     *        in append mode each time a log is written.
     */
    explicit Logfile(const std::string& path);

    /**
     * @brief Deleted copy constructor to prevent copying
     */
    Logfile(const Logfile&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent assignment
     */
    Logfile& operator=(const Logfile&) = delete;

    /**
     * @brief Write a log message to the file in a thread-safe way.
     * 
     * Each message is appended with a newline. The file is opened
     * for each write and closed afterward to minimize the risk of data loss.
     * 
     * @param msg The message to log
     */
    void threadsafe_log(const std::string& msg);

private:
    std::string path;      // Path to log file
    std::mutex m;          // Protects log file
};