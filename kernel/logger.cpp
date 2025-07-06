#include "logger.hpp"

#include <cstdio>

#include "console.hpp"

namespace
{
    auto log_level = kDebug;
}

extern Console* console;

void set_log_level(const LogLevel level)
{
    log_level = level;
}

int log(const LogLevel level, const char* format, ...)
{
    if (level > log_level)
    {
        return 0;
    }

    va_list ap;
    char s[1024];

    va_start(ap, format);
    const int result = vsnprintf(s, sizeof(s), format, ap);
    va_end(ap);

    console->put_string(s);
    return result;
}

// alias
int Log(const LogLevel level, const char* format, ...)
{
    return log(level, format);
}

