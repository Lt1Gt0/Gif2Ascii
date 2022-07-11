#include "Debug/logger.h"
#include <chrono>

Logger logger;

Logger::Logger()
{
    filename = "";
    level = ERROR;
}

void Logger::Init(std::string path, std::string filename)
{
    if (!path.empty() && !filename.empty()) {
        filename = std::string(path) + std::string(filename) + "_log.txt";
        stream.open(filename);

        unsigned char bom[] = {0xEF, 0xBB, 0xBF};
        stream.write((char*)bom, sizeof(bom));
    }
}

void Logger::Close()
{
    if (stream.is_open())
        stream.close();
}

std::string Logger::prefix(const LogLevel logLevel)
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
    std::string dateTimeStr(25, '\0');
    std::strftime(&dateTimeStr[0], dateTimeStr.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    // Strip all null characters
    for (int i = 24; dateTimeStr[i] == '\0'; i--) {
        dateTimeStr.pop_back();
    }
    
    std::string logLevelText;

    switch (logLevel) {
        case ERROR:
            logLevelText = "    ERROR   ";
            break;
        case WARN:
            logLevelText = "    WARN    ";
            break;
        case INFO:
            logLevelText = "    INFO    ";
            break;
        case DEBUG:
            logLevelText = "    DEBUG   ";
            break;
    }

    return dateTimeStr + logLevelText;
}