#include "filevault/utils/progress.hpp"

using namespace indicators;

namespace filevault {
namespace utils {

ProgressBar::ProgressBar(const std::string& prefix, size_t max_progress)
    : current_progress_(0), max_progress_(max_progress) {
    
    bar_ = std::make_unique<indicators::ProgressBar>(
        option::BarWidth{50},
        option::Start{"["},
        option::Fill{"█"},
        option::Lead{"█"},
        option::Remainder{"-"},
        option::End{"]"},
        option::PrefixText{prefix},
        option::ForegroundColor{Color::green},
        option::ShowElapsedTime{true},
        option::ShowRemainingTime{true},
        option::ShowPercentage{true},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
    );
}

ProgressBar::~ProgressBar() {
    if (bar_) {
        bar_->mark_as_completed();
    }
}

void ProgressBar::set_progress(size_t progress) {
    current_progress_ = progress;
    if (bar_) {
        bar_->set_progress(progress);
    }
}

void ProgressBar::tick() {
    if (current_progress_ < max_progress_) {
        set_progress(current_progress_ + 1);
    }
}

void ProgressBar::set_postfix(const std::string& text) {
    if (bar_) {
        bar_->set_option(option::PostfixText{text});
    }
}

void ProgressBar::mark_as_completed() {
    if (bar_) {
        bar_->mark_as_completed();
    }
}

void ProgressBar::hide() {
    indicators::show_console_cursor(false);
}

void ProgressBar::show() {
    indicators::show_console_cursor(true);
}

// BlockProgressBar implementation
BlockProgressBar::BlockProgressBar(const std::string& prefix, size_t max_progress) {
    bar_ = std::make_unique<indicators::BlockProgressBar>(
        option::BarWidth{80},
        option::ForegroundColor{Color::cyan},
        option::PrefixText{prefix},
        option::ShowPercentage{true},
        option::MaxProgress{max_progress}
    );
}

void BlockProgressBar::set_progress(size_t progress) {
    if (bar_) {
        bar_->set_progress(progress);
    }
}

void BlockProgressBar::set_option_text(const std::string& text) {
    if (bar_) {
        bar_->set_option(option::PostfixText{text});
    }
}

void BlockProgressBar::mark_as_completed() {
    if (bar_) {
        bar_->mark_as_completed();
    }
}

} // namespace utils
} // namespace filevault
