#pragma once
#ifndef _LOGGER_H
#define _LOGGER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

class Logger;
extern Logger logger;

enum LogLevel {
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3,
};

#define LOG_INIT(path, filename) logger.Init(path, filename);
#define LOG_ERROR (logger << logger.prefix(ERROR))
#define LOG_WARN (logger << logger.prefix(WARN))
#define LOG_INFO (logger << logger.prefix(INFO))
#define LOG_DEBUG (logger << logger.prefix(DEBUG))

class Logger
{
    public:
        Logger();

        void Init(std::string path, std::string filename);
        void Close();

        template<typename T> Logger& operator<<(T t);
        Logger& operator<<(std::ostream& (*func) (std::ostream&));
        std::string prefix(const LogLevel logLevel);
    
    private:
        std::string filename;
        std::ofstream stream;
        LogLevel level;
};

template<typename T> inline Logger& Logger::operator<<(T t)
{
    if (stream.is_open())
        stream << t;
    
    return *this;
}

inline Logger& Logger::operator<<(std::ostream& (*func) (std::ostream&))
{
    if (stream.is_open())
        stream << std::endl;
    
    return *this;
}

#endif // _LOGGER_H