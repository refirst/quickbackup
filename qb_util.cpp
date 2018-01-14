#include "qb_util.h"

namespace qb_util
{
    static const std::string _space_str = "\t\r\n \f\v";

    std::string trim_left(const std::string& src, const std::string& t)
    {
        std::string t_str = t;
        if (t_str.empty())
        {
            t_str = _space_str;
        }
        std::string ret = src;
        ret.erase(0, ret.find_first_not_of(t_str));
        return ret;
    }

    std::string trim_right(const std::string& src, const std::string& t)
    {
        std::string t_str = t;
        if (t_str.empty())
        {
            t_str = _space_str;
        }
        std::string ret = src;
        ret.erase(ret.find_last_not_of(t_str) + 1);
        return ret;
    }

    std::string trim(const std::string& src, const std::string& t)
    {
        std::string t_str = t;
        if (t_str.empty())
        {
            t_str = _space_str;
        }
        std::string ret = src;
        ret.erase(0, ret.find_first_not_of(t_str));
        ret.erase(ret.find_last_not_of(t_str) + 1);
        return ret;
    }
}
