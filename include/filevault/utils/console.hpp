#ifndef FILEVAULT_UTILS_CONSOLE_HPP
#define FILEVAULT_UTILS_CONSOLE_HPP

#include <string>
#include <fmt/core.h>
#include <fmt/color.h>

namespace filevault {
namespace utils {

/**
 * @brief Console utilities for colored output
 */
class Console {
public:
    static void success(const std::string& msg);
    static void error(const std::string& msg);
    static void warning(const std::string& msg);
    static void info(const std::string& msg);
    static void debug(const std::string& msg);
    
    static void separator(char ch = '=', size_t width = 80);
    static void header(const std::string& title);
};

} // namespace utils
} // namespace filevault

#endif // FILEVAULT_UTILS_CONSOLE_HPP
