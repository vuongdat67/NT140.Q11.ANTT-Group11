#ifndef FILEVAULT_UTILS_FILE_IO_HPP
#define FILEVAULT_UTILS_FILE_IO_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <span>
#include "filevault/core/result.hpp"

namespace filevault {
namespace utils {

/**
 * @brief File I/O utilities
 */
class FileIO {
public:
    /**
     * @brief Read entire file into memory
     */
    static core::Result<std::vector<uint8_t>> read_file(const std::string& path);
    
    /**
     * @brief Write data to file
     */
    static core::Result<void> write_file(const std::string& path, std::span<const uint8_t> data);
    
    /**
     * @brief Check if file exists
     */
    static bool file_exists(const std::string& path);
    
    /**
     * @brief Get file size
     */
    static size_t file_size(const std::string& path);
};

} // namespace utils
} // namespace filevault

#endif // FILEVAULT_UTILS_FILE_IO_HPP
