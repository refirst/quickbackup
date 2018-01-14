#include <getopt.h>
#include <iostream>
#include <errno.h>
#include "qb_util.h"
#include "qb_option.h"
#include "qb_log.h"

extern int _daemonize;
extern int _need_lock;
extern int _program_pid;
extern int _thread_count;
extern int _buffer_size;
extern std::string _input_src;
extern std::string _output_dir;

static int _show_help = 0;
static const struct option _long_opts[] = {
    {"help",      no_argument,       NULL, 'h'},
    {"daemonize", no_argument,       NULL, 'd'},
    {"lock",      no_argument,       NULL, 'l'},
    {"pid",       required_argument, NULL, 'p'},
    {"threads",   required_argument, NULL, 't'},
    {"buffer",    required_argument, NULL, 'b'},
    {"input",     required_argument, NULL, 'i'},
    {"output",    required_argument, NULL, 'o'},
    {NULL,        0,                 NULL,  0 }
};
static const char _short_opts[] = "hdlp:t:b:i:o:";

namespace qb_option
{
    static int get_options(int argc, char** argv)
    {
        while (1)
        {
            int ch = getopt_long(argc, argv, _short_opts, _long_opts, NULL);
            if (ch == -1)
            {
                break;
            }

            switch (ch)
            {
                case 'h':
                    _show_help = 1;
                    break;
                case 'd':
                    _daemonize = 1;
                    break;
                case 'l':
                    _need_lock = 1;
                    break;
                case 'p':
                    {
                        int pid = atoi(optarg);
                        if (pid < 0)
                        {
                            pid = 0;
                        }
                        _program_pid = pid;
                    }
                    break;
                case 't':
                    {
                        int ts = atoi(optarg);
                        if (ts < 0)
                        {
                            ts = 1;
                        }
                        _thread_count = ts;
                    }
                    break;
                case 'b':
                    {
                        int buf = atoi(optarg);
                        if (buf < 0)
                        {
                            buf = 1;
                        }
                        _buffer_size = buf;
                    }
                    break;
                case 'i':
                    {
                        _input_src = optarg;
                        _input_src = qb_util::trim(_input_src);
                        _input_src = qb_util::trim_right(_input_src, "/");
                    }
                    break;
                case 'o':
                    {
                        _output_dir = optarg;
                        _output_dir = qb_util::trim(_output_dir);
                        _output_dir = qb_util::trim_right(_output_dir, "/");
                    }
                    break;
                default:
                    return (-1);
            }
        }

        return 0;
    }

    static int args_check(const char* work_dir_path)
    {
        if (_input_src[0] != '/')
        {
            std::string tmp = work_dir_path;
            _input_src = tmp + "/" + _input_src;
        }
        if (access(_input_src.c_str(), F_OK) != 0)
        {
            //源目录或文件不存在
            LOG_WARN(("input directory or file: [%s] does not exist!", _input_src.c_str()));
            return (-1);
        }
        else
        {
            LOG_INFO(("input directory or file: [%s]", _input_src.c_str()));
        }

        if (_output_dir[0] != '/')
        {
            std::string tmp = work_dir_path;
            _output_dir = tmp + "/" + _output_dir;
        }
        if (access(_output_dir.c_str(), F_OK) == 0)
        {
            //删除已存在的目标目录
            std::string rm = "rm -rf " + _output_dir;
            system(rm.c_str());
        }
        LOG_INFO(("output directory: [%s]", _output_dir.c_str()));

        return 0;
    }

    int options_check(int argc, char** argv, const char* work_dir_path)
    {
        int opt = get_options(argc, argv);
        if (opt != 0 || _show_help || _input_src.empty() || _output_dir.empty())
        {
            std::cout << "Usage: quickbackup [-hdl] [-p pid] [-t thread count] [-b buffer size] [-i input directory or file] [-o output directory]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -h, --help      : show help (optional)" << std::endl;
            std::cout << "  -d, --daemonize : run as a daemon (optional)" << std::endl;
            std::cout << "  -l, --lock      : need lock source when backuping (optional)" << std::endl;
            std::cout << "  -p, --pid       : set pid to hang up before copy data (optional, default 0, if define this, -l will be ignored)" << std::endl;
            std::cout << "  -t, --threads   : set thread count to run (optional, default 1)" << std::endl;
            std::cout << "  -b, --buffer    : set splice read or write buffer size (optional, default 1, unit: MB)" << std::endl;
            std::cout << "  -i, --input     : set source input directory or file (required)" << std::endl;
            std::cout << "  -o, --output    : set destination output directory (required)" << std::endl;
            return (-1);
        }

        return args_check(work_dir_path);
    }
}
