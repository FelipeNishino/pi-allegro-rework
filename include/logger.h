#ifndef __LOGGER_H__
#define __LOGGER_H__

enum LogLevel
{
    LOG_FATAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
};

void logger_set_output_level(enum LogLevel level);
void logger_log(enum LogLevel lvl, const char* format, ... );

#endif //__LOGGER_H__