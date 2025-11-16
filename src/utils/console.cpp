#include "filevault/utils/console.hpp"

namespace filevault {
namespace utils {

void Console::success(const std::string& msg) {
    fmt::print(fmt::fg(fmt::color::green), "✓ ");
    fmt::print("{}\n", msg);
}

void Console::error(const std::string& msg) {
    fmt::print(fmt::fg(fmt::color::red), "✗ ");
    fmt::print("{}\n", msg);
}

void Console::warning(const std::string& msg) {
    fmt::print(fmt::fg(fmt::color::yellow), "⚠ ");
    fmt::print("{}\n", msg);
}

void Console::info(const std::string& msg) {
    fmt::print(fmt::fg(fmt::color::blue), "ℹ ");
    fmt::print("{}\n", msg);
}

void Console::debug(const std::string& msg) {
    fmt::print(fmt::fg(fmt::color::cyan), "🔍 ");
    fmt::print("{}\n", msg);
}

void Console::separator(char ch, size_t width) {
    fmt::print("{}\n", std::string(width, ch));
}

void Console::header(const std::string& title) {
    separator('=', 80);
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::cyan), "{}\n", title);
    separator('=', 80);
}

} // namespace utils
} // namespace filevault
