#pragma once
#ifndef _LOGGER_H
#define _LOGGER_H

#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <chrono>

class Logger;
extern Logger logger;

enum LogLevel {
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3,
    SUCCESS = 4,
};

#define LOG_INIT(path, filename) logger.Init(path, filename);
#define LOG_ERROR (logger << logger.prefix(ERROR))
#define LOG_WARN (logger << logger.prefix(WARN))
#define LOG_INFO (logger << logger.prefix(INFO))
#define LOG_DEBUG (logger << logger.prefix(DEBUG))
#define LOG_SUCCESS (logger << logger.prefix(SUCCESS))

class Logger
{
    public:
        Logger()
        {
            this->mFilename = ""; 
        }

        void Init(std::string path, std::string filename)
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

        void Close()
        {
            if (this->mStream.is_open())
                this->mStream.close();
        }

        template<typename T> Logger& operator<<(T t)
        {
            if (this->mStream.is_open())
                this->mStream << t;
            
            return *this;
        }

        Logger& operator<<(std::ostream& (*func) (std::ostream&))
        {
            if (this->mStream.is_open())
                this->mStream << std::endl;
    
            return *this;
        }

        std::string prefix(const LogLevel logLevel)
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

    
    private:
        std::string mFilename;
        std::ofstream mStream;
};

#endif // _LOGGER_H
