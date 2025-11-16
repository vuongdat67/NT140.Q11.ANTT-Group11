#pragma once

#include <cstdint>
#include <indicators/progress_bar.hpp>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <memory>
#include <string>
#include <vector>

namespace filevault {
namespace utils {

/**
 * @brief Modern progress bar wrapper
 */
class ProgressBar {
public:
    ProgressBar(const std::string& prefix, size_t max_progress = 100);
    ~ProgressBar();
    
    void set_progress(size_t progress);
    void tick();
    void set_postfix(const std::string& text);
    void mark_as_completed();
    
    void hide();
    void show();
    
private:
    std::unique_ptr<indicators::ProgressBar> bar_;
    size_t current_progress_;
    size_t max_progress_;
};

/**
 * @brief Block-style progress bar
 */
class BlockProgressBar {
public:
    BlockProgressBar(const std::string& prefix, size_t max_progress = 100);
    
    void set_progress(size_t progress);
    void set_option_text(const std::string& text);
    void mark_as_completed();
    
private:
    std::unique_ptr<indicators::BlockProgressBar> bar_;
};

// MultiProgress removed - not compatible with indicators API

// Spinner removed - use ProgressBar instead

} // namespace utils
} // namespace filevault
