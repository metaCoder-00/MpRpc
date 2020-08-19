#include "logger.h"
#include <time.h>
#include <iostream>

Logger& Logger::GetInstance() {
    static Logger logger;
    return logger;
}

Logger::Logger() {
    // start a write thread
    std::thread write_log_task([&](){
        while (true) {
            // get date a+
            time_t now = time(nullptr);
            tm* now_tm = localtime(&now);

            char file_name[128] = {0};
            sprintf(file_name, "../log/%d-%d-%d-log.txt", now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday);

            FILE* fp = fopen(file_name, "a+");
            if (nullptr == fp) {
                std::cout << "logger file: " << file_name << "open error!" << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string message = lock_que_.Pop();

            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d -> [%s]", 
                    now_tm->tm_hour, 
                    now_tm->tm_min, 
                    now_tm->tm_sec, 
                    (log_level_ == INFO ? "info" : "error"));
            message.insert(0, time_buf);
            message.append("\n");

            fputs(message.c_str(), fp);
            fclose(fp);
        }
    });
    // set detach thread
    write_log_task.detach();
}

// write log info into the lockqueue buffer
void Logger::Log(const std::string message) {
    lock_que_.Push(message);
}