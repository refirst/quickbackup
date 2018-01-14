#ifndef QB_LOG_H
#define QB_LOG_H

enum qb_log_level_e
{
    QB_LEVEL_INFO = 1,
    QB_LEVEL_WARN = 2
};

#define LOG_INFO(x) qb_log::log_msg(QB_LEVEL_INFO, __LINE__, __func__, qb_log::format_log_msg x)
#define LOG_WARN(x) qb_log::log_msg(QB_LEVEL_WARN, __LINE__, __func__, qb_log::format_log_msg x)

namespace qb_log
{
    void log_init(const char* file);
    void log_uninit();

    void log_msg(qb_log_level_e level, int line, const char* func_name, char* msg);
    char* format_log_msg(const char* format, ...);
}

#endif
