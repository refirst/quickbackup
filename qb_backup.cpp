#include <vector>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include "qb_backup.h"
#include "qb_util.h"
#include "qb_log.h"

extern int _thread_count;
extern int _buffer_size;
extern std::string _input_src;
extern std::string _output_dir;
extern std::vector<std::string> _all_src_files;

namespace qb_backup
{
    static size_t _input_base_dir_len = 0;
    //条件变量，控制唤醒多线程拷贝的执行
    static pthread_cond_t _wakeup_cond;
    static pthread_mutex_t _wakeup_mutex;
    static int _wakeup_flag = 0;
    //互斥锁，控制拷贝源的访问
    static pthread_mutex_t _backup_mutex;
    //条件变量，控制所有拷贝线程的结束
    static pthread_cond_t _over_cond;
    static pthread_mutex_t _over_mutex;
    static int _over_flag = 0;

    static inline std::string get_inner_path(const std::string& path)
    {
        return path.substr(_input_base_dir_len);
    }

    static int mkdir_p(const std::string& src, const std::string& dest_path)
    {
        int is_dir = 0;
        struct stat st;
        int status = stat(src.c_str(), &st);
        if (status < 0)
        {
            int err = errno;
            LOG_WARN(("directory or file: [%s] stat error: [%d:%s]!", src.c_str(), err, strerror(err)));
            return is_dir;
        }

        std::string new_path = dest_path;
        if (!S_ISDIR(st.st_mode))
        {
            size_t pos = new_path.rfind('/');
            new_path = new_path.substr(0, pos);
        }
        else
        {
            is_dir = 1;
        }

        if (access(new_path.c_str(), F_OK) < 0)
        {
            std::string mk = "mkdir -p " + new_path;
            system(mk.c_str());
        }

        return is_dir;
    }

    static void backup_thread(void* arg)
    {
        int thread_idx = (long)arg;

        LOG_INFO(("cond waits for waking up [%d]...", thread_idx));
        pthread_mutex_lock(&_wakeup_mutex);
        while (_wakeup_flag == 0)
        {
            pthread_cond_wait(&_wakeup_cond, &_wakeup_mutex);
        }
        pthread_mutex_unlock(&_wakeup_mutex);
        LOG_INFO(("cond has waked up [%d]...", thread_idx));

        while (1)
        {
            pthread_mutex_lock(&_backup_mutex);
            int cnt = _all_src_files.size();
            if (cnt <= 0)
            {
                pthread_mutex_unlock(&_backup_mutex);
                break;
            }
            std::string src = _all_src_files[0];
            _all_src_files.erase(_all_src_files.begin());
            pthread_mutex_unlock(&_backup_mutex);

            std::string dest_path = _output_dir + get_inner_path(src);
            mkdir_p(src, dest_path);

            /*std::string backup = "cp -rf \"" + src + "\" \"" + dest_path + "\"";
            system(backup.c_str());*/

            int fd_src = open(src.c_str(), O_RDONLY);
            if (fd_src < 0)
            {
                int err = errno;
                LOG_WARN(("src [%s] open failed: [%d:%s]!", src.c_str(), err, strerror(err)));
                continue;
            }

            int fd_dest = open(dest_path.c_str(), O_RDWR | O_CREAT, 0664);
            if (fd_dest < 0)
            {
                int err = errno;
                LOG_WARN(("dest [%s] open failed: [%d:%s]!", dest_path.c_str(), err, strerror(err)));
                close(fd_src);
                continue;
            }

            int pipe_fd[2];
            int status = pipe(pipe_fd);
            if (status < 0)
            {
                int err = errno;
                LOG_WARN(("pipe [%s] error: [%d:%s]!", src.c_str(), err, strerror(err)));
                close(fd_src);
                close(fd_dest);
                continue;
            }

            while (1)
            {
                status = splice(fd_src, NULL, pipe_fd[1], NULL, _buffer_size * 1024 * 1024, SPLICE_F_MORE | SPLICE_F_MOVE);
                if (status > 0)
                {
                    status = splice(pipe_fd[0], NULL, fd_dest, NULL, _buffer_size * 1024 * 1024, SPLICE_F_MORE | SPLICE_F_MOVE);
                    if (status <= 0)
                    {
                        if (status < 0)
                        {
                            int err = errno;
                            LOG_WARN(("splice write from pipe [%s] error: [%d:%s]!", src.c_str(), err, strerror(err)));
                        }
                        break;
                    }
                }
                else
                {
                    if (status < 0)
                    {
                        int err = errno;
                        LOG_WARN(("splice read to pipe [%s] error: [%d:%s]!", src.c_str(), err, strerror(err)));
                    }
                    break;
                }
            }

            close(fd_src);
            close(fd_dest);
            close(pipe_fd[0]);
            close(pipe_fd[1]);
        }

        pthread_mutex_lock(&_over_mutex);
        _over_flag--;
        pthread_mutex_unlock(&_over_mutex);
        LOG_INFO(("cond [%d] over...", _over_flag));
        pthread_cond_signal(&_over_cond);
    }

