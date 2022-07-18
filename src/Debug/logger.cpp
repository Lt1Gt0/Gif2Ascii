#include "Debug/logger.h"
#include <chrono>

Logger logger;

Logger::Logger()
{
    this->mFilename = "";
}

void Logger::Init(std::string path, std::string filename)
{
    if (!path.empty() && !filename.empty()) {
        this->mFilename = std::string(path) + std::string(filename) + "_log.txt";
        this->mStream.open(this->mFilename);

        // Write the Byte Order Mark into the beginning of the file
        unsigned char bom[] = {0xEF, 0xBB, 0xBF};
        this->mStream.write((char*)bom, sizeof(bom));
    }

    LOG_SUCCESS << "Initalized Logger" << std::endl;
}

void Logger::Close()
{
    if (this->mStream.is_open())
        this->mStream.close();
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
        case SUCCESS:
            logLevelText = "    SUCCESS ";
            break;
    }

    return dateTimeStr + logLevelText;
}
