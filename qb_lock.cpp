#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <vector>
#include "qb_lock.h"
#include "qb_log.h"

extern int _need_lock;
extern int _program_pid;
std::vector<std::string> _all_src_files;

namespace qb_lock
{
    static std::vector<int> _all_src_fds;
    static void set_file(const std::string& file)
    {
        //LOG_INFO(("[%s]", file.c_str()));
        int fd = 0;
        if (_program_pid > 1)
        {
            //do noting
        }
        else if (_need_lock)
        {
            fd = open(file.c_str(), O_WRONLY);
            if (fd < 0)
            {
                int err = errno;
                LOG_WARN(("file [%s] open failed: [%d:%s]!", file.c_str(), err, strerror(err)));
                return;
            }

            static struct flock lock;
            lock.l_type = F_WRLCK;
            lock.l_start = 0;
            lock.l_whence = SEEK_SET;
            lock.l_len = 0;
            lock.l_pid = getpid();

            int ret = fcntl(fd, F_SETLK, &lock);
            if (ret < 0)
            {
                int err = errno;
                LOG_WARN(("file [%s] lock failed: [%d:%s]!", file.c_str(), err, strerror(err)));
                close(fd);
                return;
            }
        }

        _all_src_files.push_back(file);
        _all_src_fds.push_back(fd);
    }

    static void get_all_files(const std::string& path)
    {
        DIR* dir = opendir(path.c_str());
        if (dir == NULL)
        {
            int err = errno;
            LOG_WARN(("directory [%s] open failed: [%d:%s]!", path.c_str(), err, strerror(err)));
            return;
        }

        struct dirent* ptr;
        while ((ptr = readdir(dir)) != NULL)
        {
            if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            {
                continue;
            }

            switch (ptr->d_type)
            {
                case DT_REG:
                    {
                        std::string tmp = path + "/" + ptr->d_name;
                        set_file(tmp);
                    }
                    break;
                case DT_DIR:
                    {
                        std::string tmp = path + "/" + ptr->d_name;
                        get_all_files(tmp);
                    }
                    break;
                default:
                    //LOG_WARN(("[%s/%s] type is [%d], do not copy", path.c_str(), ptr->d_name, ptr->d_type));
                    break;
            }
        }

        closedir(dir);
    }

    void lock_init(const char* src_path)
    {
        std::string src = src_path;

        struct stat st;
        int status = stat(src.c_str(), &st);
        if (status < 0)
        {
            int err = errno;
            LOG_WARN(("source directory or file: [%s] stat error: [%d:%s]!", src.c_str(), err, strerror(err)));
            return;
        }

        LOG_INFO(("------begin locking [%s]", src.c_str()));
        if (_program_pid > 1)
        {
            char op_pid[100] = { 0 };
            sprintf(op_pid, "kill -STOP %d", _program_pid);
            system(op_pid);
        }
        if (S_ISDIR(st.st_mode))
        {
            get_all_files(src);
        }
        else if (S_ISREG(st.st_mode))
        {
            set_file(src);
        }
        LOG_INFO(("------end locking [%s]", src.c_str()));
    }

    void lock_uninit()
    {
        if (_program_pid > 1)
        {
            char op_pid[100] = { 0 };
            sprintf(op_pid, "kill -CONT %d", _program_pid);
            system(op_pid);
        }
        else if (_need_lock)
        {
            std::vector<int>::iterator iter = _all_src_fds.begin();
            for (; iter != _all_src_fds.end(); ++iter)
            {
                if ((*iter) > 0)
                {
                    close(*iter);
                }
            }
        }
    }
}
