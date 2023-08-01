#pragma once


enum __LOG_LEVEL {
    __LOG_LEVEL_INFO = 0,
    __LOG_LEVEL_WARN,
    __LOG_LEVEL_ERROR,
    __LOG_LEVEL_DEBUG,
};


#define LOG_INFO(domain, ...)  log(__LOG_LEVEL_INFO, domain, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(domain, ...)  log(__LOG_LEVEL_WARN, domain, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(domain, ...) log(__LOG_LEVEL_ERROR, domain, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(domain, ...) log(__LOG_LEVEL_DEBUG, domain, __FILE__, __LINE__, __VA_ARGS__)


void log(int level, const char *domain, 
          const char *file, int line, const char *fmt, ...);