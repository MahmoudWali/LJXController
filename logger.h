#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"

class Logger
{
public:
    static std::shared_ptr<spdlog::logger> getLogger()
    {
        static std::shared_ptr<spdlog::logger> logger = initializeLogger();
        return logger;
    }


private:
    Logger() = default;

    static std::shared_ptr<spdlog::logger> initializeLogger()
    {
        auto logger = spdlog::daily_logger_mt("daily_logger", "LJX_Logger.txt", 0, 0);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
        logger->flush_on(spdlog::level::info);
        return logger;
    }
};

#endif // LOGGER_H
