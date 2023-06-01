#pragma once
#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <chrono>
#include "utils/strutils.hpp"

class Logger;
extern Logger logger;
constexpr const char* COLOR_TRACE           {"\033[1;30m"};
constexpr const char* COLOR_SUCCESS         {"\033[1;32m"};
constexpr const char* COLOR_DEBUG           {"\033[1;37m"};
constexpr const char* COLOR_UNIMPLEMENTED   {"\033[1;35m"};
constexpr const char* COLOR_INFO            {"\033[1;34m"};
constexpr const char* COLOR_WARNING         {"\033[1;33m"};
constexpr const char* COLOR_ERROR           {"\033[1;31m"};
constexpr const char* COLOR_RESET           {"\033[0m"};

enum LogLevel {
    ERROR,
    WARNING,
    SUCCESS,
    INFO,
    DEBUG,
    TRACE,
    UNIMPLEMENTED
};

class Logger
{
    public:
        Logger(bool enableConsoleOut = true)
        {
            mFilename = ""; 
            mConsoleOutEnabled = enableConsoleOut; 
        }

        Logger(std::string path, std::string filename, bool enableConsoleOut = true) {
            if (!path.empty() && !filename.empty()) {
                mFilename = std::string(path) + std::string(filename) + ".log";
                mStream.open(mFilename);

                // Write the Byte Order Mark into the beginning of the file
                unsigned char bom[] = {0xEF, 0xBB, 0xBF};
                mStream.write((char*)bom, sizeof(bom));
            }

            mConsoleOutEnabled = enableConsoleOut;
            Log(SUCCESS, "Initialized Logger");        
        }

        void Close()
        {
            if (mStream.is_open())
                mStream.close();
        }

        template<typename T> Logger& operator<<(T t)
        {
            if (mStream.is_open())
                mStream << t;
            
            return *this;
        }

        Logger& operator<<(std::ostream& (*func) (std::ostream&))
        {
            if (mStream.is_open())
                mStream << std::endl;
    
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
                case WARNING:
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
                case TRACE:
                    logLevelText = "    TRACE   ";
                    break;
                case UNIMPLEMENTED:
                    logLevelText = "    UNIMPLEMENTED ";
                    break;
            }

            return dateTimeStr + logLevelText;
        }

        void Log(LogLevel level, const char* fmt, ...) 
        {
            if (!ShouldLog(level))
                return;

            va_list args;
            va_start(args, fmt);
            std::string msg = strFormat(fmt, args);
            va_end(args);
            
            if (mStream.is_open())
                mStream << prefix(level) << msg << std::endl;

            fprintf((level < LogLevel::ERROR) ? stdout : stderr,
                "%s| %s%s\n",
                LevelColor(level), COLOR_RESET, msg.c_str());
        }

        void SetLevel(LogLevel level) {
            mCurrentLevel = level;
        }

        void EnableTracing() {
            mTracingEnabled = true;
        }

        auto ShouldLog(LogLevel level) const -> bool 
        {
            if (!mTracingEnabled && level == LogLevel::TRACE) 
                return false;

            return (mConsoleOutEnabled) && (mCurrentLevel <= level);
        }

        inline auto LevelColor(LogLevel level) -> const char* 
        {
            switch (level) {
                case LogLevel::TRACE:
                    return COLOR_TRACE;
                case LogLevel::DEBUG:
                    return COLOR_DEBUG;
                case LogLevel::SUCCESS:
                    return COLOR_SUCCESS;
                case LogLevel::UNIMPLEMENTED:
                    return COLOR_UNIMPLEMENTED;
                case LogLevel::INFO:
                    return COLOR_INFO;
                case LogLevel::WARNING:
                    return COLOR_WARNING;
                case LogLevel::ERROR:
                    return COLOR_ERROR;
            }

            return COLOR_UNIMPLEMENTED;
        }

        void LogSetLevel(LogLevel level) {
            SetLevel(level);
        }

    private:
        std::string mFilename;
        std::ofstream mStream;
        LogLevel mCurrentLevel;
        bool mTracingEnabled;
        bool mConsoleOutEnabled;
};

#endif // _LOGGER_HPP_
