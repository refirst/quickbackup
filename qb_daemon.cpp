#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <iostream>
#include "qb_daemon.h"
#include "qb_log.h"

namespace qb_daemon
{
    void daemon_run(int daemon, int cur_work)
    {
        if (daemon == 0)
        {
            return;
        }

        //创建第一个子进程
        pid_t pid = fork();
        if (pid < 0)
        {
            int err = errno;
            LOG_WARN(("first fork failed: [%d:%s]!", err, strerror(err)));
            return;
        }
        else if (pid > 0)
        {
            //退出父进程
            exit(0);
        }

        //在第一个子进程中创建新的会话，并担任会话组leader
        pid_t sid = setsid();
        if (sid < 0)
        {
            int err = errno;
            LOG_WARN(("setsid failed: [%d:%s]!", err, strerror(err)));
            return;
        }

        if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
        {
            int err = errno;
            LOG_WARN(("sighup failed: [%d:%s]!", err, strerror(err)));
            return;
        }

        //创建第二个子进程
        pid = fork();
        if (pid < 0)
        {
            int err = errno;
            LOG_WARN(("second fork failed: [%d:%s]!", err, strerror(err)));
            return;
        }
        else if (pid > 0)
        {
            //退出第一个子进程
            exit(0);
        }

        if (cur_work != 0)
        {
            //设置当前工作路径为根路径
            int status = chdir("/");
            if (status < 0)
            {
                int err = errno;
                LOG_WARN(("chdir failed: [%d:%s]!", err, strerror(err)));
                return;
            }
        }

        //设置文件权限掩码
        umask(0);

        //将标准输入、输出、错误重定向到/dev/null，也可直接关闭标准输入、输出、错误文件描述符
        int fd = open("/dev/null", O_RDWR);
        if (fd < 0)
        {
            int err = errno;
            LOG_WARN(("open dev null failed: [%d:%s]!", err, strerror(err)));
            return;
        }

        int status = dup2(fd, STDIN_FILENO);
        if (status < 0)
        {
            int err = errno;
                        LOG_WARN(("dup2 stdin failed: [%d:%s]!", err, strerror(err)));
            close(fd);
            return;
        }

        status = dup2(fd, STDOUT_FILENO);
        if (status < 0)
        {
            int err = errno;
            LOG_WARN(("dup2 stdout failed: [%d:%s]!", err, strerror(err)));
            close(fd);
            return;
        }

        status = dup2(fd, STDERR_FILENO);
        if (status < 0)
        {
            int err = errno;
            LOG_WARN(("dup2 stderr failed: [%d:%s]!", err, strerror(err)));
            close(fd);
            return;
        }

        if (fd > STDERR_FILENO)
        {
            status = close(fd);
            if (status < 0)
            {
                int err = errno;
                LOG_WARN(("close fd failed: [%d:%s]!", err, strerror(err)));
                return;
            }
        }

        LOG_INFO(("run as a daemon!"));
    }
}
