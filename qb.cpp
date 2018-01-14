#include <iostream>
#include "qb_util.h"
#include "qb_log.h"
#include "qb_option.h"
#include "qb_daemon.h"
#include "qb_lock.h"
#include "qb_backup.h"

int _daemonize = 0;
int _need_lock = 0;
int _program_pid = 0;
int _thread_count = 1;
int _buffer_size = 1;
std::string _input_src = "";
std::string _output_dir = "";

int main(int argc, char** argv)
{
    //获取程序当前路径
    char work_dir_path[2600] = { 0 };
    if (readlink("/proc/self/exe", work_dir_path, 2600) < 0)
    {
        std::cout << "get exe directory error!!!" << std::endl;
        return (-1);
    }

    //设置日志文件
    std::string log_path = work_dir_path;
    log_path = log_path + ".log";

    char* end_ptr = strrchr(work_dir_path, '/');
    if (end_ptr == NULL)
    {
        std::cout << "get exe parent directory error!!!" << std::endl;
        return (-1);
    }
    *end_ptr = '\0';

    qb_log::log_init(log_path.c_str());
    if (qb_option::options_check(argc, argv, work_dir_path) != 0)
    {
        qb_log::log_uninit();
        exit(1);
    }
    qb_daemon::daemon_run(_daemonize, 0);

    qb_backup::backup_init();
    qb_lock::lock_init(_input_src.c_str());
    qb_backup::backup_run();
    qb_lock::lock_uninit();
    qb_backup::backup_uninit();

    qb_log::log_uninit();
    return 0;
}
