#pragma once

#include <tabulate/table.hpp>
#include <string>
#include <vector>

namespace filevault {
namespace utils {

/**
 * @brief Beautiful table formatter
 */
class TableFormatter {
public:
    TableFormatter(const std::vector<std::string>& headers);
    
    void add_row(const std::vector<std::string>& row);
    void print();
    std::string to_string();
    
    // Styling
    void set_border_style(const std::string& style);  // "ascii", "unicode", "markdown"
    void set_column_alignment(size_t col_index, tabulate::FontAlign align);
    void set_column_format(size_t col_index, tabulate::Color color);
    
    // Pre-built themes
    static TableFormatter algorithm_list_table();
    static TableFormatter benchmark_table();
    static TableFormatter file_info_table();
    
private:
    tabulate::Table table_;
};

/**
 * @brief Quick table builders
 */
class TableBuilder {
public:
    static void print_algorithm_list(
        const std::vector<std::tuple<std::string, std::string, std::string>>& algorithms
    );
    
    static void print_benchmark_results(
        const std::vector<std::tuple<std::string, double, double>>& results
    );
    
    static void print_file_summary(
        const std::string& input_file,
        const std::string& output_file,
        size_t input_size,
        size_t output_size,
        double processing_time_ms
    );
    
    static void print_encryption_config(
        const std::string& algorithm,
        const std::string& kdf,
        const std::string& security_level,
        size_t kdf_iterations,
        size_t kdf_memory_kb
    );
};

} // namespace utils
} // namespace filevault
