#include "filevault/utils/table_formatter.hpp"
#include <fmt/core.h>
#include <iostream>
#include <locale>

using namespace tabulate;

// Fix locale issues on Windows
static void init_locale() {
    try {
        std::locale::global(std::locale(""));
    } catch (...) {
        // Fallback to C locale if system locale fails
        std::locale::global(std::locale("C"));
    }
}

namespace filevault {
namespace utils {

TableFormatter::TableFormatter(const std::vector<std::string>& headers) {
    tabulate::Table::Row_t row;
    for (const auto& h : headers) {
        row.push_back(h);
    }
    table_.add_row(row);
    
    // Style header row
    table_[0].format()
        .font_color(Color::cyan)
        .font_align(FontAlign::center)
        .font_style({FontStyle::bold});
}

void TableFormatter::add_row(const std::vector<std::string>& row) {
    tabulate::Table::Row_t table_row;
    for (const auto& cell : row) {
        table_row.push_back(cell);
    }
    table_.add_row(table_row);
}

void TableFormatter::print() {
    try {
        init_locale();
        std::cout << table_ << std::endl;
    } catch (const std::exception&) {
        // Fallback: print as simple text
        std::cout << to_string() << std::endl;
    }
}

std::string TableFormatter::to_string() {
    try {
        init_locale();
        std::stringstream ss;
        ss << table_;
        return ss.str();
    } catch (const std::exception&) {
        return "[Table formatting error]";
    }
}

void TableFormatter::set_border_style(const std::string& style) {
    if (style == "unicode") {
        table_.format()
            .border_top("━")
            .border_bottom("━")
            .border_left("┃")
            .border_right("┃")
            .corner_top_left("┏")
            .corner_top_right("┓")
            .corner_bottom_left("┗")
            .corner_bottom_right("┛")
            .corner("┼");
    } else if (style == "markdown") {
        table_.format()
            .multi_byte_characters(false)
            .border_top("-")
            .border_bottom("-")
            .border_left("|")
            .border_right("|")
            .corner("+");
    }
    // ascii is default
}

void TableFormatter::set_column_alignment(size_t col_index, FontAlign align) {
    table_.column(col_index).format().font_align(align);
}

void TableFormatter::set_column_format(size_t col_index, Color color) {
    table_.column(col_index).format()
        .font_color(color);
}

// Pre-built tables
TableFormatter TableFormatter::algorithm_list_table() {
    TableFormatter table({"Algorithm", "Type", "Security", "Speed"});
    table.set_border_style("unicode");
    return table;
}

TableFormatter TableFormatter::benchmark_table() {
    TableFormatter table({"Size", "Throughput", "Time"});
    table.set_border_style("unicode");
    table.set_column_alignment(1, FontAlign::right);
    table.set_column_alignment(2, FontAlign::right);
    return table;
}

TableFormatter TableFormatter::file_info_table() {
    TableFormatter table({"Property", "Value"});
    table.set_border_style("unicode");
    return table;
}

// Quick builders
void TableBuilder::print_algorithm_list(
    const std::vector<std::tuple<std::string, std::string, std::string>>& algorithms
) {
    auto table = TableFormatter::algorithm_list_table();
    
    for (const auto& [name, type, security] : algorithms) {
        table.add_row({name, type, security, "⚡"});
    }
    
    table.print();
}

void TableBuilder::print_benchmark_results(
    const std::vector<std::tuple<std::string, double, double>>& results
) {
    auto table = TableFormatter::benchmark_table();
    
    for (const auto& [size, throughput, time] : results) {
        table.add_row({
            size,
            fmt::format("{:.2f} MB/s", throughput),
            fmt::format("{:.2f} ms", time)
        });
    }
    
    table.print();
}

void TableBuilder::print_file_summary(
    const std::string& input_file,
    const std::string& output_file,
    size_t input_size,
    size_t output_size,
    double processing_time_ms
) {
    auto table = TableFormatter::file_info_table();
    
    auto format_size = [](size_t bytes) -> std::string {
        if (bytes >= 1024 * 1024 * 1024) {
            return fmt::format("{:.2f} GB", bytes / (1024.0 * 1024.0 * 1024.0));
        } else if (bytes >= 1024 * 1024) {
            return fmt::format("{:.2f} MB", bytes / (1024.0 * 1024.0));
        } else if (bytes >= 1024) {
            return fmt::format("{:.2f} KB", bytes / 1024.0);
        }
        return fmt::format("{} bytes", bytes);
    };
    
    table.add_row({"Input File", input_file});
    table.add_row({"Output File", output_file});
    table.add_row({"Input Size", format_size(input_size)});
    table.add_row({"Output Size", format_size(output_size)});
    table.add_row({"Compression Ratio", 
                  fmt::format("{:.1f}%", 100.0 * output_size / input_size)});
    table.add_row({"Processing Time", fmt::format("{:.2f} ms", processing_time_ms)});
    
    table.print();
}

void TableBuilder::print_encryption_config(
    const std::string& algorithm,
    const std::string& kdf,
    const std::string& security_level,
    size_t kdf_iterations,
    size_t kdf_memory_kb
) {
    auto table = TableFormatter::file_info_table();
    
    table.add_row({"Algorithm", algorithm});
    table.add_row({"KDF", kdf});
    table.add_row({"Security Level", security_level});
    table.add_row({"KDF Iterations", std::to_string(kdf_iterations)});
    table.add_row({"KDF Memory", fmt::format("{} MB", kdf_memory_kb / 1024)});
    
    table.print();
}

} // namespace utils
} // namespace filevault
