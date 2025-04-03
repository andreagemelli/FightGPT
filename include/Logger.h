#pragma once
#include <iostream>
#include <string>
#include <ctime>

class Logger {
public:
    static void info(const std::string& message) {
        log("INFO", message);
    }
    
    static void error(const std::string& message) {
        log("ERROR", message);
    }
    
private:
    static void log(const std::string& level, const std::string& message) {
        time_t now = time(0);
        std::string timestamp = ctime(&now);
        timestamp.pop_back(); // Remove newline
        std::cout << "[" << timestamp << "] [" << level << "] " << message << std::endl;
    }
}; 