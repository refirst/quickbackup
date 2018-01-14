#include <iostream>
#include <string>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include "qb_log.h"

#define LOG_BUF_LEN (2600)

namespace qb_log
{
    static FILE* _log_file_handler = NULL;

    void log_init(const char* file)
    {
        _log_file_handler = fopen(file, "a+");
        if (_log_file_handler == NULL)
        {
            int err = errno;
            char buf[2600] = { 0 };
            sprintf(buf, "open log file [%s] failed: [%d:%s]!", file, err, strerror(err));
            std::cout << buf << std::endl;
        }
        else
        {
            LOG_INFO(("open log file [%s] success!", file));
        }
    }

    void log_uninit()
    {
        if (_log_file_handler != NULL)
        {
            fflush(_log_file_handler);
            fclose(_log_file_handler);
            _log_file_handler = NULL;
        }
    }

    void log_msg(qb_log_level_e level, int line, const char* func_name, char* msg)
    {
        if (_log_file_handler == NULL || msg == NULL)
        {
            return;
        }

        std::string level_str = "WARN";
        if (level == QB_LEVEL_INFO)
        {
            level_str = "INFO";
        }

        struct timeval tv;
        gettimeofday(&tv, NULL);
        time_t ltime = tv.tv_sec;
        struct tm* t = localtime(&ltime);
        fprintf(_log_file_handler,
            "[%04d-%02d-%02d %02d:%02d:%02d.%06d][%s][%s:%d] %s\n",
            t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec,
            tv.tv_usec,
            level_str.c_str(),
            func_name,
            line,
            msg);
        fflush(_log_file_handler);

        free(msg);
        msg = NULL;
    }

    char* format_log_msg(const char* format, ...)
    {
        char* buf = (char*)malloc(LOG_BUF_LEN);
        if (buf == NULL)
        {
            return NULL;
        }

        va_list args;
        va_start(args, format);
        int ret = vsnprintf(buf, LOG_BUF_LEN, format, args);
        if (ret <= 0 || ret > LOG_BUF_LEN)
        {
            free(buf);
            buf = NULL;
        }
        va_end(args);

        return buf;
    }
}
