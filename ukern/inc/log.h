#pragma once


enum __LOG_LEVEL {
    __LOG_LEVEL_INFO = 0,
    __LOG_LEVEL_WARN,
    __LOG_LEVEL_ERROR,
    __LOG_LEVEL_DEBUG,
};


#define LOG_INFO(...)  log(__LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log(__LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log(__LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log(__LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)


void log(int level, const char *file, int line, const char *fmt, ...);