#ifndef MPRPC_SRC_INCLUDE_LOGGER_H_
#define MPRPC_SRC_INCLUDE_LOGGER_H_

// MpRPC logger 
#include <string>
#include "lockqueue.h"

enum LogLevel {
    INFO,   // normal info.
    ERROR,  // error info.
};

class Logger {
public:
    static Logger& GetInstance();

    inline void SetLogLevel(LogLevel level) {log_level_ = level;}
    void Log(const std::string message);      // write log
private:
    int log_level_;     // log level 
    LockQueue<std::string> lock_que_;    // log buffer

    Logger();
    Logger(const Logger& log) = delete;
    Logger(Logger&&) = delete;
};

// user custom log format
#define LOG_INFO(log_massage_format, ...) \
    do { \
        Logger& logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, log_massage_format, ##__VA_ARGS__); \
        logger.Log(buf); \
    } while (0)

#define LOG_ERROR(log_massage_format, ...) \
    do { \
        Logger& logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, log_massage_format, ##__VA_ARGS__); \
        logger.Log(buf); \
    } while (0)

#endif // MPRPC_SRC_INCLUDE_LOGGER_H_