    void backup_init()
    {
        if (_thread_count <= 1)
        {
            return;
        }

        pthread_mutexattr_t mutex_attr;
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_wakeup_mutex, &mutex_attr);
        pthread_cond_init(&_wakeup_cond, NULL);
        _wakeup_flag = 0;
        pthread_mutex_init(&_backup_mutex, &mutex_attr);
        pthread_mutex_init(&_over_mutex, &mutex_attr);
        pthread_cond_init(&_over_cond, NULL);
        _over_flag = _thread_count;
        pthread_mutexattr_destroy(&mutex_attr);

        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        for (int i = 0; i < _thread_count; ++i)
        {
            int status = pthread_create(&tid, &attr, (void*(*)(void*))backup_thread, (void*)i);
            if (status != 0)
            {
                int err = errno;
                LOG_WARN(("create backup thread error: [%d:%s]!", err, strerror(err)));
                exit(1);
            }
        }
    }

    void backup_uninit()
    {
        if (_thread_count <= 1)
        {
            return;
        }

        pthread_cond_destroy(&_over_cond);
        pthread_mutex_destroy(&_over_mutex);
        pthread_mutex_destroy(&_backup_mutex);
        pthread_cond_destroy(&_wakeup_cond);
        pthread_mutex_destroy(&_wakeup_mutex);
    }

    void backup_run()
    {
        std::string input_dir = _input_src;
        size_t pos = input_dir.rfind('/');
        input_dir = input_dir.substr(0, pos);
        _input_base_dir_len = input_dir.size();

        if (_thread_count <= 1)
        {
            //单线程时直接使用cp进行拷贝
            LOG_INFO(("======begin backuping: [%s]", _input_src.c_str()));
            std::string dest_path = _output_dir + get_inner_path(_input_src);
            int is_dir = mkdir_p(_input_src, dest_path);
            std::string backup = "cp -rf " + _input_src + " " + dest_path;
            if (is_dir)
            {
                backup = "cp -rf " + _input_src + "/* " + dest_path;
            }
            system(backup.c_str());
            LOG_INFO(("======end backuping: [%s]", _input_src.c_str()));
            return;
        }

        LOG_INFO(("======begin backuping: [%s]", _input_src.c_str()));
        //唤醒拷贝线程开始进行拷贝
        pthread_mutex_lock(&_wakeup_mutex);
        _wakeup_flag = 1;
        pthread_mutex_unlock(&_wakeup_mutex);
        pthread_cond_broadcast(&_wakeup_cond);

        //等待所有拷贝线程退出
        pthread_mutex_lock(&_over_mutex);
        while (_over_flag > 0)
        {
            pthread_cond_wait(&_over_cond, &_over_mutex);
        }
        pthread_mutex_unlock(&_over_mutex);
        LOG_INFO(("======end backuping: [%s]", _input_src.c_str()));
    }
}
