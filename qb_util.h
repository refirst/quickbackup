#ifndef QB_UTIL_H
#define QB_UTIL_H

#include <string>

#define UNUSED(x) ((void)(x))

namespace qb_util
{
    std::string trim_left(const std::string& src, const std::string& t = "");
    std::string trim_right(const std::string& src, const std::string& t = "");
    std::string trim(const std::string& src, const std::string& t = "");
}

#endif
