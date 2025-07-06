#ifndef LOGGER_HPP
#define LOGGER_HPP

enum LogLevel
{
    kError = 3,
    kWarn = 4,
    kInfo = 6,
    kDebug = 7,
};

void set_log_level(LogLevel level);

int log(LogLevel level, const char* format, ...);
int Log(LogLevel level, const char* format, ...);


#endif //LOGGER_HPP
